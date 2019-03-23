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

	AddressSpace addressArray[] = {
		// Dark Prophecy return - v1.00
		0x00557B85, 0x005ADFCC, 0x0059A30B, 0x005AECAF, 0x006622BE, 0x0066242E, 0x006D3728,
		0x00552876, 0x0066378C, 0x00552A0C, 0x006CD194, 0x005A98B0, 0x005A97B0,
		0x0051E67B, 0x005A10A1, 0x006CCD00, 0x005261BF, 0x00526258,
		0x0052DBEE, 0x0052DC2D, 0x0052DC36, 0x0052DC88,

		// v1.40
		0x0055DFDD, 0x005B4516, 0x005A0826, 0x005B5216, 0x0066BEBB, 0x0066C014, 0x006DFD98,
		0x00558982, 0x0066D37F, 0x00558B18, 0x006D950C, 0x005AFD70, 0x005AFC70,
		0x00523979, 0x005A740F, 0x006D9078, 0x0052BD52, 0x0052BDEB,
		0x005339AF, 0x005339EE, 0x005339F7, 0x00533A49,

		// v2.01
		0x00563A39, 0x005BB2A9, 0x005A7434, 0x005BC059, 0x006758C0, 0x00675A27, 0x006EA4E8,
		0x0055E55E, 0x00676D96, 0x0055E6F4, 0x006E398C, 0x005B69F0, 0x005B68F0,
		0x00529178, 0x005AE075, 0x006E34C8, 0x005317C9, 0x00531862,
		0x00539521, 0x00539560, 0x00539569, 0x005395BB,

		// Galean Return - v2.02
		0x00562857, 0x005B9F86, 0x005A6311, 0x005BACE1, 0x0067427E, 0x00674400, 0x006E9408,
		0x0055D283, 0x00675771, 0x0055D419, 0x006E2894, 0x005B5660, 0x005B5560,
		0x005280D9, 0x005ACF11, 0x006E23D0, 0x0053056A, 0x00530603,
		0x0053835D, 0x0053839C, 0x005383A5, 0x005383F7,

		// v3.01a
		0x00566C02, 0x005BF7E4, 0x005AB6D8, 0x005C0654, 0x0067F2BE, 0x0067F42A, 0x006F5E50,
		0x005616BE, 0x006807FE, 0x00561854, 0x006EEE74, 0x005BAFC0, 0x005BAEC0,
		0x0052CAA6, 0x005B22A1, 0x006EE9A8, 0x00535048, 0x005350E1,
		0x0053CE21, 0x0053CE60, 0x0053CE69, 0x0053CEBB,

		// v3.01b
		0x0056638C, 0x005BE82F, 0x005AA960, 0x005BF589, 0x0067DC5A, 0x0067DDDC, 0x006F3E00,
		0x00560E5B, 0x0067F172, 0x00560FF1, 0x006ECE14, 0x005B9F70, 0x005B9E70,
		0x0052C03C, 0x005B15A2, 0x006EC948, 0x00534626, 0x005346BF,
		0x0053C4CD, 0x0053C50C, 0x0053C515, 0x0053C567
	};

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

#pragma region 32 BPP
#define COLORKEY_AND 0x00F8FCF8
#define COLORKEY_CHECK 0x00F800F8

	DWORD pBinkCopyToBuffer;
	INT __stdcall BinkCopyToBufferHook(VOID* hBnk, VOID* dest, INT pitch, DWORD height, DWORD x, DWORD y, DWORD flags)
	{
		if (config.bpp32Hooked)
		{
			pitch <<= 1;
			flags = 0x80000004;
		}

		return ((INT(__stdcall*)(VOID*, VOID*, INT, DWORD, DWORD, DWORD, DWORD))pBinkCopyToBuffer)(hBnk, dest, pitch, height, x, y, flags);
	}

	BlendData blendList[32];

#pragma region Memory
	DWORD back_005A6316;
	VOID __declspec(naked) hook_005A6311()
	{
		__asm
		{
			SHL EAX, 1
			IMUL EAX, DWORD PTR DS : [ESI + 0x4]
			PUSH EBX

			JMP back_005A6316
		}
	}

	DWORD back_005BACE7;
	VOID __declspec(naked) hook_005BACE1()
	{
		__asm
		{
			SHL EAX, 2
			IMUL EAX, DWORD PTR SS : [EBP - 0x14]

			JMP back_005BACE7
		}
	}

	DWORD back_00674283;
	VOID __declspec(naked) hook_0067427E()
	{
		__asm
		{
			SHL EAX, 1
			IMUL EAX, DWORD PTR SS : [EDI + 0x4]
			PUSH EAX

			JMP back_00674283
		}
	}

	DWORD back_00674405;
	VOID __declspec(naked) hook_00674400()
	{
		__asm
		{
			SHL EAX, 1
			IMUL EAX, DWORD PTR SS : [EDI + 0x4]
			PUSH EAX

			JMP back_00674405
		}
	}
#pragma endregion

#pragma region PIXEL
	VOID __stdcall Pixel_Blit_Indexed_to_565(BYTE* srcData, DWORD srcPitch, DWORD* palette, DWORD* dstData, DWORD dstPitch, RECT *rect)
	{
		dstPitch >>= 1;

		BYTE* lpSrc = srcData + rect->top * srcPitch + rect->left;
		DWORD* lpDst = dstData + rect->top * dstPitch + rect->left;

		DWORD height = rect->bottom - rect->top;
		do
		{
			BYTE* src = lpSrc;
			DWORD* dst = lpDst;

			DWORD width = rect->right - rect->left;
			do
				*dst++ = palette[*src++] & 0x00FFFFFF;
			while (--width);

			lpSrc += srcPitch;
			lpDst += dstPitch;
		} while (--height);
	}

	DWORD __stdcall Pixel_ConvertPixel_565(BYTE red, BYTE green, BYTE blue)
	{
		return  (red >> 3) | (green >> 3 << 6) | (blue >> 3 << 11);
	}

	VOID __stdcall Pixel_Blit_By_Masks(DWORD* srcData, LONG srcPitch, DWORD redMask, DWORD greenMask, DWORD blueMask, DWORD alphaMask, BYTE* dstData, LONG dstPitch, RECT* rect)
	{
		if (srcData && dstData && (VOID*)srcData != (VOID*)dstData &&
			rect->left >= 0 && rect->left < rect->right && rect->top >= 0 && rect->top < rect->bottom)
		{
			srcPitch >>=  1;
			if (rect->right <= srcPitch && rect->right <= dstPitch / 3)
			{
				srcData += rect->top * srcPitch + rect->left * 2;
				dstData += rect->top * dstPitch + rect->left * 3;

				LONG height = rect->bottom - rect->top;
				while (height--)
				{
					DWORD* src = srcData;
					BYTE* dst = dstData;

					LONG width = rect->right - rect->left;
					while (width--)
					{
						*dst++ = LOBYTE(*src >> 16);
						*dst++ = LOBYTE(*src >> 8);
						*dst++ = LOBYTE(*src);

						++src;
					}

					srcData += srcPitch;
					dstData += dstPitch;
				}
			}
		}
	}

	VOID __stdcall Pixel_ConvertPixel_565_to_RGB(DWORD color, BYTE* red, BYTE* green, BYTE* blue)
	{
		*red = LOBYTE(color);
		*green = LOBYTE(color >> 8);
		*blue = LOBYTE(color >> 16);
	}

	VOID __stdcall Pixel_Blit_RGB_to_565(BYTE* srcData, DWORD srcPitch, DWORD* dstData, DWORD dstPitch, RECT* rect)
	{
		dstPitch >>= 1;

		srcData += rect->left + rect->top * srcPitch;
		dstData += rect->left + rect->top * dstPitch;

		DWORD height = rect->bottom - rect->top;
		do
		{
			BYTE* src = srcData;
			DWORD* dst = dstData;

			DWORD width = rect->right - rect->left;
			do
			{
				BYTE red = *src++;
				BYTE green = *src++;
				BYTE blue = *src++;

				*dst++ = red | (green << 8) | (blue << 16);
			} while (--width);

			srcData += srcPitch;
			dstData += dstPitch;
		} while (--height);
	}

	VOID __stdcall Pixel_RGB_Swap(BYTE* data, LONG pitch, SIZE *size)
	{
		/*DWORD height = size->cy;
		while (height--)
		{
			BYTE* px = data;

			DWORD width = size->cx;
			while (width--)
			{
				BYTE temp = *(px + 2);
				*(px + 2) = *px;
				*px = temp;

				px += 3;
			}

			data += pitch;
		}*/
	}

	DWORD __stdcall Pixel_BlendSome(DWORD pix, BYTE b, BYTE g, BYTE r, BYTE msk)
	{
		if (msk == 255)
			return r | (g << 8) | (b << 16);
		else if (msk)
		{
			DWORD px = pix & 0xFF;
			DWORD red = (((px * 255) + (r - px) * msk) / 255) & 0xFF;

			px = (pix >> 8) & 0xFF;
			DWORD green = ((((px * 255) + (g - px) * msk) / 255) & 0xFF) << 8;

			px = (pix >> 16) & 0xFF;
			DWORD blue = ((((px * 255) + (b - px) * msk) / 255) & 0xFF) << 16;

			return red | green | blue;
		}
		else
			return pix;
	}

	DWORD __stdcall Pixel_Blend(DWORD dstData, DWORD srcData, BYTE msk)
	{
		if (msk == 255)
			return srcData;
		else if (msk)
		{
			DWORD src = srcData & 0x000000FF;
			DWORD dst = dstData & 0x000000FF;
			DWORD red = (((dst * 255) + (src - dst) * msk) / 255) & 0x000000FF;

			src = srcData & 0x0000FF00;
			dst = dstData & 0x0000FF00;
			DWORD green = (((dst * 255) + (src - dst) * msk) / 255) & 0x0000FF00;

			src = srcData & 0x00FF0000;
			dst = dstData & 0x00FF0000;
			DWORD blue = (((dst * 255) + (src - dst) * msk) / 255) & 0x00FF0000;

			return red | green | blue;
		}
		else
			return dstData;
	}

	VOID __stdcall Pixel_BlitBlend(DWORD* srcData, DWORD* dstData, DWORD count, BYTE* mskData)
	{
		while (count--)
		{
			DWORD msk = *mskData;

			if (msk == 255)
				*dstData = *srcData;
			else if (msk)
			{
				DWORD src = *srcData & 0x000000FF;
				DWORD dst = *dstData & 0x000000FF;
				DWORD red = (((dst * 255) + (src - dst) * msk) / 255) & 0x000000FF;

				src = *srcData & 0x0000FF00;
				dst = *dstData & 0x0000FF00;
				DWORD green = (((dst * 255) + (src - dst) * msk) / 255) & 0x0000FF00;

				src = *srcData & 0x00FF0000;
				dst = *dstData & 0x00FF0000;
				DWORD blue = (((dst * 255) + (src - dst) * msk) / 255) & 0x00FF0000;

				*dstData = red | green | blue;
			}

			++srcData;
			++dstData;
			++mskData;
		}
	}

	VOID __stdcall Pixel_BlitBlendWithColorKey(BlendData* blendItem, DWORD count, DWORD colorKey)
	{
		colorKey = ((colorKey & 0x001F) << 3) | ((colorKey & 0x07E0) << 5) | ((colorKey & 0xF800) << 8);
		while (count--)
		{
			DWORD* srcData = blendItem->srcData;
			DWORD* dstData = blendItem->dstData;
			BYTE* mskData = (BYTE*)blendItem->mskData;

			DWORD length = blendItem->length;
			while (length--)
			{
				DWORD msk = *mskData;
				if (msk == 255)
				{
					if ((*dstData & COLORKEY_AND) != colorKey)
						*dstData = *srcData;
				}
				else if (msk)
				{
					if ((*dstData & COLORKEY_AND) != colorKey)
					{
						DWORD src = *srcData & 0x000000FF;
						DWORD dst = *dstData & 0x000000FF;
						DWORD red = (((dst * 255) + (src - dst) * msk) / 255) & 0x000000FF;

						src = *srcData & 0x0000FF00;
						dst = *dstData & 0x0000FF00;
						DWORD green = (((dst * 255) + (src - dst) * msk) / 255) & 0x0000FF00;

						src = *srcData & 0x00FF0000;
						dst = *dstData & 0x00FF0000;
						DWORD blue = (((dst * 255) + (src - dst) * msk) / 255) & 0x00FF0000;

						*dstData = red | green | blue;
					}
				}

				++srcData;
				++dstData;
				++mskData;
			}

			++blendItem;
		}
	}

	VOID __stdcall Pixel_BlitBlendAvarage(DWORD* srcData, LONG srcPitch, POINT* srcPos, DWORD* dstData, LONG dstPitch, POINT* dstPos, SIZE *size, BYTE flag, DWORD colorKey)
	{
		if (!flag || flag == 0xFF)
			return;

		colorKey = ((colorKey & 0x001F) << 3) | ((colorKey & 0x07E0) << 5) | ((colorKey & 0xF800) << 8);
		srcPitch >>= 1;
		dstPitch >>= 1;

		srcData += srcPitch * srcPos->y + srcPos->x;
		dstData += dstPitch * dstPos->y + dstPos->x;

		DWORD height = size->cy;
		while (height--)
		{
			DWORD* src = srcData;
			DWORD* dst = dstData;

			DWORD width = size->cx;
			while (width--)
			{
				if ((*src & COLORKEY_AND) != colorKey)
					*dst = Pixel_Blend(*dst, *src, 128);

				++src;
				++dst;
			}

			srcData += srcPitch;
			dstData += dstPitch;
		}
	}

	VOID __stdcall Pixel_Add(DWORD* srcData, LONG srcPitch, POINT* srcPos, DWORD* dstData, LONG dstPitch, POINT* dstPos, SIZE *size)
	{
		srcPitch >>= 1;
		dstPitch >>= 1;

		srcData += srcPitch * srcPos->y + srcPos->x;
		dstData += dstPitch * dstPos->y + dstPos->x;

		DWORD height = size->cy;
		do
		{
			DWORD* src = srcData;
			DWORD* dst = dstData;

			DWORD width = size->cx;
			do
			{
				DWORD cs = *src++;
				DWORD cd = *dst;

				DWORD r = (cd & 0xFF) + (cs & 0xFF);
				if (r > 0xFF)
					r = 0xFF;

				DWORD g = (cd & 0xFF00) + (cs & 0xFF00);
				if (g > 0xFF00)
					g = 0xFF00;

				DWORD b = (cd & 0xFF0000) + (cs & 0xFF0000);
				if (b > 0xFF0000)
					b = 0xFF0000;

				*dst++ = r | g | b;
			} while (--width);

			srcData += srcPitch;
			dstData += dstPitch;
		} while (--height);
	}

	VOID __stdcall Pixel_Sub(DWORD* srcData, LONG srcPitch, POINT* srcPos, DWORD* dstData, LONG dstPitch, POINT* dstPos, SIZE *size)
	{
		srcPitch >>= 1;
		dstPitch >>= 1;

		srcData += srcPitch * srcPos->y + srcPos->x;
		dstData += dstPitch * dstPos->y + dstPos->x;

		DWORD height = size->cy;
		do
		{
			DWORD* src = srcData;
			DWORD* dst = dstData;

			DWORD width = size->cx;
			do
			{
				DWORD cs = *src++;
				DWORD cd = *dst;

				INT r = (cd & 0xFF) - (cs & 0xFF);
				if (r < 0)
					r = 0;

				INT g = (cd & 0xFF00) - (cs & 0xFF00);
				if (g > 0)
					g = 0;

				INT b = (cd & 0xFF0000) - (cs & 0xFF0000);
				if (b > 0)
					b = 0;

				*dst++ = r | g | b;
			} while (--width);

			srcData += srcPitch;
			dstData += dstPitch;
		} while (--height);
	}

	VOID __stdcall Pixel_BlitColorKey(DWORD* srcData, LONG srcPitch, POINT* srcPos, DWORD* dstData, LONG dstPitch, POINT* dstPos, SIZE *size, BYTE flag, DWORD colorKey)
	{
		if (!flag || flag == 0xFF)
			return;

		colorKey = ((colorKey & 0x001F) << 3) | ((colorKey & 0x07E0) << 5) | ((colorKey & 0xF800) << 8);
		srcPitch >>= 1;
		dstPitch >>= 1;

		srcData += srcPitch * srcPos->y + srcPos->x;
		dstData += dstPitch * dstPos->y + dstPos->x;

		DWORD height = size->cy;
		while (height--)
		{
			DWORD* src = srcData;
			DWORD* dst = dstData;

			DWORD width = size->cx;
			while (width--)
			{
				if ((*src & COLORKEY_AND) != colorKey)
					*dst = *src;

				++src;
				++dst;
			}

			srcData += srcPitch;
			dstData += dstPitch;
		}
	}

	VOID __stdcall Pixel_BlitEmptyColor(DWORD* srcData, LONG srcPitch, POINT* srcPos, DWORD* dstData, LONG dstPitch, POINT* dstPos, SIZE *size)
	{
		srcPitch >>= 1;
		dstPitch >>= 1;

		srcData += srcPitch * srcPos->y + srcPos->x;
		dstData += dstPitch * dstPos->y + dstPos->x;

		DWORD height = size->cy;
		while (height--)
		{
			DWORD* src = srcData;
			DWORD* dst = dstData;

			DWORD width = size->cx;
			while (width--)
			{
				if (*src)
					*dst = *src;

				++src;
				++dst;
			}

			srcData += srcPitch;
			dstData += dstPitch;
		}
	}

	VOID __stdcall Pixel_DoubleLighter(DWORD* data, LONG pitch, SIZE* size, DWORD colorKey, BOOL flag)
	{
		if (!flag)
			return;

		colorKey = ((colorKey & 0x001F) << 3) | ((colorKey & 0x07E0) << 5) | ((colorKey & 0xF800) << 8);
		pitch >>= 1;

		DWORD height = size->cy;
		while (height--)
		{
			DWORD* ptr = data;

			DWORD width = size->cx;
			while (width--)
			{
				if ((*ptr & COLORKEY_AND) == colorKey)
					*ptr = 0;
				/*else if (*ptr)
				{
					DWORD r = (*ptr & 0xFF) << 1;
					if (r > 0xFF)
						r = 0xFF;

					DWORD g = (*ptr & 0xFF00) << 1;
					if (g > 0xFF00)
						g = 0xFF00;

					DWORD b = (*ptr & 0xFF0000) << 1;
					if (b > 0xFF0000)
						b = 0xFF0000;

					*ptr = r | g | b;
				}*/

				++ptr;
			}

			data += pitch;
		}
	}
#pragma endregion

	BOOL __fastcall Clip(LONG* shiftX, LONG* shiftY, LONG* left, LONG* top, LONG* width, LONG* height, RECT* clipper)
	{
		if (*left < clipper->left)
		{
			*width += *left - clipper->left;
			*shiftX += clipper->left - *left;
			*left = clipper->left;
		}

		if (*left + *width > clipper->right)
			*width = clipper->right - *left;

		if (*top < clipper->top)
		{
			*height += *top - clipper->top;
			*shiftY += clipper->top - *top;
			*top = clipper->top;
		}

		if (*top + *height > clipper->bottom)
			*height = clipper->bottom - *top;

		return *width > 0 && *height > 0;
	}

	VOID __stdcall DrawMinimapObjects(BlitObject* obj, VOID* data, RECT* rect, DWORD pitch, DWORD colorKey, POINT* loc)
	{
		if (data && rect->left <= rect->right && rect->top <= rect->bottom &&
			(colorKey <= (obj->isTrueColor ? 0xFFFFu : 0xFFu) || colorKey == 0xFFFFFFFF))
		{
			LONG shiftX = rect->left;
			LONG shiftY = rect->top;

			LONG left = loc->x;
			LONG top = loc->y;

			LONG width = rect->right - rect->left;
			LONG height = rect->bottom - rect->top;

			if (Clip(&shiftX, &shiftY, &left, &top, &width, &height, &obj->rect))
			{
				if (colorKey == 0xFFFFFFFF)
				{
					if (obj->isTrueColor)
					{
						colorKey = ((colorKey & 0x001F) << 3) | ((colorKey & 0x07E0) << 5) | ((colorKey & 0xF800) << 8);

						DWORD srcPitch = pitch >> 1;
						DWORD dstPitch = obj->pitch >> 1;

						DWORD* srcData = (DWORD*)data + shiftY * srcPitch + shiftX;
						DWORD* dstData = (DWORD*)obj->data + top * dstPitch + left;

						do
						{
							DWORD* src = srcData;
							DWORD* dst = dstData;

							DWORD count = width;
							do
							{
								if ((*src & COLORKEY_AND) != colorKey)
									*dst = _byteswap_ulong(_rotl(*src, 8));

								++src;
								++dst;
							} while (--count);

							dstData += dstPitch;
							srcData += srcPitch;
						} while (--height);
					}
					else
					{
						DWORD srcPitch = pitch;
						DWORD dstPitch = obj->pitch;

						BYTE* srcData = (BYTE*)data + shiftY * srcPitch + shiftX;
						BYTE* dstData = (BYTE*)obj->data + top * dstPitch + left;

						do
						{
							MemoryCopy(dstData, srcData, width);
							dstData += dstPitch;
							srcData += srcPitch;
						} while (--height);
					}
				}
				else
				{
					if (obj->isTrueColor)
					{
						colorKey = ((colorKey & 0x001F) << 3) | ((colorKey & 0x07E0) << 5) | ((colorKey & 0xF800) << 8);

						DWORD srcPitch = pitch >> 1;
						DWORD dstPitch = obj->pitch >> 1;

						DWORD* srcData = (DWORD*)data + shiftY * srcPitch + shiftX;
						DWORD* dstData = (DWORD*)obj->data + top * dstPitch + left;

						do
						{
							DWORD* src = srcData;
							DWORD* dst = dstData;

							DWORD count = width;
							do
							{
								if ((*src & COLORKEY_AND) != colorKey)
									*dst = _byteswap_ulong(_rotl(*src, 8));

								++src;
								++dst;
							} while (--count);

							dstData += dstPitch;
							srcData += srcPitch;
						} while (--height);
					}
					else
					{
						DWORD srcPitch = pitch;
						DWORD dstPitch = obj->pitch;

						BYTE* srcData = (BYTE*)data + shiftY * srcPitch + shiftX;
						BYTE* dstData = (BYTE*)obj->data + top * dstPitch + left;

						do
						{
							BYTE* src = srcData;
							BYTE* dst = dstData;

							DWORD count = width;
							do
							{
								if (*src != colorKey)
									*dst = *src;

								++src;
								++dst;
							} while (--count);

							dstData += dstPitch;
							srcData += srcPitch;
						} while (--height);
					}
				}
			}
		}
	}

	VOID __declspec(naked) hook_0055D419()
	{
		__asm
		{
			POP EAX
			PUSH ECX
			PUSH EAX
			JMP DrawMinimapObjects
		}
	}

	VOID __stdcall FillColor(BlitObject* obj, DWORD color, RECT *rect)
	{
		DWORD height = rect->bottom - rect->top;

		if (obj->isTrueColor)
		{
			DWORD pitch = obj->pitch >> 1;
			color = ((color & 0x001F) << 3) | ((color & 0x07E0) << 5) | ((color & 0xF800) << 8);

			DWORD* data = (DWORD*)obj->data;
			while (height--)
			{
				DWORD* ptr = data;
				DWORD width = rect->right - rect->left;
				while (width--)
					*ptr++ = color;

				data += pitch;
			}
		}
		else
		{
			DWORD pitch = obj->pitch;

			BYTE* data = (BYTE*)obj->data;
			while (height--)
			{
				BYTE* ptr = data;
				DWORD width = rect->right - rect->left;
				while (width--)
					*ptr++ = LOBYTE(color);

				data += pitch;
			}
		}
	}

	VOID __declspec(naked) hook_0055D283()
	{
		__asm
		{
			POP EAX
			PUSH ECX
			PUSH EAX
			JMP FillColor
		}
	}

	VOID __stdcall DrawWaterBorders(DWORD* thisObj, BlitObject* obj, POINT* loc, RECT* rect)
	{
		DWORD pitch = obj->pitch >> 1;

		DWORD* srcData = (*(DWORD*(__thiscall **)(DWORD*))((*thisObj) + 12))(thisObj);
		DWORD* dstData = (DWORD*)obj->data + loc->y * pitch;

		LONG offset = 30;
		LONG width = 2;

		LONG idx = 0;
		do
		{
			LONG y = loc->y + idx;
			if (y >= rect->top && y < rect->bottom)
			{
				LONG srcX = offset;
				LONG size = width;

				LONG dstX = loc->x + srcX;

				if (dstX < rect->left)
				{
					size += dstX - rect->left;
					srcX += rect->left - dstX;
					dstX = 0;
				}

				if (size >= rect->right - dstX)
					size = rect->right - dstX;

				if (size > 0)
				{
					DWORD* src = srcData + srcX;
					DWORD* dst = dstData + dstX;
					do
					{
						if ((*src & COLORKEY_AND) != COLORKEY_CHECK)
							*dst = *src;

						++src;
						++dst;
					} while (--size);
				}
			}

			if (++idx == 32)
				return;

			if (idx < 16)
			{
				offset -= 2;
				width += 4;
			}
			else if (idx != 16)
			{
				offset += 2;
				width -= 4;
			}

			srcData += 64;
			dstData += pitch;
		} while (TRUE);
	}

	VOID __declspec(naked) hook_005B5560()
	{
		__asm
		{
			POP EAX
			PUSH ECX
			PUSH EAX
			JMP DrawWaterBorders
		}
	}

	VOID __stdcall DrawGround(DWORD* thisObj, BlitObject* obj, POINT* srcLoc, POINT* dstLoc, RECT* rect, BYTE* blendMask, DWORD* alphaMask)
	{
		DWORD pitch = obj->pitch >> 1;

		DWORD* srcData = **(DWORD***)thisObj[1] + srcLoc->y * 192;
		DWORD* dstData = (DWORD*)obj->data + dstLoc->y * pitch;

		BlendData* blendItem = blendList;
		DWORD blendCount = 0;

		LONG offset = 30;
		LONG width = 2;

		LONG idx = 0;
		do
		{
			LONG srcY = srcLoc->y + idx;
			LONG dstY = dstLoc->y + idx;

			if (srcY >= 0 && srcY < 192 &&
				dstY >= rect->top && dstY < rect->bottom)
			{
				LONG srcX = offset + srcLoc->x;
				LONG dstX = offset + dstLoc->x;

				LONG off = offset;
				LONG size = width;

				if (srcX < 0)
				{
					size += srcX;
					off = offset - srcX;
					dstX -= srcX;
					srcX = 0;
				}

				if (dstX < rect->left)
				{
					size += dstX - rect->left;
					off += rect->left - dstX;
					srcX += rect->left - dstX;
					dstX = 0;
				}

				if (size >= 192 - srcX)
					size = 192 - srcX;

				if (size >= rect->right - dstX)
					size = rect->right - dstX;

				if (size > 0)
				{
					if (blendMask) // Water
					{
						blendItem->srcData = srcData + srcX;
						blendItem->dstData = dstData + dstX;
						blendItem->length = size;
						blendItem->mskData = blendMask + (idx * 64) + off;

						++blendCount;
						++blendItem;
					}
					else if (alphaMask)
					{
						DWORD* msk = alphaMask + (idx * 32) + off;
						DWORD* src = srcData + srcX;
						DWORD* dst = dstData + dstX;

						do
						{
							DWORD pix = *msk;
							if ((pix & COLORKEY_AND) != COLORKEY_CHECK)
							{
								if ((pix & COLORKEY_AND) == 0x00F8FC80)
									pix = *src;

								*dst = pix;
							}

							++msk;
							++src;
							++dst;
						} while (--size);
					}
					else // ground
						MemoryCopy(dstData + dstX, srcData + srcX, size * sizeof(DWORD));
				}
			}

			if (++idx == 32)
				break;

			if (idx < 16)
			{
				offset -= 2;
				width += 4;
			}
			else if (idx != 16)
			{
				offset += 2;
				width -= 4;
			}

			srcData += 192;
			dstData += pitch;
		} while (TRUE);

		if (blendCount)
			Pixel_BlitBlendWithColorKey(blendList, blendCount, 0xF81F);
	}

	VOID __declspec(naked) hook_005B5660()
	{
		__asm
		{
			POP EAX
			PUSH ECX
			PUSH EAX
			JMP DrawGround
		}
	}

	VOID __stdcall ClearGround(BlitObject* obj, POINT* loc, RECT* rect)
	{
		DWORD pitch = obj->pitch >> 1;

		DWORD* dstData = (DWORD*)obj->data + loc->y * pitch;

		LONG offset = 30;
		LONG width = 2;

		LONG idx = 0;
		do
		{
			LONG y = loc->y + idx;
			if (y >= rect->top && y < rect->bottom)
			{
				LONG dstX = loc->x + offset;
				LONG size = width;

				if (dstX < rect->left)
				{
					size += dstX - rect->left;
					dstX = 0;
				}

				if (size >= rect->right - dstX)
					size = rect->right - dstX;

				if (size > 0)
					MemoryZero(dstData + dstX, size * sizeof(DWORD));
			}

			if (++idx == 32)
				return;

			if (idx < 16)
			{
				offset -= 2;
				width += 4;
			}
			else if (idx != 16)
			{
				offset += 2;
				width -= 4;
			}

			dstData += pitch;
		} while (TRUE);
	}

	VOID __stdcall DrawSymbol(DWORD* obj, DWORD* data, LONG dstPitch, LONG left, LONG top, RECT* clipper, RECT* rect, DWORD colorFill, DWORD colorShadow, BYTE castShadow, CHAR symbol)
	{
		if (symbol != '\n' && symbol != '\r')
		{
			LONG* font = *(LONG**)(*obj + 4 * *(BYTE*)&symbol + 16);

			POINT shift = { 0, 0 };
			SIZE size = { font[0], font[1] };

			if (Clip(&shift.x, &shift.y, &left, &top, &size.cx, &size.cy, clipper))
			{
				colorFill = ((colorFill & 0x001F) << 3) | ((colorFill & 0x07E0) << 5) | ((colorFill & 0xF800) << 8);
				colorShadow = ((colorShadow & 0x001F) << 3) | ((colorShadow & 0x07E0) << 5) | ((colorShadow & 0xF800) << 8);

				DWORD srcPitch = font[3];
				BYTE* src = (BYTE*)font[4] + shift.y * srcPitch + (shift.x >> 3);

				DWORD mod = shift.x % 8;
				if (!mod)
					mod = 8;

				dstPitch >>= 1;
				DWORD* dst = (DWORD*)data + top * dstPitch + left;

				LONG height = size.cy;
				LONG y = top;
				do
				{
					BYTE* srcPtr = src;
					BYTE srcVal = *srcPtr;

					DWORD count = 8 - mod;
					while (count--)
						srcVal <<= 1;
					DWORD mask = mod;

					DWORD* dstPtr = dst;

					LONG width = size.cx;
					LONG x = left;
					do
					{
						if (srcVal & 0x80)
						{
							*dstPtr = colorFill;

							if (castShadow)
							{
								if (x > rect->left && *(dstPtr - 1) != colorFill)
									*(dstPtr - 1) = colorShadow;

								if (x < rect->right - 1 && *(dstPtr + 1) != colorFill)
									*(dstPtr + 1) = colorShadow;

								if (y > rect->top && *(dstPtr - dstPitch) != colorFill)
									*(dstPtr - dstPitch) = colorShadow;

								if (y < rect->bottom - 1 && *(dstPtr + dstPitch) != colorFill)
									*(dstPtr + dstPitch) = colorShadow;
							}
						}

						++x;

						if (!--mask)
						{
							mask = 8;
							srcVal = *(++srcPtr);
						}
						else
							srcVal <<= 1;

						++dstPtr;
					} while (--width);

					++y;
					src += srcPitch;
					dst += dstPitch;
				} while (--height);
			}
		}
	}

	VOID __declspec(naked) hook_005280D9()
	{
		__asm
		{
			POP EAX
			PUSH ECX
			PUSH EAX
			JMP DrawSymbol
		}
	}

	VOID __stdcall DrawLineHorizontal(DWORD* data, SIZE* sizePitch, LONG left, LONG top, LONG width, DWORD colorFill)
	{
		POINT shift = { 0, 0 };
		LONG height = 1;
		RECT clipper = { 0, 0, sizePitch->cx, sizePitch->cy };

		if (Clip(&shift.x, &shift.y, &left, &top, &width, &height, &clipper))
		{
			colorFill = ((colorFill & 0x001F) << 3) | ((colorFill & 0x07E0) << 5) | ((colorFill & 0xF800) << 8);

			DWORD* dst = (DWORD*)data + top * sizePitch->cx + left;
			do
				*dst++ = colorFill;
			while (--width);
		}
	}

	VOID __declspec(naked) hook_0053056A() { __asm { JMP DrawLineHorizontal } }

	VOID __stdcall DrawLineVertical(DWORD* data, SIZE* sizePitch, LONG left, LONG top, LONG height, DWORD colorFill)
	{
		POINT shift = { 0, 0 };
		LONG width = 1;
		RECT clipper = { 0, 0, sizePitch->cx, sizePitch->cy };

		if (Clip(&shift.x, &shift.y, &left, &top, &width, &height, &clipper))
		{
			colorFill = ((colorFill & 0x001F) << 3) | ((colorFill & 0x07E0) << 5) | ((colorFill & 0xF800) << 8);

			DWORD* dst = (DWORD*)data + top * sizePitch->cx + left;
			do
			{
				*dst = colorFill;
				dst += sizePitch->cx;
			} while (--height);
		}
	}

	VOID __declspec(naked) hook_00530603() { __asm { JMP DrawLineVertical } }

	VOID __stdcall DrawMinimapGround(DWORD* thisObj, LONG left, LONG top, BlitObject* obj)
	{
		if (left > 0 && left < obj->rect.right && top > 0 && top < obj->rect.bottom)
		{
			SIZE* size = (*(SIZE*(__thiscall **)(DWORD*))(*thisObj + 4))(thisObj);
			LONG pitch = (*(LONG(__thiscall **)(DWORD*))(*thisObj + 12))(thisObj);
			VOID* data = (*(VOID*(__thiscall **)(DWORD*))(*thisObj + 8))(thisObj);

			LONG shiftX = left % size->cx;
			if (shiftX >= size->cx - 4)
				shiftX -= size->cx - 4;

			LONG shiftY = top % size->cy;
			if (shiftY >= size->cy - 2)
				shiftY -= size->cy - 2;

			LONG srcPitch = pitch >> 1;
			DWORD* srcData = (DWORD*)data + shiftY * srcPitch + shiftX;

			LONG dstPitch = obj->pitch >> 1;
			DWORD* dstData = (DWORD*)obj->data + top * dstPitch + left;

			DWORD y = 1;
			do
			{
				DWORD* src = srcData;
				DWORD* dst = dstData;

				DWORD x = 3;
				do
				{
					if ((0x4E >> ((y << 2) + x)) & 1)
						*dst = _byteswap_ulong(_rotl(*src, 8));

					++src;
					++dst;
				} while (x--);

				srcData += srcPitch;
				dstData += dstPitch;
			} while (y--);
		}
	}

	VOID __stdcall DrawCastleBuildings(DWORD* thisObj, BlitObject* obj)
	{
		obj->color = 0xF81F;
		FillColor(obj, obj->color, &obj->rect);

		LONG width = thisObj[12];
		LONG height = thisObj[13];
		if (height > 0 && width > 0)
		{
			BOOL isBlend;
			DWORD blend;

			switch (thisObj[14])
			{
			case 1: // selected line
				isBlend = TRUE;
				blend = 0xC40000;
				break;

			case 2: // unavailable yet
				isBlend = TRUE;
				blend = 0x0000FF;
				break;

			case 3: // other line
				isBlend = TRUE;
				blend = 0x000000;
				break;

			default:
				isBlend = FALSE;
				break;
			}

			LONG srcPitch = width;
			LONG dstPitch = obj->pitch >> 1;

			DWORD* srcData = *((DWORD**)thisObj[11] + 1);
			DWORD* dstData = (DWORD*)obj->data;

			while (height--)
			{
				DWORD* src = srcData;
				DWORD* dst = dstData;

				INT count = width;
				while (count--)
				{
					DWORD pix = *src;
					if ((pix & COLORKEY_AND) != COLORKEY_CHECK)
					{
						if (isBlend)
							pix = Pixel_Blend(pix, blend, 128);

						*dst = pix;
					}

					++src;
					++dst;
				}

				srcData += srcPitch;
				dstData += dstPitch;
			}
		}

		*(BYTE*)&thisObj[9] = 0;
	}

	VOID __stdcall DrawFaces(VOID* dstData, LONG dstPitch, VOID* srcData, LONG srcPitch, LONG top, LONG bottom, LONG width, BYTE isMirror, BYTE k, DWORD minIdx, PixObject* pixData_1, PixObject* pixData_2, VOID* mskData, LONG mskPitch)
	{
		DWORD* source = (DWORD*)srcData + top * srcPitch;
		DWORD* mask = (DWORD*)mskData + top * mskPitch;
		DWORD* destination = (DWORD*)dstData + top * dstPitch + (isMirror ? width - 1 : 0);

		DWORD idx = 0;

		DWORD height = bottom - top;
		while (height--)
		{
			DWORD* src = source;
			DWORD* msk = mask;
			DWORD* dst = destination;

			LONG count = width;
			while (count--)
			{
				DWORD pix = *src;
				if (mskData || pixData_1->exists)
				{
					if (k == 255)
						pix = mskData ? *msk : pixData_1->color;
					else if (k)
					{
						DWORD mult = mskData ? *msk : pixData_1->color;

						DWORD r1 = pix & 0x000000FF;
						DWORD r2 = mult & 0x000000FF;
						DWORD red = (((r1 * 255) + (r2 - r1) * k) / 255) & 0x000000FF;

						r1 = pix & 0x0000FF00;
						r2 = mult & 0x0000FF00;
						DWORD green = (((r1 * 255) + (r2 - r1) * k) / 255) & 0x0000FF00;

						r1 = pix & 0x00FF0000;
						r2 = mult & 0x00FF0000;
						DWORD blue = (((r1 * 255) + (r2 - r1) * k) / 255) & 0x00FF0000;

						pix = red | green | blue;
					}
				}

				if (idx >= minIdx && pixData_2->exists) // color
				{
					if (k == 255)
						pix = pixData_2->color;
					else if (k)
					{
						DWORD mult = pixData_2->color;

						DWORD r1 = pix & 0x000000FF;
						DWORD r2 = mult & 0x000000FF;
						DWORD red = (((r1 * 255) + (r2 - r1) * k) / 255) & 0x000000FF;

						r1 = pix & 0x0000FF00;
						r2 = mult & 0x0000FF00;
						DWORD green = (((r1 * 255) + (r2 - r1) * k) / 255) & 0x0000FF00;

						r1 = pix & 0x00FF0000;
						r2 = mult & 0x00FF0000;
						DWORD blue = (((r1 * 255) + (r2 - r1) * k) / 255) & 0x00FF0000;

						pix = red | green | blue;
					}
				}

				*dst = pix;

				++src;
				++msk;

				if (isMirror)
					--dst;
				else
					++dst;
			}

			source += srcPitch;
			mask += mskPitch;
			destination += dstPitch;

			++idx;
		}
	}

	VOID __stdcall DrawLine(BlitObject* obj, POINT* loc, DWORD count, DWORD color)
	{
		DWORD pitch = obj->pitch >> 1;
		color = ((color & 0x001F) << 3) | ((color & 0x07E0) << 5) | ((color & 0xF800) << 8);

		DWORD* data = (DWORD*)obj->data + loc->y * pitch + loc->x;
		while (count--)
			*data++ = color;
	}

	DWORD __stdcall Color565toRGB(DWORD color)
	{
		return ((color & 0x001F) << 3) | ((color & 0x07E0) << 5) | ((color & 0xF800) << 8);
	}

	DWORD back_005383AA;
	VOID __declspec(naked) hook_005383A5()
	{
		__asm
		{
			MOV EAX, DWORD PTR SS : [EBP - 0x1C]
			PUSH EAX
			CALL Color565toRGB
			MOV DWORD PTR SS : [EBP - 0x1C], EAX

			MOV EAX, DWORD PTR SS : [EBP - 0x14]
			TEST EAX, EAX

			JMP back_005383AA
		}
	}

	DWORD back_00538413;
	VOID __declspec(naked) hook_005383F7()
	{
		__asm
		{
			SHL EBX, 0x1
			MOV ECX, EAX
			MOV EAX, DWORD PTR DS : [ECX + 0x4]
			IMUL EAX, EBX
			ADD ESI, EAX
			MOV EAX, DWORD PTR DS : [ECX]
			MOV ECX, DWORD PTR SS : [EBP - 0x1C]
			MOV DWORD PTR DS : [EAX * 0x4 + ESI], ECX
			INC EDI

			JMP back_00538413
		}
	}

#pragma endregion

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

					pBinkCopyToBuffer = PatchFunction(&file, "_BinkCopyToBuffer@28", BinkCopyToBufferHook);
				}
			}

			if (file.address)
				UnmapViewOfFile(file.address);

			if (file.hMap)
				CloseHandle(file.hMap);

			if (file.hFile)
				CloseHandle(file.hFile);
		}

		if (!config.version)
		{
			AddressSpace* hookSpace = addressArray;
			DWORD hookCount = sizeof(addressArray) / sizeof(AddressSpace);
			do
			{
				DWORD check;
				if (ReadDWord(hookSpace->check + 1, &check) && check == WS_POPUP)
				{
					config.bpp32Hooked = TRUE;

					DWORD* pixAddress = (DWORD*)hookSpace->pixel;
					PatchDWord((DWORD)(pixAddress++), (DWORD)Pixel_Blit_Indexed_to_565);
					PatchDWord((DWORD)(pixAddress++), (DWORD)Pixel_ConvertPixel_565);
					PatchDWord((DWORD)(pixAddress++), (DWORD)Pixel_Blit_By_Masks);
					PatchDWord((DWORD)(pixAddress++), (DWORD)Pixel_ConvertPixel_565_to_RGB);
					PatchDWord((DWORD)(pixAddress++), (DWORD)Pixel_Blit_RGB_to_565);
					PatchDWord((DWORD)(pixAddress++), (DWORD)Pixel_RGB_Swap);
					PatchDWord((DWORD)(pixAddress++), (DWORD)Pixel_BlendSome);
					PatchDWord((DWORD)(pixAddress++), (DWORD)Pixel_Blend);
					PatchDWord((DWORD)(pixAddress++), (DWORD)Pixel_BlitBlend);
					PatchDWord((DWORD)(pixAddress++), (DWORD)Pixel_BlitBlendWithColorKey);
					PatchDWord((DWORD)(pixAddress++), (DWORD)Pixel_BlitBlendAvarage);
					PatchDWord((DWORD)(pixAddress++), (DWORD)Pixel_Add);
					PatchDWord((DWORD)(pixAddress++), (DWORD)Pixel_Sub);
					PatchDWord((DWORD)(pixAddress++), (DWORD)Pixel_BlitColorKey);
					PatchDWord((DWORD)(pixAddress++), (DWORD)Pixel_BlitEmptyColor);
					PatchDWord((DWORD)(pixAddress++), (DWORD)Pixel_DoubleLighter);

					// GOOD =================================================================

					PatchHook(hookSpace->fillColor, hook_0055D283); // Fill color
					PatchCall(hookSpace->minimapGround, DrawMinimapGround); // Minimap ground
					PatchHook(hookSpace->minimapObjects, hook_0055D419); // Draw minimap object
					PatchDWord(hookSpace->clearGround, (DWORD)ClearGround); // Clear ground
					PatchHook(hookSpace->mapGround, hook_005B5660); // Draw map ground
					PatchHook(hookSpace->waterBorders, hook_005B5560); // Draw water borders

					PatchHook(hookSpace->symbol, hook_005280D9); // Draw Symbol
					PatchCall(hookSpace->faces, DrawFaces);
					PatchDWord(hookSpace->buildings, (DWORD)DrawCastleBuildings);

					PatchHook(hookSpace->horLine, hook_0053056A); // Draw Horizontal Line
					PatchHook(hookSpace->verLine, hook_00530603); // Draw Vertical Line

					// GOOD =================================================================

					PatchCall(hookSpace->line_1, DrawLine);
					PatchCall(hookSpace->line_2, DrawLine);

					PatchHook(hookSpace->unknown_1, hook_005383A5);
					back_005383AA = hookSpace->unknown_1 + 5 + baseOffset;

					PatchHook(hookSpace->unknown_2, hook_005383F7);
					back_00538413 = hookSpace->unknown_2 + 28 + baseOffset;

					// Increase memory
					{
						BYTE value;
						if (ReadByte(hookSpace->memory_1 + 2, &value))
							PatchByte(hookSpace->memory_1 + 2, value << 1);

						PatchHook(hookSpace->memory_2, hook_005A6311);
						back_005A6316 = hookSpace->memory_2 + 5 + baseOffset;

						PatchHook(hookSpace->memory_3, hook_005BACE1);
						back_005BACE7 = hookSpace->memory_3 + 6 + baseOffset;

						PatchHook(hookSpace->memory_4, hook_0067427E);
						back_00674283 = hookSpace->memory_4 + 5 + baseOffset;

						PatchHook(hookSpace->memory_5, hook_00674400);
						back_00674405 = hookSpace->memory_5 + 5 + baseOffset;
					}

					break;
				}

				++hookSpace;
			} while (--hookCount);
		}

		return TRUE;
	}
}