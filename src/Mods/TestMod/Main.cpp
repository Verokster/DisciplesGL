/*
	MIT License

	Copyright (c) 2020 Oleksiy Ryabchun

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
#include "Main.h"
#include "Config.h"
#include "Window.h"
#include "TextRenderer.h"
#include "Resource.h"

namespace Main
{
	const CHAR* __stdcall GetName()
	{
		return "Test Mod";
	}

	VOID ChangeMenuID(HMENU hMenu, MENUITEMINFO* info)
	{
		INT count = GetMenuItemCount(hMenu);
		for (INT i = 0; i < count; ++i)
		{
			info->fMask = MIIM_ID | MIIM_SUBMENU | MIIM_FTYPE;
			if (GetMenuItemInfo(hMenu, i, TRUE, info))
			{
				if (info->fType & MFT_SEPARATOR)
					continue;

				if (!info->hSubMenu)
				{
					info->fMask = MIIM_ID;
					info->wID += config.index;
					SetMenuItemInfo(hMenu, i, TRUE, info);
				}
				else
					ChangeMenuID(info->hSubMenu, info);
			}
		}
	}

	HMENU __stdcall GetModMenu(DWORD offset)
	{
		config.index = offset;
		HMENU hMenu = LoadMenu(config.hModule, MAKEINTRESOURCE(IDR_MENU));

		MENUITEMINFO info = {};
		info.cbSize = sizeof(MENUITEMINFO);
		ChangeMenuID(hMenu, &info);

		Window::CheckMenu(hMenu);
		return hMenu;
	}

	VOID __stdcall SetHWND(HWND hWnd)
	{
		config.hWnd = hWnd;
		Window::SetCaptureWindow(hWnd);
	}

	VOID __stdcall DrawFrame(DWORD width, DWORD height, LONG pitch, DWORD pixelFormat, VOID* buffer)
	{
		if (!config.state.enabled)
			return;

		const RECT padShadow = { 11, 7, 9, 3 };
		const RECT padText = { 10, 5, 10, 5 };

		const FrameType* frame = (const FrameType*)&width;

		CHAR time[256];

		if (!config.state.paused)
			config.tick.end = GetTickCount();

		DWORD tick = config.tick.end - config.tick.begin;
		sprintf(time, "%02d:%02d:%02d", tick / 3600000, (tick % 3600000) / 60000, (tick % 60000) / 1000);

		DWORD align;
		switch (config.displayCorner)
		{
		case DisplayTopRight:
			align = DT_TOP | DT_RIGHT;
			break;
		case DisplayBottomLeft:
			align = DT_BOTTOM | DT_LEFT;
			break;
		case DisplayBottomRight:
			align = DT_BOTTOM | DT_RIGHT;
			break;
		default:
			align = DT_TOP | DT_LEFT;
			break;
		}

		COLORREF color, shadow;
		if (!config.state.paused && tick / 1000)
		{
			color = RGB(209, 174, 9);
			shadow = RGB(104, 16, 0);
		}
		else
		{
			color = RGB(255, 255, 255);
			shadow = RGB(96, 96, 96);
		}

		if (!(pixelFormat & PIXEL_BGR))
		{
			shadow = _byteswap_ulong(_rotl(shadow, 8));
			color = _byteswap_ulong(_rotl(color, 8));
		}

		txt->Draw(time, shadow, RGB(0, 0, 0), align, &padShadow, frame);
		txt->Draw(time, color, RGB(0, 0, 0), align, &padText, frame);
	}
}