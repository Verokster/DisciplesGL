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
#include "MappedFile.h"

MappedFile::MappedFile(HMODULE hModule)
{
	this->hModule = hModule;
	this->hFile = INVALID_HANDLE_VALUE;
	this->hMap = NULL;
	this->address = NULL;
}

MappedFile::~MappedFile()
{
	if (this->address)
		UnmapViewOfFile(this->address);

	if (this->hMap)
		CloseHandle(this->hMap);

	if (this->hFile != INVALID_HANDLE_VALUE)
		CloseHandle(this->hFile);
}

VOID MappedFile::Load()
{
	CHAR filePath[MAX_PATH];
	GetModuleFileName(hModule, filePath, MAX_PATH);
	this->hFile = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (this->hFile == INVALID_HANDLE_VALUE)
		return;

	this->hMap = CreateFileMapping(this->hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (!this->hMap)
		return;

	this->address = MapViewOfFile(this->hMap, FILE_MAP_READ, 0, 0, 0);
}


