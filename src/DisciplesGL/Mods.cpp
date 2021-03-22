/*
	MIT License

	Copyright (c) 2021 Oleksiy Ryabchun

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
#include "Config.h"
#include "Window.h"
#include "Mods.h"
#include "Resource.h"

Mod* mods;

namespace Mods
{
	VOID Load()
	{
		CHAR file[MAX_PATH];
		GetModuleFileName(NULL, file, sizeof(file));
		CHAR* p = StrLastChar(file, '\\');
		if (!p)
			return;
		StrCopy(++p, "mods\\");
		p += StrLength(p);
		StrCopy(p, "*.mod");

		WIN32_FIND_DATA fData;
		HANDLE hFile = FindFirstFile(file, &fData);
		if (hFile && hFile != INVALID_HANDLE_VALUE)
		{
			do
			{
				StrCopy(p, fData.cFileName);
				HMODULE hModule = LoadLibrary(file);
				if (hModule)
				{
					Mod* mod = (Mod*)MemoryAlloc(sizeof(Mod));
					if (mod)
					{
						mod->hWnd = NULL;
						mod->GetName = (GETNAME)GetProcAddress(hModule, "GetName");
						mod->GetMenu = (GETMENU)GetProcAddress(hModule, "GetMenu");
						mod->SetHWND = (SETHWND)GetProcAddress(hModule, "SetHWND");
						mod->DrawFrame = (DRAWFRAME)GetProcAddress(hModule, "DrawFrame");

						if (mod->GetName && mod->GetMenu && mod->SetHWND && mod->DrawFrame)
						{
							mod->last = mods;
							mods = mod;
							continue;
						}
						
						MemoryFree(mod);
					}

					FreeLibrary(hModule);
				}
			} while (FindNextFile(hFile, &fData));

			FindClose(hFile);
		}

		MenuItemData mData;
		mData.childId = IDM_MODS;
		if (Window::GetMenuByChildID(&mData) && DeleteMenu(config.menu, IDM_MODS, MF_BYCOMMAND))
		{
			BOOL added = FALSE;
			Mod* mod = mods;
			while (mod)
			{
				HMENU hMenu = mod->GetMenu();
				if (hMenu && InsertMenu(mData.hMenu, 0, MF_BYPOSITION | MF_POPUP, (UINT_PTR)hMenu, mod->GetName()))
					added = TRUE;

				mod = mod->last;
			}

			if (!added)
				DeleteMenu(config.menu, mData.index, MF_BYPOSITION);
		}
	}

	VOID SetHWND(HWND hWnd)
	{
		Mod* mod = mods;
		while (mod)
		{
			if (!mod->hWnd)
			{
				mod->hWnd = hWnd;
				mod->SetHWND(hWnd);
			}
			mod = mod->last;
		}
	}

	VOID DrawFrame(DWORD width, DWORD height, DWORD format, DWORD pitch, VOID* buffer)
	{
		Mod* mod = mods;
		while (mod)
		{
			mod->DrawFrame(width, height, format, pitch, buffer);
			mod = mod->last;
		}
	}
}