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
	HDC hDc = GetDC(NULL);
	{
		this->hDc = CreateCompatibleDC(hDc);

		BITMAPV5HEADER hdr;
		MemoryZero(&hdr, sizeof(BITMAPV5HEADER));

		BITMAPV4HEADER bmi = *(BITMAPV4HEADER*)&hdr;
		bmi.bV4Size = sizeof(BITMAPINFOHEADER);
		bmi.bV4Width = *(LONG*)&config.mode->width;
		bmi.bV4Height = -*(LONG*)&config.mode->height;
		bmi.bV4Planes = 1;
		bmi.bV4XPelsPerMeter = 1;
		bmi.bV4YPelsPerMeter = 1;

		if (isTrue)
			bmi.bV4BitCount = 32;
		else
		{
			bmi.bV4BitCount = 16;
			bmi.bV4V4Compression = BI_BITFIELDS;
			bmi.bV4RedMask = 0xF800;
			bmi.bV4GreenMask = 0x07E0;
			bmi.bV4BlueMask = 0x001F;
		}

		this->hBmp = CreateDIBSection(hDc, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, &this->data, NULL, 0);
		SelectObject(this->hDc, this->hBmp);
	}
	ReleaseDC(NULL, hDc);

	MemoryZero(this->data, config.mode->width * config.mode->height * (isTrue ? sizeof(DWORD) : sizeof(WORD)));

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
		DeleteObject(this->hBmp);
		DeleteDC(this->hDc);
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