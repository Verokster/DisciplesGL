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

#pragma once
#define WIN32_LEAN_AND_MEAN
//#define WINVER 0x0400

#include "windows.h"
#include "mmreg.h"
#include "math.h"
#include "shellscalingapi.h"
#include "locale.h"
#include "ddraw.h"
#include "ExtraTypes.h"

#define WC_DRAW "7903f211-51ca-4a51-9ec5-e1301db2d24d"
#define WM_SNAPSHOT "WM_SNAPSHOT"
#define WM_CHECK_MENU "WM_CHECK_MENU"
#define WS_WINDOWED (WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPSIBLINGS)
#define WS_FULLSCREEN (WS_POPUP | WS_SYSMENU | WS_VISIBLE | WS_CLIPSIBLINGS | WS_MAXIMIZE)

#define ALPHA_COMPONENT 0xFF000000
#define COLORKEY_AND 0xFFF8FCF8
#define COLORKEY_CHECK 0xFFF800F8

typedef HANDLE(__stdcall* CREATEACTCTXA)(ACTCTX* pActCtx);
typedef VOID(__stdcall* RELEASEACTCTX)(HANDLE hActCtx);
typedef BOOL(__stdcall* ACTIVATEACTCTX)(HANDLE hActCtx, ULONG_PTR* lpCookie);
typedef BOOL(__stdcall* DEACTIVATEACTCTX)(DWORD dwFlags, ULONG_PTR ulCookie);

extern CREATEACTCTXA CreateActCtxC;
extern RELEASEACTCTX ReleaseActCtxC;
extern ACTIVATEACTCTX ActivateActCtxC;
extern DEACTIVATEACTCTX DeactivateActCtxC;

typedef HRESULT(__stdcall* SETPROCESSDPIAWARENESS)(PROCESS_DPI_AWARENESS);

extern SETPROCESSDPIAWARENESS SetProcessDpiAwarenessC;

extern "C"
{
	_CRTIMP int __CRTDECL sprintf(char*, const char*, ...);
	_CRTIMP int __CRTDECL vsprintf(char*, const char*, va_list);
	_CRTIMP void* __CRTDECL shi_new(size_t size);
	_CRTIMP void __CRTDECL shi_delete(void* block);
	_CRTIMP void* __CRTDECL shi_malloc(size_t size);
	_CRTIMP void __CRTDECL shi_free(void* block);
}

#define MemoryNew(size) shi_new(size)
#define MemoryDelete(block) shi_delete(block)
#define MemoryAlloc(size) shi_malloc(size)
#define MemoryFree(block) shi_free(block)
#define MemorySet(dst, val, size) memset(dst, val, size)
#define MemoryZero(dst, size) memset(dst, 0, size)
#define MemoryCopy(dst, src, size) memcpy(dst, src, size)
#define MemoryCompare(buf1, buf2, size) memcmp(buf1, buf2, size)
#define MemoryChar(block, ch, length) memchr(block, ch, length)
#define MathCeil(x) ceil(x)
#define MathFloor(x) floor(x)
#define StrPrint(buf, fmt, ...) sprintf(buf, fmt, __VA_ARGS__)
#define StrPrintVar(buf, fmt, va) vsprintf(buf, fmt, va)
#define StrLength(str) strlen(str)
#define StrCompare(str1, str2) strcmp(str1, str2)
#define StrCompareInsensitive(str1, str2) _stricmp(str1, str2)
#define StrCopy(dst, src) strcpy(dst, src)
#define StrCat(dst, src) strcat(dst, src)
#define StrDuplicate(str) _strdup(str)
#define StrLastChar(str, ch) strrchr(str, ch)
#define StrChar(str, ch) strchr(str, ch)
#define StrStr(str, substr) strstr(str, substr)
#define StrToInt(src) atoi(src)
#define FileGetStr(str, num, stream) fgets(str, num, stream)
#define Random() rand()
#define SeedRandom(seed) srand(seed)
#define SetLocale(cat, loc) setlocale(cat, loc)
#define IsAlpha(ch) isalpha(ch)
#define IsAlNum(ch) isalnum(ch)
#define IsDigit(ch) isdigit(ch)
#define IsSpace(ch) isspace(ch)
#define IsPunct(ch) ispunct(ch)
#define IsCntrl(ch) iscntrl(ch)
#define IsUpper(ch) isupper(ch)
#define ToUpper(ch) toupper(ch)
#define ToLower(ch) tolower(ch)
#define Exit(code) exit(code)

DOUBLE __fastcall MathRound(DOUBLE);
VOID* __fastcall AlignedAlloc(size_t size);
VOID __fastcall AlignedFree(VOID* block);

extern HMODULE hDllModule;
extern HANDLE hActCtx;
extern CHAR snapshotName[];

VOID LoadKernel32();
VOID LoadShcore();