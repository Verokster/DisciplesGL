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
#include "StateBuffer.h"

StateBuffer::StateBuffer()
{
	this->data = NULL;
	this->isAllocated = FALSE;
}

StateBuffer::StateBuffer(DWORD size)
{
	this->data = MemoryAlloc(size);
	this->isAllocated = TRUE;

	MemoryZero(this->data, size);
}

StateBuffer::StateBuffer(VOID* data)
{
	this->data = data;
	this->isAllocated = FALSE;
}

StateBuffer::~StateBuffer()
{
	if (this->isAllocated)
		MemoryFree(this->data);
}

StateBufferAligned::StateBufferAligned(DWORD size)
{
	this->size = { 0, 0 };

	this->data = AlignedAlloc(size);
	MemoryZero(this->data, size);

	this->isReady = FALSE;
	this->isZoomed = FALSE;
	this->isBorder = FALSE;
	this->isAllocated = TRUE;
}

StateBufferAligned::~StateBufferAligned()
{
	if (this->isAllocated)
	{
		this->isAllocated = FALSE;
		AlignedFree(this->data);
	}
}

StateBufferBorder::StateBufferBorder(DWORD width, DWORD height, DWORD size)
	: StateBuffer(size)
{
	this->width = LOWORD(width);
	this->height = LOWORD(height);
}

StateBufferBorder::StateBufferBorder(DWORD width, DWORD height, VOID* data)
	: StateBuffer(data)
{
	this->width = LOWORD(width);
	this->height = LOWORD(height);
}