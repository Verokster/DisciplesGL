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
#include "intrin.h"
#include "Mmsystem.h"

HMODULE hDllModule;
HANDLE hActCtx;
UINT WM_SNAPSHOT;
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

VOID _declspec(naked) __stdcall ex_c4ltoa45()
{
	_asm { JMP _c4ltoa45 }
}
VOID _declspec(naked) __stdcall ex_c4dtoa45()
{
	_asm { JMP _c4dtoa45 }
}
VOID _declspec(naked) __stdcall ex_c4atod()
{
	_asm { JMP _c4atod }
}
VOID _declspec(naked) __stdcall ex_c4atol()
{
	_asm { JMP _c4atol }
}
VOID _declspec(naked) __stdcall ex_f4ptr()
{
	_asm { JMP _f4ptr }
}
VOID _declspec(naked) __stdcall ex_f4name()
{
	_asm { JMP _f4name }
}
VOID _declspec(naked) __stdcall ex_f4type()
{
	_asm { JMP _f4type }
}
VOID _declspec(naked) __stdcall ex_f4len()
{
	_asm { JMP _f4len }
}
VOID _declspec(naked) __stdcall ex_f4decimals()
{
	_asm { JMP _f4decimals }
}
VOID _declspec(naked) __stdcall ex_d4delete()
{
	_asm { JMP _d4delete }
}
VOID _declspec(naked) __stdcall ex_d4close()
{
	_asm { JMP _d4close }
}
VOID _declspec(naked) __stdcall ex_d4pack()
{
	_asm { JMP _d4pack }
}
VOID _declspec(naked) __stdcall ex_d4fieldJ()
{
	_asm { JMP _d4fieldJ }
}
VOID _declspec(naked) __stdcall ex_d4numFields()
{
	_asm { JMP _d4numFields }
}
VOID _declspec(naked) __stdcall ex_d4open()
{
	_asm { JMP _d4open }
}
VOID _declspec(naked) __stdcall ex_d4openClone()
{
	_asm { JMP _d4openClone }
}
VOID _declspec(naked) __stdcall ex_d4appendBlank()
{
	_asm { JMP _d4appendBlank }
}
VOID _declspec(naked) __stdcall ex_d4bof()
{
	_asm { JMP _d4bof }
}
VOID _declspec(naked) __stdcall ex_d4eof()
{
	_asm { JMP _d4eof }
}
VOID _declspec(naked) __stdcall ex_d4deleted()
{
	_asm { JMP _d4deleted }
}
VOID _declspec(naked) __stdcall ex_d4goBof()
{
	_asm { JMP _d4goBof }
}
VOID _declspec(naked) __stdcall ex_d4skip()
{
	_asm { JMP _d4skip }
}
VOID _declspec(naked) __stdcall ex_d4lockAll()
{
	_asm { JMP _d4lockAll }
}
VOID _declspec(naked) __stdcall ex_d4unlock()
{
	_asm { JMP _d4unlock }
}
VOID _declspec(naked) __stdcall ex_d4changed()
{
	_asm { JMP _d4changed }
}
VOID _declspec(naked) __stdcall ex_d4field()
{
	_asm { JMP _d4field }
}
VOID _declspec(naked) __stdcall ex_u4freeDefault()
{
	_asm { JMP _u4freeDefault }
}
VOID _declspec(naked) __stdcall ex_code4optStart()
{
	_asm { JMP _code4optStart }
}
VOID _declspec(naked) __stdcall ex_code4initLow()
{
	_asm { JMP _code4initLow }
}
VOID _declspec(naked) __stdcall ex_code4initUndo()
{
	_asm { JMP _code4initUndo }
}
VOID _declspec(naked) __stdcall ex_code4close()
{
	_asm { JMP _code4close }
}
VOID _declspec(naked) __stdcall ex_error4text()
{
	_asm { JMP _error4text }
}
VOID _declspec(naked) __stdcall ex_error4default()
{
	_asm { JMP _error4default }
}
VOID _declspec(naked) __stdcall ex_expr4parseLow()
{
	_asm { JMP _expr4parseLow }
}
VOID _declspec(naked) __stdcall ex_expr4source()
{
	_asm { JMP _expr4source }
}
VOID _declspec(naked) __stdcall ex_expr4true()
{
	_asm { JMP _expr4true }
}

DOUBLE __fastcall MathRound(DOUBLE number)
{
	DOUBLE floorVal = MathFloor(number);
	return floorVal + 0.5f > number ? floorVal : MathCeil(number);
}

struct Aligned {
	Aligned* last;
	VOID* data;
	VOID* block;
} * alignedList;

VOID* __fastcall AlignedAlloc(size_t size)
{
	Aligned* entry = (Aligned*)MemoryAlloc(sizeof(Aligned));
	entry->last = alignedList;
	alignedList = entry;

	entry->data = MemoryAlloc(size + 16);
	entry->block = (VOID*)(((DWORD)entry->data + 16) & 0xFFFFFFF0);

	return entry->block;
}

VOID __fastcall AlignedFree(VOID* block)
{
	Aligned* entry = alignedList;
	if (entry)
	{
		if (entry->block == block)
		{
			Aligned* last = entry->last;
			MemoryFree(entry->data);
			MemoryFree(entry);
			alignedList = last;
			return;
		}
		else
			while (entry->last)
			{
				if (entry->last->block == block)
				{
					Aligned* last = entry->last->last;
					MemoryFree(entry->last->data);
					MemoryFree(entry->last);
					entry->last = last;
					return;
				}

				entry = entry->last;
			}
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