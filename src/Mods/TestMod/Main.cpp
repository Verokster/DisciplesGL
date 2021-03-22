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

	HMENU __stdcall GetMenu()
	{
		HMENU hMenu = LoadMenu(config.hModule, MAKEINTRESOURCE(IDR_MENU));
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
		if (!config.enabled || !(pixelFormat & PIXEL_TRUECOLOR))
			return;

		const RECT padShadow = { 11, 7, 9, 3 };
		const RECT padText = { 10, 5, 10, 5 };

		const FrameType* frame = (const FrameType*)&width;

		CHAR time[256];
		DWORD tick = GetTickCount() - config.tick;
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

		txt->Draw(time, RGB(10, 10, 10), RGB(0, 0, 0), align, &padShadow, frame);

		DWORD color = tick / 1000 ? RGB(255, 255, 255) : RGB(255, 255, 0);
		if (!(pixelFormat & PIXEL_BGR))
			color = _byteswap_ulong(_rotl(color, 8));

		txt->Draw(time, color, RGB(0, 0, 0), align, &padText, frame);
	}
}