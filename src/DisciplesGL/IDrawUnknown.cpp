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
#include "IDrawUnknown.h"

VOID* IDrawUnknown::operator new(size_t size) { return MemoryNew(size); }

VOID IDrawUnknown::operator delete(VOID* p)
{
	IDrawUnknown* item = (IDrawUnknown*)p;
	if (item->list)
	{
		IDrawUnknown* entry = *item->list;
		if (entry)
		{
			if (entry == item)
			{
				*item->list = entry->last;
				MemoryDelete(p);
				return;
			}
			else
				while (entry->last)
				{
					if (entry->last == item)
					{
						entry->last = item->last;
						MemoryDelete(p);
						return;
					}

					entry = entry->last;
				}
		}
	}
}

IDrawUnknown::IDrawUnknown(IDrawUnknown** list)
{
	this->refCount = 1;
	this->list = list;
	if (list)
	{
		this->last = *list;
		*list = this;
	}
	else
		this->last = NULL;
}

HRESULT __stdcall IDrawUnknown::QueryInterface(REFIID, LPVOID*) { return DD_OK; }

ULONG __stdcall IDrawUnknown::AddRef()
{
	return ++this->refCount;
}

ULONG __stdcall IDrawUnknown::Release()
{
	if (--this->refCount)
		return this->refCount;

	delete this;
	return 0;
}
