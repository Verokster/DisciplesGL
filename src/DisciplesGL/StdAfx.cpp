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
#include "intrin.h"
#include "Mmsystem.h"

HMODULE hDllModule;
HANDLE hActCtx;
CHAR snapshotName[MAX_PATH];

CREATEACTCTXA CreateActCtxC;
RELEASEACTCTX ReleaseActCtxC;
ACTIVATEACTCTX ActivateActCtxC;
DEACTIVATEACTCTX DeactivateActCtxC;

SETPROCESSDPIAWARENESS SetProcessDpiAwarenessC;

extern "C"
{
	VOID __stdcall _c4ltoa45(DWORD, DWORD, DWORD);
	VOID __stdcall _c4dtoa45(DWORD, DWORD, DWORD, DWORD, DWORD);
	VOID __stdcall _c4atod(DWORD, DWORD);
	VOID __stdcall _c4atol(DWORD, DWORD);
	VOID __stdcall _f4ptr(DWORD);
	VOID __stdcall _f4name(DWORD);
	VOID __stdcall _f4type(DWORD);
	VOID __stdcall _f4len(DWORD);
	VOID __stdcall _f4decimals(DWORD);
	VOID __stdcall _d4delete(DWORD);
	VOID __stdcall _d4close(DWORD);
	VOID __stdcall _d4pack(DWORD);
	VOID __stdcall _d4fieldJ(DWORD, DWORD);
	VOID __stdcall _d4numFields(DWORD);
	VOID __stdcall _d4open(DWORD, DWORD);
	VOID __stdcall _d4openClone(DWORD);
	VOID __stdcall _d4appendBlank(DWORD);
	VOID __stdcall _d4bof(DWORD);
	VOID __stdcall _d4eof(DWORD);
	VOID __stdcall _d4deleted(DWORD);
	VOID __stdcall _d4goBof(DWORD);
	VOID __stdcall _d4skip(DWORD, DWORD);
	VOID __stdcall _d4lockAll(DWORD);
	VOID __stdcall _d4unlock(DWORD);
	VOID __stdcall _d4changed(DWORD, DWORD);
	VOID __stdcall _d4field(DWORD, DWORD);
	VOID __stdcall _u4freeDefault(DWORD);
	VOID __stdcall _code4optStart(DWORD);
	VOID __stdcall _code4initLow(DWORD, DWORD, DWORD);
	VOID __stdcall _code4initUndo(DWORD);
	VOID __stdcall _code4close(DWORD);
	VOID __stdcall _error4text(DWORD, DWORD);
	VOID __stdcall _error4default(DWORD, DWORD, DWORD);
	VOID __stdcall _expr4parseLow(DWORD, DWORD, DWORD);
	VOID __stdcall _expr4source(DWORD);
	VOID __stdcall _expr4true(DWORD);
}

#define LIBEXP(a) VOID __declspec(naked) __stdcall ex_##a() { _asm { jmp _##a } }

LIBEXP(c4ltoa45)
LIBEXP(c4dtoa45)
LIBEXP(c4atod)
LIBEXP(c4atol)
LIBEXP(f4ptr)
LIBEXP(f4name)
LIBEXP(f4type)
LIBEXP(f4len)
LIBEXP(f4decimals)
LIBEXP(d4delete)
LIBEXP(d4close)
LIBEXP(d4pack)
LIBEXP(d4fieldJ)
LIBEXP(d4numFields)
LIBEXP(d4open)
LIBEXP(d4openClone)
LIBEXP(d4appendBlank)
LIBEXP(d4bof)
LIBEXP(d4eof)
LIBEXP(d4deleted)
LIBEXP(d4goBof)
LIBEXP(d4skip)
LIBEXP(d4lockAll)
LIBEXP(d4unlock)
LIBEXP(d4changed)
LIBEXP(d4field)
LIBEXP(u4freeDefault)
LIBEXP(code4optStart)
LIBEXP(code4initLow)
LIBEXP(code4initUndo)
LIBEXP(code4close)
LIBEXP(error4text)
LIBEXP(error4default)
LIBEXP(expr4parseLow)
LIBEXP(expr4source)
LIBEXP(expr4true)

DOUBLE MathRound(DOUBLE number)
{
	DOUBLE floorVal = MathFloor(number);
	return floorVal + 0.5 > number ? floorVal : MathCeil(number);
}

struct Aligned {
	Aligned* last;
	VOID* block;
	DWORD data[1];
} * alignedList;

VOID* AlignedAlloc(size_t size)
{
	Aligned* entry = (Aligned*)MemoryAlloc(sizeof(Aligned) + size + 12);
	entry->last = alignedList;
	entry->block = (VOID*)(DWORD(entry->data + 4) & 0xFFFFFFF0);
	alignedList = entry;
	
	return entry->block;
}

VOID AlignedFree(VOID* block)
{
	Aligned** list = &alignedList;
	Aligned* entry = alignedList;
	while (entry)
	{
		if (entry->block == block)
		{
			*list = entry->last;
			MemoryFree(entry);
			break;
		}

		list = &entry->last;
		entry = entry->last;
	}
}

VOID LoadKernel32()
{
	HMODULE hLib = GetModuleHandle("KERNEL32.dll");
	if (hLib)
	{
		CreateActCtxC = (CREATEACTCTXA)GetProcAddress(hLib, "CreateActCtxA");
		ReleaseActCtxC = (RELEASEACTCTX)GetProcAddress(hLib, "ReleaseActCtx");
		ActivateActCtxC = (ACTIVATEACTCTX)GetProcAddress(hLib, "ActivateActCtx");
		DeactivateActCtxC = (DEACTIVATEACTCTX)GetProcAddress(hLib, "DeactivateActCtx");
	}
}

VOID LoadShcore()
{
	HMODULE hLib = LoadLibrary("SHCORE.dll");
	if (hLib)
		SetProcessDpiAwarenessC = (SETPROCESSDPIAWARENESS)GetProcAddress(hLib, "SetProcessDpiAwareness");
}