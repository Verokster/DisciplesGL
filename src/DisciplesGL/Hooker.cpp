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
#include "Hooker.h"

Hooker::Hooker(HMODULE hModule)
{
	this->hModule = hModule;
	this->headNT = (PIMAGE_NT_HEADERS)((DWORD)this->hModule + ((PIMAGE_DOS_HEADER)this->hModule)->e_lfanew);
	this->baseOffset = (INT)this->hModule - (INT)this->headNT->OptionalHeader.ImageBase;

	this->hFile = INVALID_HANDLE_VALUE;
	this->hMap = NULL;
	this->mapAddress = NULL;
}

Hooker::~Hooker()
{
	this->UnmapFile();
}

BOOL Hooker::MapFile()
{
	if (!this->mapAddress)
	{
		if (this->hFile == INVALID_HANDLE_VALUE)
		{
			CHAR filePath[MAX_PATH];
			GetModuleFileName(this->hModule, filePath, MAX_PATH);
			this->hFile = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (this->hFile == INVALID_HANDLE_VALUE)
				return FALSE;
		}

		if (!this->hMap)
		{
			this->hMap = CreateFileMapping(this->hFile, NULL, PAGE_READONLY, 0, 0, NULL);
			if (!this->hMap)
				return FALSE;
		}

		this->mapAddress = MapViewOfFile(this->hMap, FILE_MAP_READ, 0, 0, 0);
	}

	return (BOOL)this->mapAddress;
}

VOID Hooker::UnmapFile()
{
	if (this->mapAddress && UnmapViewOfFile(this->mapAddress))
		this->mapAddress = NULL;

	if (this->hMap && CloseHandle(this->hMap))
		this->hMap = NULL;

	if (this->hFile != INVALID_HANDLE_VALUE && CloseHandle(this->hFile))
		this->hFile = INVALID_HANDLE_VALUE;
}

#pragma optimize("s", on)
BOOL Hooker::PatchSet(DWORD addr, BYTE byte, DWORD size)
{
	DWORD address = addr + this->baseOffset;

	DWORD old_prot;
	if (VirtualProtect((VOID*)address, size, PAGE_EXECUTE_READWRITE, &old_prot))
	{
		MemorySet((VOID*)address, byte, size);
		VirtualProtect((VOID*)address, size, old_prot, &old_prot);

		return TRUE;
	}
	return FALSE;
}

BOOL Hooker::PatchNop(DWORD addr, DWORD size)
{
	return PatchSet(addr, 0x90, size);
}

BOOL Hooker::PatchRedirect(DWORD addr, DWORD dest, BYTE instruction, DWORD nop)
{
	DWORD address = addr + this->baseOffset;

	DWORD size = instruction == 0xEB ? 2 : 5;

	DWORD old_prot;
	if (VirtualProtect((VOID*)address, size + nop, PAGE_EXECUTE_READWRITE, &old_prot))
	{
		BYTE* jump = (BYTE*)address;
		*jump = instruction;
		++jump;
		*(DWORD*)jump = dest - address - size;

		if (nop)
			MemorySet((VOID*)(address + size), 0x90, nop);

		VirtualProtect((VOID*)address, size + nop, old_prot, &old_prot);

		return TRUE;
	}
	return FALSE;
}

BOOL Hooker::PatchJump(DWORD addr, DWORD dest)
{
	INT relative = dest - addr - this->baseOffset - 2;
	return PatchRedirect(addr, dest, relative >= -128 && relative <= 127 ? 0xEB : 0xE9, 0);
}

BOOL Hooker::PatchHook(DWORD addr, VOID* hook, DWORD nop)
{
	return PatchRedirect(addr, (DWORD)hook, 0xE9, nop);
}

BOOL Hooker::PatchCall(DWORD addr, VOID* hook, DWORD nop)
{
	return PatchRedirect(addr, (DWORD)hook, 0xE8, nop);
}

BOOL Hooker::PatchBlock(DWORD addr, VOID* block, DWORD size)
{
	DWORD address = addr + this->baseOffset;

	DWORD old_prot;
	if (VirtualProtect((VOID*)address, size, PAGE_EXECUTE_READWRITE, &old_prot))
	{
		switch (size)
		{
		case 4:
			*(DWORD*)address = *(DWORD*)block;
			break;
		case 2:
			*(WORD*)address = *(WORD*)block;
			break;
		case 1:
			*(BYTE*)address = *(BYTE*)block;
			break;
		default:
			MemoryCopy((VOID*)address, block, size);
			break;
		}

		VirtualProtect((VOID*)address, size, old_prot, &old_prot);

		return TRUE;
	}
	return FALSE;
}

BOOL Hooker::ReadBlock(DWORD addr, VOID* block, DWORD size)
{
	DWORD address = addr + this->baseOffset;

	DWORD old_prot;
	if (VirtualProtect((VOID*)address, size, PAGE_READONLY, &old_prot))
	{
		switch (size)
		{
		case 4:
			*(DWORD*)block = *(DWORD*)address;
			break;
		case 2:
			*(WORD*)block = *(WORD*)address;
			break;
		case 1:
			*(BYTE*)block = *(BYTE*)address;
			break;
		default:
			MemoryCopy(block, (VOID*)address, size);
			break;
		}

		VirtualProtect((VOID*)address, size, old_prot, &old_prot);

		return TRUE;
	}
	return FALSE;
}

BOOL Hooker::PatchWord(DWORD addr, WORD value)
{
	return PatchBlock(addr, &value, sizeof(value));
}

BOOL Hooker::PatchDWord(DWORD addr, DWORD value)
{
	return PatchBlock(addr, &value, sizeof(value));
}

BOOL Hooker::PatchByte(DWORD addr, BYTE value)
{
	return PatchBlock(addr, &value, sizeof(value));
}

BOOL Hooker::ReadWord(DWORD addr, WORD* value)
{
	return ReadBlock(addr, value, sizeof(*value));
}

BOOL Hooker::ReadDWord(DWORD addr, DWORD* value)
{
	return ReadBlock(addr, value, sizeof(*value));
}

BOOL Hooker::ReadByte(DWORD addr, BYTE* value)
{
	return ReadBlock(addr, value, sizeof(*value));
}

BOOL Hooker::ReadRedirect(DWORD addr, DWORD* value)
{
	if (ReadDWord(addr + 1, value))
	{
		*value += addr + this->baseOffset + 5;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL Hooker::RedirectCall(DWORD addr, VOID* hook, DWORD* old)
{
	if (ReadDWord(addr + 1, old))
	{
		*old += addr + 5 + this->baseOffset;
		return PatchCall(addr, hook);
	}

	return FALSE;
}

DWORD Hooker::PatchImport(const CHAR* function, VOID* addr, BOOL deep)
{
	DWORD res = NULL;

	PIMAGE_DATA_DIRECTORY dataDir = &this->headNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if (dataDir->Size)
	{
		PIMAGE_IMPORT_DESCRIPTOR imports = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)this->hModule + dataDir->VirtualAddress);
		for (DWORD idx = 0; imports->Name; ++idx, ++imports)
		{
			// CHAR* libraryName = (CHAR*)((DWORD)this->hModule + imports->Name);

			PIMAGE_THUNK_DATA addressThunk = (PIMAGE_THUNK_DATA)((DWORD)this->hModule + imports->FirstThunk);
			PIMAGE_THUNK_DATA nameThunk;
			BOOL nameInternal = imports->OriginalFirstThunk;
			if (imports->OriginalFirstThunk)
				nameThunk = (PIMAGE_THUNK_DATA)((DWORD)this->hModule + imports->OriginalFirstThunk);
			else if (this->MapFile())
			{
				PIMAGE_NT_HEADERS headNT = (PIMAGE_NT_HEADERS)((DWORD)this->mapAddress + ((PIMAGE_DOS_HEADER)this->mapAddress)->e_lfanew);
				PIMAGE_SECTION_HEADER sh = (PIMAGE_SECTION_HEADER)((DWORD)&headNT->OptionalHeader + headNT->FileHeader.SizeOfOptionalHeader);

				nameThunk = NULL;
				DWORD sCount = headNT->FileHeader.NumberOfSections;
				while (sCount)
				{
					--sCount;

					if (imports->FirstThunk >= sh->VirtualAddress && imports->FirstThunk < sh->VirtualAddress + sh->Misc.VirtualSize)
					{
						nameThunk = PIMAGE_THUNK_DATA((DWORD)this->mapAddress + sh->PointerToRawData + imports->FirstThunk - sh->VirtualAddress);
						break;
					}

					++sh;
				}

				if (!nameThunk)
					return res;
			}
			else
				return res;

			for (; nameThunk->u1.AddressOfData; ++nameThunk, ++addressThunk)
			{
				PIMAGE_IMPORT_BY_NAME name = PIMAGE_IMPORT_BY_NAME((DWORD)this->hModule + nameThunk->u1.AddressOfData);

				WORD hint;
				if (ReadWord((INT)name - this->baseOffset, &hint) && !StrCompare((CHAR*)name->Name, function))
				{
					INT address = (INT)&addressThunk->u1.AddressOfData;
					if (ReadDWord(address - this->baseOffset, &res) && PatchDWord(address - this->baseOffset, (DWORD)addr))
					{
						if (nameInternal)
							PatchSet((DWORD)name->Name - this->baseOffset, NULL, StrLength(function));

						if (deep)
						{
							IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(this->headNT);
							for (DWORD idx = 0; idx < this->headNT->FileHeader.NumberOfSections; ++idx, ++section)
							{
								if (section->VirtualAddress == this->headNT->OptionalHeader.BaseOfCode && section->Misc.VirtualSize)
								{
									BYTE block[6];
									block[0] = 0xFF;
									block[1] = 0x25;
									*(DWORD*)&block[2] = address;

									BYTE* entry = (BYTE*)(this->headNT->OptionalHeader.ImageBase + section->VirtualAddress + this->baseOffset);
									DWORD total = section->Misc.VirtualSize;
									do
									{
										BYTE* ptr1 = entry;
										BYTE* ptr2 = block;

										DWORD count = sizeof(block);
										do
										{
											if (*ptr1++ != *ptr2++)
												goto lbl_cont;
										} while (--count);

										PatchJump((DWORD)entry - this->baseOffset, (DWORD)addr);
										goto lbl_exit;

									lbl_cont:
										++entry;
									} while (--total);
								}
							}

						lbl_exit:;
						}
					}

					return res;
				}
			}
		}
	}

	return res;
}

DWORD Hooker::PatchEntryPoint(VOID* entryPoint)
{
	return PatchHook((DWORD)this->hModule + this->headNT->OptionalHeader.AddressOfEntryPoint, entryPoint);
}
#pragma optimize("", on)