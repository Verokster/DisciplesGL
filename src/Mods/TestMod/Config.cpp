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
#include "Config.h"
#include "Resource.h"

ConfigItems config;

namespace Config
{
	VOID Load(HMODULE hModule)
	{
		config.hModule = hModule;

		HMODULE hMain = GetModuleHandle(NULL);
		GetModuleFileName(hMain, config.file, MAX_PATH);
		strcpy(strrchr(config.file, '\\'), "\\disciple.ini");

		config.font = (HFONT)CreateFont(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
			OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE, TEXT("MS Shell Dlg"));

		config.enabled = (BOOL)Config::Get(CONFIG_MOD, "Enabled", TRUE);
		config.tick = config.enabled ? GetTickCount() : 0;

		config.displayCorner = (DisplayCorner)Config::Get(CONFIG_MOD, "DisplayCorner", DisplayBottomLeft);
		if (config.displayCorner < DisplayTopLeft || config.displayCorner > DisplayBottomRight)
			config.displayCorner = DisplayTopLeft;
	}

	INT Get(const CHAR* app, const CHAR* key, INT default)
	{
		return GetPrivateProfileInt(app, key, (INT)default, config.file);
	}

	DWORD Get(const CHAR* app, const CHAR* key, const CHAR* default, CHAR* returnString, DWORD nSize)
	{
		return GetPrivateProfileString(app, key, default, returnString, nSize, config.file);
	}

	BOOL Set(const CHAR* app, const CHAR* key, INT value)
	{
		CHAR res[20];
		_itoa(value, res, 10);
		return WritePrivateProfileString(app, key, res, config.file);
	}

	BOOL Set(const CHAR* app, const CHAR* key, CHAR* value)
	{
		return WritePrivateProfileString(app, key, value, config.file);
	}
}