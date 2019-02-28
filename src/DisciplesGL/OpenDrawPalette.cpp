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
#include "OpenDrawPalette.h"
#include "OpenDraw.h"

OpenDrawPalette::OpenDrawPalette(IDrawUnknown** list, OpenDraw* lpDD)
{
	this->refCount = 1;
	this->list = list;
	this->last = *list;
	*list = this;

	this->ddraw = lpDD;

	this->entries = (DWORD*)AlignedAlloc(256 * sizeof(DWORD));
}

OpenDrawPalette::~OpenDrawPalette()
{
	AlignedFree(this->entries);

	IDrawDestruct(this);
}

HRESULT __stdcall OpenDrawPalette::GetEntries(DWORD dwFlags, DWORD dwBase, DWORD dwNumEntries, LPPALETTEENTRY lpEntries)
{
	MemoryCopy(lpEntries, this->entries + dwBase, dwNumEntries * sizeof(PALETTEENTRY));
	return DD_OK;
}

HRESULT __stdcall OpenDrawPalette::SetEntries(DWORD dwFlags, DWORD dwBase, DWORD dwNumEntries, LPPALETTEENTRY lpEntries)
{
	MemoryCopy(this->entries + dwBase, lpEntries, dwNumEntries * sizeof(PALETTEENTRY));
	return DD_OK;
}