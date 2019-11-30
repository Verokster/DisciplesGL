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

#include "Allocation.h"

class StateBuffer : public Allocation {
public:
	BYTE isAllocated;
	VOID* data;

	StateBuffer();
	StateBuffer(DWORD size);
	StateBuffer(VOID* data);
	virtual ~StateBuffer();
};

class StateBufferAligned : public StateBuffer {
private:
	HBITMAP hBmp;

public:
	BOOL isReady;
	BOOL isZoomed;
	BOOL isBorder;
	HDC hDc;

	Size size;

	StateBufferAligned();
	~StateBufferAligned();
};

class StateBufferBorder : public StateBuffer {
public:
	DWORD width;
	DWORD height;

	StateBufferBorder(DWORD width, DWORD height, DWORD size);
	StateBufferBorder(DWORD width, DWORD height, VOID* data);
};