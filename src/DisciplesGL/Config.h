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

#pragma once

#include "ExtraTypes.h"

#define CONFIG_WRAPPER "Wrapper"
#define CONFIG_DISCIPLE "Disciple"
#define CONFIG_SETTINGS "Settings"
#define CONFIG_KEYS "FunktionKeys"

#define BORDERLESS_OFFSET 1

#define WIDE_WIDTH 990
#define WIDE_HEIGHT 800

#define MSG_TIMEOUT 10

#define MAX_SPEED_INDEX 21

extern LONG GAME_WIDTH;
extern LONG GAME_HEIGHT;

extern FLOAT GAME_WIDTH_FLOAT;
extern FLOAT GAME_HEIGHT_FLOAT;

extern ConfigItems config;
extern DisplayMode modesList[4];
extern const Resolution resolutionsList[28];

namespace Config
{
	BOOL __fastcall Load();
	INT __fastcall Get(const CHAR* app, const CHAR* key, INT default);
	DWORD __fastcall Get(const CHAR* app, const CHAR* key, CHAR* default, CHAR* returnString, DWORD nSize);
	BOOL __fastcall Set(const CHAR* app, const CHAR* key, INT value);
	BOOL __fastcall Set(const CHAR* app, const CHAR* key, CHAR* value);
	BOOL __fastcall IsZoomed();
	VOID __fastcall CalcZoomed(Size* dst, Size* src, DWORD scale);
	VOID __fastcall CalcZoomed();
	VOID __fastcall UpdateLocale();
}