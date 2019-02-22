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
#define WIN32_LEAN_AND_MEAN

#include "windows.h"
#include "stdlib.h"
#include "stdio.h"
#include "mmreg.h"
#include "ddraw.h"
#include "ExtraTypes.h"

#define WC_DRAW "drawclass"
#define WS_WINDOWED (WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPSIBLINGS)
#define WS_FULLSCREEN (WS_POPUP | WS_SYSMENU | WS_VISIBLE | WS_CLIPSIBLINGS | WS_MAXIMIZE)

typedef DWORD(__stdcall *AIL_WAVEOUTOPEN)(LPVOID driver, DWORD a1, DWORD a2, LPPCMWAVEFORMAT waveFormat);
typedef LPVOID(__stdcall *AIL_OPEN_STREAM)(LPVOID driver, CHAR* filePath, DWORD unknown);
typedef DWORD(__stdcall *AIL_STREAM_POSITION)(LPVOID stream);
typedef VOID(__stdcall *AIL_SET_STREAM_POSITION)(LPVOID stream, DWORD position);

typedef HRESULT(__stdcall *DIRECTDRAWCREATE)(GUID* lpGUID, LPDIRECTDRAW* lplpDD, IUnknown* pUnkOuter);

extern DIRECTDRAWCREATE DDCreate;

typedef HANDLE(__stdcall *CREATEACTCTXA)(ACTCTX* pActCtx);
typedef VOID(__stdcall *RELEASEACTCTX)(HANDLE hActCtx);
typedef BOOL(__stdcall *ACTIVATEACTCTX)(HANDLE hActCtx, ULONG_PTR* lpCookie);
typedef BOOL(__stdcall *DEACTIVATEACTCTX)(DWORD dwFlags, ULONG_PTR ulCookie);

extern CREATEACTCTXA CreateActCtxC;
extern RELEASEACTCTX ReleaseActCtxC;
extern ACTIVATEACTCTX ActivateActCtxC;
extern DEACTIVATEACTCTX DeactivateActCtxC;

typedef VOID*(__cdecl *MALLOC)(size_t);
typedef VOID(__cdecl *FREE)(VOID*);
typedef VOID*(__cdecl *ALIGNED_MALLOC)(size_t, size_t);
typedef VOID(__cdecl *ALIGNED_FREE)(VOID*);
typedef VOID*(__cdecl *MEMSET)(VOID*, INT, size_t);
typedef VOID*(__cdecl *MEMCPY)(VOID*, const VOID*, size_t);
typedef DOUBLE(__cdecl *CEIL)(DOUBLE);
typedef DOUBLE(__cdecl *FLOOR)(DOUBLE);
typedef INT(__cdecl *SPRINTF)(CHAR*, const CHAR*, ...);
typedef INT(__cdecl *STRCMP)(const CHAR*, const CHAR*);
typedef CHAR*(__cdecl *STRCPY)(CHAR*, const CHAR*);
typedef CHAR*(__cdecl *STRCAT)(CHAR*, const CHAR*);
typedef CHAR*(__cdecl *STRRCHR)(const CHAR*, INT);
typedef CHAR*(__cdecl *STRSTR)(const CHAR*, const CHAR*);
typedef size_t(__cdecl *WCSTOMBS)(CHAR*, const WCHAR*, size_t);
typedef VOID(__cdecl *EXIT)(INT);

extern MALLOC MemoryAlloc;
extern FREE MemoryFree;
extern ALIGNED_MALLOC AlignedAlloc;
extern ALIGNED_FREE AlignedFree;
extern MEMSET MemorySet;
extern MEMCPY MemoryCopy;
extern CEIL MathCeil;
extern FLOOR MathFloor;
extern SPRINTF StrPrint;
extern STRCMP StrCompare;
extern STRCPY StrCopy;
extern STRCAT StrCat;
extern STRRCHR StrLastChar;
extern STRSTR StrStr;
extern WCSTOMBS StrToAnsi;
extern EXIT Exit;

DOUBLE __fastcall MathRound(DOUBLE);

#define MemoryZero(destination,length) MemorySet(destination,0,length)

extern HMODULE hDllModule;
extern HANDLE hActCtx;

VOID LoadKernel32();
VOID LoadMsvCRT();
VOID LoadDDraw();