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
#include "IDrawUnknown.h"

HRESULT __stdcall IDrawUnknown::QueryInterface(REFIID, LPVOID*) { return DD_OK; }

ULONG __stdcall IDrawUnknown::AddRef()
{
	return ++this->refCount;
}

ULONG __stdcall IDrawUnknown::Release()
{
	ULONG count = --this->refCount;
	if (!count)
		delete this;

	return count;
}

VOID IDrawDestruct(IDrawUnknown* item)
{
	IDrawUnknown* entry = *item->list;

	if (entry == item)
		*item->list = NULL;
	else while (entry)
	{
		if (entry->last == item)
		{
			entry->last = item->last;
			break;
		}

		entry = entry->last;
	}
}
