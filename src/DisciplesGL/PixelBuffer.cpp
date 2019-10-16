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
#include "PixelBuffer.h"

PixelBuffer::PixelBuffer(DWORD width, DWORD height, BOOL isTrue, VOID* tempBuffer)
{
	this->lastSize = { 0, 0 };
	this->size.width = width;
	this->size.height = height;
	this->isTrue = isTrue;

	this->tempBuffer = tempBuffer;

	DWORD length = width * height * (isTrue ? sizeof(DWORD) : sizeof(WORD));
	this->secondaryBuffer = AlignedAlloc(length);
}

PixelBuffer::~PixelBuffer()
{
	AlignedFree(this->secondaryBuffer);
}

VOID PixelBuffer::Reset()
{
	this->lastSize = { 0, 0 };
}

DWORD __forceinline ForwardCompare(DWORD* ptr1, DWORD* ptr2, DWORD slice, DWORD count)
{
	__asm {
		MOV ECX, count
		TEST ECX, ECX
		JZ lbl_ret

		MOV ESI, ptr1
		MOV EDI, ptr2

		MOV EAX, slice
		LEA EAX, [EAX * 0x4]

		ADD ESI, EAX
		ADD EDI, EAX

		REPE CMPSD
		JZ lbl_ret
		INC ECX

		lbl_ret:
		MOV EAX, ECX
	}
}

DWORD __forceinline BackwardCompare(DWORD* ptr1, DWORD* ptr2, DWORD slice, DWORD count)
{
	__asm {
		MOV ECX, count
		TEST ECX, ECX
		JZ lbl_ret

		MOV ESI, ptr1
		MOV EDI, ptr2

		MOV EAX, slice
		LEA EAX, [EAX * 0x4]

		ADD ESI, EAX
		ADD EDI, EAX

		STD
		REPE CMPSD
		CLD
		JZ lbl_ret
		INC ECX

		lbl_ret:
		MOV EAX, ECX
	}
}

BOOL PixelBuffer::Check(Size* newSize, StateBuffer* stateBuffer)
{
	BOOL res = FALSE;
	this->primaryBuffer = stateBuffer->data;

	if (newSize->width != this->lastSize.width || newSize->height != this->lastSize.height)
	{
		this->lastSize = *newSize;
		res = TRUE;
	}
	else if (MemoryCompare(this->secondaryBuffer, this->primaryBuffer, newSize->width * newSize->height * (this->isTrue ? sizeof(DWORD) : sizeof(WORD))))
		res = TRUE;

	if (res)
	{
		stateBuffer->data = this->secondaryBuffer;
		this->secondaryBuffer = this->primaryBuffer;
	}

	return res;
}

BOOL PixelBuffer::Update(Size* newSize, StateBuffer* stateBuffer)
{
	BOOL res = FALSE;
	this->primaryBuffer = stateBuffer->data;

	if (newSize->width != this->lastSize.width || newSize->height != this->lastSize.height)
	{
		this->lastSize = *newSize;

		if (!this->isTrue)
			GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, newSize->width, newSize->height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, this->primaryBuffer);
		else
			GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, newSize->width, newSize->height, GL_RGBA, GL_UNSIGNED_BYTE, this->primaryBuffer);

		res = TRUE;
	}
	else
	{
		DWORD width = newSize->width;
		if (!this->isTrue)
			width >>= 1;

		DWORD length = width * newSize->height;
		DWORD index = ForwardCompare((DWORD*)this->secondaryBuffer, (DWORD*)this->primaryBuffer, 0, length);
		if (index)
		{
			DWORD top = (length - index) / width;
			for (DWORD y = top; y < newSize->height; y += BLOCK_SIZE)
			{
				DWORD bottom = y + BLOCK_SIZE;
				if (bottom > newSize->height)
					bottom = newSize->height;

				for (DWORD x = 0; x < newSize->width; x += BLOCK_SIZE)
				{
					DWORD right = x + BLOCK_SIZE;
					if (right > newSize->width)
						right = newSize->width;

					RECT rc = { *(LONG*)&x, *(LONG*)&y, *(LONG*)&right, *(LONG*)&bottom };
					this->UpdateBlock(newSize, &rc);
				}
			}

			res = TRUE;
		}
	}

	if (res)
	{
		stateBuffer->data = this->secondaryBuffer;
		this->secondaryBuffer = this->primaryBuffer;
	}

	return res;
}

BOOL PixelBuffer::UpdateBlock(Size* newSize, RECT* newRect)
{
	DWORD found = NULL;
	RECT rc;

	DWORD width = newSize->width;
	if (!this->isTrue)
	{
		width >>= 1;
		newRect->left >>= 1;
		newRect->right >>= 1;
	}

	{
		DWORD* srcData = (DWORD*)this->secondaryBuffer + newRect->top * width;
		DWORD* dstData = (DWORD*)this->primaryBuffer + newRect->top * width;

		for (LONG y = newRect->top; y < newRect->bottom; ++y)
		{
			DWORD index = ForwardCompare(srcData, dstData, newRect->left, newRect->right - newRect->left);
			if (index)
			{
				found = 1;
				rc.left = rc.right = newRect->right - index;
			}

			if (found)
			{
				LONG start = newRect->right - 1;
				rc.right += BackwardCompare(srcData, dstData, start, start - rc.right);
				rc.top = rc.bottom = y;
				break;
			}

			srcData += width;
			dstData += width;
		}
	}

	if (found)
	{
		{
			{
				DWORD* srcData = (DWORD*)this->secondaryBuffer + (newRect->bottom - 1) * width;
				DWORD* dstData = (DWORD*)this->primaryBuffer + (newRect->bottom - 1) * width;

				for (LONG y = newRect->bottom - 1; y > rc.top; --y)
				{
					DWORD index = ForwardCompare(srcData, dstData, newRect->left, newRect->right - newRect->left);
					if (index)
					{
						found |= 2;

						LONG x = newRect->right - index;
						if (rc.left > x)
							rc.left = x;
						else if (rc.right < x)
							rc.right = x;
					}

					if (found & 2)
					{
						LONG start = newRect->right - 1;
						rc.right += BackwardCompare(srcData, dstData, start, start - rc.right);
						rc.bottom = y;
						break;
					}

					srcData -= width;
					dstData -= width;
				}
			}

			{
				DWORD* srcData = (DWORD*)this->secondaryBuffer + (rc.top + 1) * width;
				DWORD* dstData = (DWORD*)this->primaryBuffer + (rc.top + 1) * width;

				for (LONG y = rc.top + 1; y < rc.bottom; ++y)
				{
					rc.left -= ForwardCompare(srcData, dstData, newRect->left, rc.left - newRect->left);

					LONG start = newRect->right - 1;
					rc.right += BackwardCompare(srcData, dstData, start, start - rc.right);

					srcData += width;
					dstData += width;
				}
			}
		}

		++rc.right;
		++rc.bottom;

		Rect rect = { rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top };

		DWORD check = rect.width;
		if (!this->isTrue)
			check <<= 1;

		{
			DWORD* srcData = (DWORD*)this->primaryBuffer + rect.y * width + rect.x;
			DWORD* dstData = (DWORD*)this->tempBuffer;

			DWORD height = rect.height;
			do
			{
				MemoryCopy(dstData, srcData, rect.width * sizeof(DWORD));

				srcData += width;
				dstData += rect.width;
			} while (--height);

			if (!this->isTrue)
				GLTexSubImage2D(GL_TEXTURE_2D, 0, rect.x << 1, rect.y, check, rect.height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, this->tempBuffer);
			else
				GLTexSubImage2D(GL_TEXTURE_2D, 0, rect.x, rect.y, check, rect.height, GL_RGBA, GL_UNSIGNED_BYTE, this->tempBuffer);
		}
	}

	return found;
}
