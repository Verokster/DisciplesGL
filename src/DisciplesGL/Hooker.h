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

#include "Allocation.h"

#pragma once
class Hooker : public Allocation {
private:
	HANDLE hFile;
	HANDLE hMap;

public:
	HMODULE hModule;
	PIMAGE_NT_HEADERS headNT;
	DWORD baseOffset;
	VOID* mapAddress;

	Hooker(HMODULE);
	~Hooker();

	BOOL MapFile();
	VOID UnmapFile();

	BOOL PatchSet(DWORD, BYTE, DWORD);
	BOOL PatchNop(DWORD, DWORD);
	BOOL PatchRedirect(DWORD, DWORD, BYTE, DWORD);
	BOOL PatchJump(DWORD, DWORD);
	BOOL PatchHook(DWORD, VOID*, DWORD = 0);
	BOOL PatchCall(DWORD, VOID*, DWORD = 0);
	BOOL PatchBlock(DWORD, VOID* block, DWORD);
	BOOL ReadBlock(DWORD, VOID* block, DWORD);
	BOOL PatchWord(DWORD, WORD);
	BOOL PatchDWord(DWORD, DWORD);
	BOOL PatchByte(DWORD, BYTE);
	BOOL ReadWord(DWORD, WORD*);
	BOOL ReadDWord(DWORD, DWORD*);
	BOOL ReadByte(DWORD, BYTE*);
	BOOL ReadRedirect(DWORD, DWORD*);
	BOOL RedirectCall(DWORD, VOID*, DWORD* old);
	DWORD PatchImport(const CHAR*, VOID*, BOOL = FALSE);
	DWORD PatchEntryPoint(VOID*);
};
