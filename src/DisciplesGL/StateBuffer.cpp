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
#include "StateBuffer.h"
#include "Config.h"

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

StateBufferAligned::StateBufferAligned()
{
	this->size = { 0, 0 };

	BOOL isTrue = config.mode->bpp != 16 || config.bpp32Hooked;
	DWORD length = config.mode->width * config.mode->height * (isTrue ? sizeof(DWORD) : sizeof(WORD));

	if (config.renderer != RendererGDI || !isTrue)
		this->data = AlignedAlloc(length);
	else
	{
		HDC hDc = GetDC(NULL);
		{
			this->hDc = CreateCompatibleDC(hDc);

			BITMAPINFO bmi;
			MemoryZero(&bmi, sizeof(BITMAPINFO));
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = *(LONG*)&config.mode->width;
			bmi.bmiHeader.biHeight = -*(LONG*)&config.mode->height;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biXPelsPerMeter = 1;
			bmi.bmiHeader.biYPelsPerMeter = 1;
			
			this->hBmp = CreateDIBSection(hDc, &bmi, DIB_RGB_COLORS, &this->data, NULL, 0);
			SelectObject(this->hDc, this->hBmp);
		}
		ReleaseDC(NULL, hDc);
	}

	MemoryZero(this->data, length);

	this->isReady = FALSE;
	this->isZoomed = FALSE;
	this->borders = BordersNone;
	this->isBack = FALSE;
	this->isAllocated = TRUE;
}

StateBufferAligned::~StateBufferAligned()
{
	if (this->isAllocated)
	{
		this->isAllocated = FALSE;

		if (config.renderer != RendererGDI)
			AlignedFree(this->data);
		else
		{
			DeleteObject(this->hBmp);
			DeleteDC(this->hDc);
		}
	}
}

StateBufferBorder::StateBufferBorder(DWORD width, DWORD height, DWORD size)
	: StateBuffer(size)
{
	this->width = LOWORD(width);
	this->height = LOWORD(height);
	this->type = config.borders.type;
}

StateBufferBorder::StateBufferBorder(DWORD width, DWORD height, VOID* data)
	: StateBuffer(data)
{
	this->width = LOWORD(width);
	this->height = LOWORD(height);
	this->type = config.borders.type;
}