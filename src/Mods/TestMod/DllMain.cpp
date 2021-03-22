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
#include "Hooks.h"
#include "TextRenderer.h"

HFONT hFont;

BOOL __stdcall DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		if (TRUE) // check if mod is compatible with an executable
		{
			Config::Load(hModule);
			Hooks::Load();

			hFont = CreateFont(28, 0, 0, 0, 700, 0, FALSE, FALSE, ANSI_CHARSET, 0, 0, DEFAULT_QUALITY, 0, "Digital-7 Mono");
			if (!hFont)
				hFont = CreateFont(28, 0, 0, 0, 0, 0, FALSE, FALSE, ANSI_CHARSET, 0, 0, DEFAULT_QUALITY, 0, "Arial");
			txt = new TextRenderer(hFont);

			return TRUE;
		}

		return FALSE;

	case DLL_PROCESS_DETACH:
		if (txt)
			delete txt;

		if (hFont)
			DeleteObject(hFont);

		return TRUE;

	default:
		return TRUE;
	}
}