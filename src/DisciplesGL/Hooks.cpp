/*
	MIT License

	Copyright (c) 2019 Oleksiy Ryabchun

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#include "stdafx.h"
#include "Shellapi.h"
#include "Objbase.h"
#include "Hooks.h"
#include "Main.h"
#include "Config.h"
#include "Resource.h"
#include "Window.h"

namespace Hooks
{
	HMODULE hModule;
	INT baseOffset;

#pragma region Hook helpers
	BOOL __fastcall PatchRedirect(DWORD addr, VOID* hook, BYTE instruction)
	{
		DWORD address = addr + baseOffset;

		DWORD old_prot;
		if (VirtualProtect((VOID*)address, 5, PAGE_EXECUTE_READWRITE, &old_prot))
		{
			BYTE* jump = (BYTE*)address;
			*jump = instruction;
			++jump;
			*(DWORD*)jump = (DWORD)hook - (DWORD)address - 5;

			VirtualProtect((VOID*)address, 5, old_prot, &old_prot);

			return TRUE;
		}
		return FALSE;
	}

	BOOL __fastcall PatchHook(DWORD addr, VOID* hook)
	{
		return PatchRedirect(addr, hook, 0xE9);
	}

	BOOL __fastcall PatchCall(DWORD addr, VOID* hook)
	{
		return PatchRedirect(addr, hook, 0xE8);
	}

	BOOL __fastcall PatchNop(DWORD addr, DWORD size)
	{
		DWORD address = addr + baseOffset;

		DWORD old_prot;
		if (VirtualProtect((VOID*)address, size, PAGE_EXECUTE_READWRITE, &old_prot))
		{
			MemorySet((VOID*)address, 0x90, size);
			VirtualProtect((VOID*)address, size, old_prot, &old_prot);

			return TRUE;
		}
		return FALSE;
	}

	BOOL __fastcall PatchBlock(DWORD addr, VOID* block, DWORD size)
	{
		DWORD address = addr + baseOffset;

		DWORD old_prot;
		if (VirtualProtect((VOID*)address, size, PAGE_EXECUTE_READWRITE, &old_prot))
		{
			switch (size)
			{
			case 4:
				*(DWORD*)address = *(DWORD*)block;
				break;
			case 2:
				*(WORD*)address = *(WORD*)block;
				break;
			case 1:
				*(BYTE*)address = *(BYTE*)block;
				break;
			default:
				MemoryCopy((VOID*)address, block, size);
				break;
			}

			VirtualProtect((VOID*)address, size, old_prot, &old_prot);

			return TRUE;
		}
		return FALSE;
	}

	BOOL __fastcall ReadBlock(DWORD addr, VOID* block, DWORD size)
	{
		DWORD address = addr + baseOffset;

		DWORD old_prot;
		if (VirtualProtect((VOID*)address, size, PAGE_READONLY, &old_prot))
		{
			switch (size)
			{
			case 4:
				*(DWORD*)block = *(DWORD*)address;
				break;
			case 2:
				*(WORD*)block = *(WORD*)address;
				break;
			case 1:
				*(BYTE*)block = *(BYTE*)address;
				break;
			default:
				MemoryCopy(block, (VOID*)address, size);
				break;
			}

			VirtualProtect((VOID*)address, size, old_prot, &old_prot);

			return TRUE;
		}
		return FALSE;
	}

	BOOL __fastcall PatchWord(DWORD addr, WORD value)
	{
		return PatchBlock(addr, &value, sizeof(value));
	}

	BOOL __fastcall PatchDWord(DWORD addr, DWORD value)
	{
		return PatchBlock(addr, &value, sizeof(value));
	}

	BOOL __fastcall PatchByte(DWORD addr, BYTE value)
	{
		return PatchBlock(addr, &value, sizeof(value));
	}

	BOOL __fastcall ReadWord(DWORD addr, WORD* value)
	{
		return ReadBlock(addr, value, sizeof(*value));
	}

	BOOL __fastcall ReadDWord(DWORD addr, DWORD* value)
	{
		return ReadBlock(addr, value, sizeof(*value));
	}

	BOOL __fastcall ReadByte(DWORD addr, BYTE* value)
	{
		return ReadBlock(addr, value, sizeof(*value));
	}

	DWORD __fastcall PatchFunction(MappedFile* file, const CHAR* function, VOID* addr)
	{
		DWORD res = NULL;

		DWORD base = (DWORD)file->hModule;
		PIMAGE_NT_HEADERS headNT = (PIMAGE_NT_HEADERS)((BYTE*)base + ((PIMAGE_DOS_HEADER)file->hModule)->e_lfanew);

		PIMAGE_DATA_DIRECTORY dataDir = &headNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
		if (dataDir->Size)
		{
			PIMAGE_IMPORT_DESCRIPTOR imports = (PIMAGE_IMPORT_DESCRIPTOR)(base + dataDir->VirtualAddress);
			for (DWORD idx = 0; imports->Name; ++idx, ++imports)
			{
				CHAR* libraryName = (CHAR*)(base + imports->Name);

				PIMAGE_THUNK_DATA addressThunk = (PIMAGE_THUNK_DATA)(base + imports->FirstThunk);
				PIMAGE_THUNK_DATA nameThunk;
				if (imports->OriginalFirstThunk)
					nameThunk = (PIMAGE_THUNK_DATA)(base + imports->OriginalFirstThunk);
				else
				{
					if (!file->hFile)
					{
						CHAR filePath[MAX_PATH];
						GetModuleFileName(file->hModule, filePath, MAX_PATH);
						file->hFile = CreateFile(filePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
						if (!file->hFile)
							return res;
					}

					if (!file->hMap)
					{
						file->hMap = CreateFileMapping(file->hFile, NULL, PAGE_READONLY, 0, 0, NULL);
						if (!file->hMap)
							return res;
					}

					if (!file->address)
					{
						file->address = MapViewOfFile(file->hMap, FILE_MAP_READ, 0, 0, 0);;
						if (!file->address)
							return res;
					}

					headNT = (PIMAGE_NT_HEADERS)((BYTE*)file->address + ((PIMAGE_DOS_HEADER)file->address)->e_lfanew);
					PIMAGE_SECTION_HEADER sh = (PIMAGE_SECTION_HEADER)((DWORD)&headNT->OptionalHeader + headNT->FileHeader.SizeOfOptionalHeader);

					nameThunk = NULL;
					DWORD sCount = headNT->FileHeader.NumberOfSections;
					while (sCount--)
					{
						if (imports->FirstThunk >= sh->VirtualAddress && imports->FirstThunk < sh->VirtualAddress + sh->Misc.VirtualSize)
						{
							nameThunk = PIMAGE_THUNK_DATA((DWORD)file->address + sh->PointerToRawData + imports->FirstThunk - sh->VirtualAddress);
							break;
						}

						++sh;
					}

					if (!nameThunk)
						return res;
				}

				for (; nameThunk->u1.AddressOfData; ++nameThunk, ++addressThunk)
				{
					PIMAGE_IMPORT_BY_NAME name = PIMAGE_IMPORT_BY_NAME(base + nameThunk->u1.AddressOfData);

					WORD hint;
					if (ReadWord((INT)name - baseOffset, &hint) && !StrCompare((CHAR*)name->Name, function))
					{
						if (ReadDWord((INT)&addressThunk->u1.AddressOfData - baseOffset, &res))
							PatchDWord((INT)&addressThunk->u1.AddressOfData - baseOffset, (DWORD)addr);

						return res;
					}
				}
			}
		}

		return res;
	}
#pragma endregion

	// ===============================================================
	HWND hWndMain;

	HWND __stdcall CreateWindowExHook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, INT X, INT Y, INT nWidth, INT nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
	{
		BOOL isMain = !StrCompare(lpClassName, "MQ_UIManager");
		if (isMain)
		{
			dwStyle = WS_WINDOWED;
			RECT rect = { 0, 0, (LONG)config.mode->width, (LONG)config.mode->height };
			AdjustWindowRect(&rect, dwStyle, TRUE);
			
			nWidth = rect.right - rect.left;
			nHeight = rect.bottom - rect.top;

			X = (GetSystemMetrics(SM_CXSCREEN) - nWidth) >> 1;
			Y = (GetSystemMetrics(SM_CYSCREEN) - nHeight) >> 1;
		}

		HWND hWnd = CreateWindowEx(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		if (isMain)
			hWndMain = hWnd;

		return hWnd;
	}

	ATOM __stdcall RegisterClassHook(WNDCLASSA* lpWndClass)
	{
		if (!StrCompare(lpWndClass->lpszClassName, "MQ_UIManager"))
		{
			config.cursor = lpWndClass->hCursor;
			config.icon = lpWndClass->hIcon;
			lpWndClass->hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		}

		return RegisterClass(lpWndClass);
	}

	LONG __stdcall SetWindowLongHook(HWND hWnd, INT nIndex, LONG dwNewLong)
	{
		if (hWnd == hWndMain)
		{
			if (nIndex == GWL_WNDPROC)
			{
				LONG res = SetWindowLong(hWnd, nIndex, dwNewLong);
				Window::SetCaptureWindow(hWnd);
				return res;
			}

			if (nIndex == GWL_STYLE)
				dwNewLong = WS_FULLSCREEN;
		}

		return SetWindowLong(hWnd, nIndex, dwNewLong);
	}

	BOOL __stdcall ShowWindowHook(HWND hWnd, INT nCmdShow)
	{
		if (hWnd == hWndMain && config.windowedMode)
			SetMenu(hWnd, config.menu);

		return ShowWindow(hWnd, nCmdShow);
	}

	HWND __stdcall GetForegroundWindowHook()
	{
		HWND hWnd = GetForegroundWindow();
		OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
		return ddraw ? ddraw->hWnd : hWnd;
	}

	INT __stdcall MessageBoxHook(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
	{
		INT res;
		ULONG_PTR cookie = NULL;
		if (hActCtx && hActCtx != INVALID_HANDLE_VALUE && !ActivateActCtxC(hActCtx, &cookie))
			cookie = NULL;

		res = MessageBox(hWnd, lpText, lpCaption, uType);

		if (cookie)
			DeactivateActCtxC(0, cookie);

		return res;
	}

	INT __stdcall ShowCursorHook(BOOL bShow) { return bShow ? 1 : -1; }

	BOOL __stdcall ClipCursorHook(RECT*) { return TRUE; }

	BOOL __stdcall GetClientRectHook(HWND hWnd, LPRECT lpRect)
	{
		if (hWnd == hWndMain)
		{
			lpRect->left = 0;
			lpRect->top = 0;
			lpRect->right = config.mode->width;
			lpRect->bottom = config.mode->height;
			return TRUE;
		}
		else
			return GetClientRect(hWnd, lpRect);
	}

	BOOL __stdcall GetWindowRectHook(HWND hWnd, LPRECT lpRect)
	{
		if (GetWindowRect(hWnd, lpRect))
		{
			if (hWnd == hWndMain)
			{
				RECT rect = { 0, 0, (LONG)config.mode->width, (LONG)config.mode->height };
				AdjustWindowRect(&rect, config.windowedMode ? WS_WINDOWED : WS_FULLSCREEN, config.windowedMode);

				lpRect->right = lpRect->left + rect.right - rect.left;
				lpRect->bottom = lpRect->top + rect.bottom - rect.top;
			}

			return TRUE;
		}

		return FALSE;
	}

	BOOL __stdcall GetCursorPosHookV1(LPPOINT lpPoint)
	{
		if (GetCursorPos(lpPoint))
		{
			RECT rect;
			if (GetClientRect(hWndMain, &rect) &&
				ClientToScreen(hWndMain, (LPPOINT)&rect))
			{
				FLOAT fx = (FLOAT)config.mode->width / rect.right;
				FLOAT fy = (FLOAT)config.mode->height / rect.bottom;

				POINT offset = { 0, 0 };
				if (config.image.aspect && fx != fy)
				{
					if (fx < fy)
					{
						fx = fy;
						offset.x = (rect.right - LONG((FLOAT)config.mode->width / fx)) >> 1;
					}
					else
					{
						fy = fx;
						offset.y = (rect.bottom - LONG((FLOAT)config.mode->height / fy)) >> 1;
					}
				}

				lpPoint->x = LONG(fx * (lpPoint->x - offset.x - rect.left));
				lpPoint->y = LONG(fy * (lpPoint->y - offset.y - rect.top));
			}

			return TRUE;
		}

		return FALSE;
	}

	BOOL __stdcall GetCursorPosHookV2(LPPOINT lpPoint)
	{
		if (GetCursorPos(lpPoint))
		{
			RECT rect;
			if (GetClientRect(hWndMain, &rect) &&
				(rect.right != config.mode->width || rect.bottom != config.mode->height) &&
				ClientToScreen(hWndMain, (LPPOINT)&rect))
			{
				FLOAT fx = (FLOAT)config.mode->width / rect.right;
				FLOAT fy = (FLOAT)config.mode->height / rect.bottom;

				POINT offset = { 0, 0 };
				if (config.image.aspect && fx != fy)
				{
					if (fx < fy)
					{
						fx = fy;
						offset.x = (rect.right - LONG((FLOAT)config.mode->width / fx)) >> 1;
					}
					else
					{
						fy = fx;
						offset.y = (rect.bottom - LONG((FLOAT)config.mode->height / fy)) >> 1;
					}
				}

				lpPoint->x = LONG(fx * (lpPoint->x - rect.left - offset.x)) + rect.left;
				lpPoint->y = LONG(fy * (lpPoint->y - rect.top - offset.y)) + rect.top;
			}

			return TRUE;
		}

		return FALSE;
	}

	BOOL __stdcall SetCursorPosHook(INT X, INT Y)
	{
		RECT rect;
		if (GetClientRect(hWndMain, &rect) &&
			(rect.right != config.mode->width || rect.bottom != config.mode->height) &&
			ClientToScreen(hWndMain, (LPPOINT)&rect))
		{
			FLOAT fx = (FLOAT)rect.right / config.mode->width;
			FLOAT fy = (FLOAT)rect.bottom / config.mode->height;

			POINT offset = { 0, 0 };
			if (config.image.aspect && fx != fy)
			{
				if (fx > fy)
				{
					fx = fy;
					offset.x = (rect.right - LONG(fx * config.mode->width)) >> 1;
				}
				else
				{
					fy = fx;
					offset.y = (rect.bottom - LONG(fy * config.mode->height)) >> 1;
				}
			}

			X = LONG(fx * (X - rect.left + offset.x)) + rect.left;
			Y = LONG(fy * (Y - rect.top + offset.y)) + rect.top;
		}

		return SetCursorPos(X, Y);
	}

	BOOL __stdcall ClientToScreenHook(HWND hWnd, LPPOINT lpPoint)
	{
		if (hWnd == hWndMain)
			return TRUE;

		return ClientToScreen(hWnd, lpPoint);
	}

	INT __stdcall GetDeviceCapsHook(HDC hdc, INT index)
	{
		if (index == BITSPIXEL)
			return 16;

		return GetDeviceCaps(hdc, index);
	}

	IID CLSID_DirectDraw = { 0xD7B70EE0, 0x4340, 0x11CF, 0xB0, 0x63, 0x00, 0x20, 0xAF, 0xC2, 0xCD, 0x35 };

	HRESULT __stdcall CoCreateInstanceHook(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID* ppv)
	{
		if (!MemoryCompare((VOID*)&rclsid, &CLSID_DirectDraw, sizeof(IID)))
			return Main::DrawCreateEx(NULL, ppv, riid, NULL);

		return CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
	}

	BOOL Load()
	{
		hModule = GetModuleHandle(NULL);
		PIMAGE_NT_HEADERS headNT = (PIMAGE_NT_HEADERS)((BYTE*)hModule + ((PIMAGE_DOS_HEADER)hModule)->e_lfanew);
		baseOffset = (INT)hModule - (INT)headNT->OptionalHeader.ImageBase;

		{
			MappedFile file = { hModule, NULL, NULL, NULL };
			{
				PatchFunction(&file, "GetDeviceCaps", GetDeviceCapsHook);
				PatchFunction(&file, "GetForegroundWindow", GetForegroundWindowHook);

				PatchFunction(&file, "CreateWindowExA", CreateWindowExHook);
				PatchFunction(&file, "RegisterClassA", RegisterClassHook);

				PatchFunction(&file, "SetWindowLongA", SetWindowLongHook);
				PatchFunction(&file, "ShowWindow", ShowWindowHook);

				PatchFunction(&file, "MessageBoxA", MessageBoxHook);

				PatchFunction(&file, "ShowCursor", ShowCursorHook);
				PatchFunction(&file, "ClipCursor", ClipCursorHook);

				PatchFunction(&file, "GetClientRect", GetClientRectHook);
				PatchFunction(&file, "GetWindowRect", GetWindowRectHook);

				if (config.version)
				{
					PatchFunction(&file, "CoCreateInstance", CoCreateInstanceHook);
					PatchFunction(&file, "GetCursorPos", GetCursorPosHookV1);
					PatchFunction(&file, "ClientToScreen", ClientToScreenHook);
				}
				else
				{
					PatchFunction(&file, "DirectDrawEnumerateExA", Main::DrawEnumerateEx);
					PatchFunction(&file, "DirectDrawCreate", Main::DrawCreate);
					PatchFunction(&file, "DirectDrawCreateEx", Main::DrawCreateEx);
					PatchFunction(&file, "GetCursorPos", GetCursorPosHookV2);
					PatchFunction(&file, "SetCursorPos", SetCursorPosHook);
				}
			}

			if (file.address)
				UnmapViewOfFile(file.address);

			if (file.hMap)
				CloseHandle(file.hMap);

			if (file.hFile)
				CloseHandle(file.hFile);
		}

		return TRUE;
	}
}