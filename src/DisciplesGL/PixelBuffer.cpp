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

DWORD __forceinline ForwardCompare(DWORD* ptr1, DWORD* ptr2, DWORD slice, DWORD count)
{
	__asm {
		MOV ECX, count

		MOV ESI, ptr1
		MOV EDI, ptr2

		MOV EAX, slice
		SAL EAX, 2

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

		MOV ESI, ptr1
		MOV EDI, ptr2

		MOV EAX, slice
		SAL EAX, 2

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

DWORD __forceinline ForwardCompare(DWORD* ptr1, DWORD* ptr2, DWORD slice, DWORD width, DWORD height, DWORD pitch, POINT* p)
{
	__asm {
		MOV EAX, width
		MOV EDX, height

		MOV EBX, pitch
		SUB EBX, EAX
		SAL EBX, 2

		MOV ESI, ptr1
		MOV EDI, ptr2

		MOV ECX, slice
		SAL ECX, 2

		ADD ESI, ECX
		ADD EDI, ECX

		lbl_cycle:
			MOV ECX, EAX
			REPE CMPSD
			JNE lbl_break

			ADD ESI, EBX
			ADD EDI, EBX
		DEC EDX
		JNZ lbl_cycle

		XOR EAX, EAX
		JMP lbl_ret

		lbl_break:
		MOV EBX, p
		
		INC ECX
		SUB EAX, ECX
		MOV [EBX], EAX

		MOV EAX, height
		SUB EAX, EDX
		MOV [EBX+4], EAX

		XOR EAX, EAX
		INC EAX

		lbl_ret:
	}
}

DWORD __forceinline BackwardCompare(DWORD* ptr1, DWORD* ptr2, DWORD slice, DWORD width, DWORD height, DWORD pitch, POINT* p)
{
	__asm {
		MOV EAX, width
		MOV EDX, height

		MOV EBX, pitch
		SUB EBX, EAX
		SAL EBX, 2

		MOV ESI, ptr1
		MOV EDI, ptr2

		MOV ECX, slice
		SAL ECX, 2

		ADD ESI, ECX
		ADD EDI, ECX

		STD

		lbl_cycle:
			MOV ECX, EAX
			REPE CMPSD
			JNZ lbl_break

			SUB ESI, EBX
			SUB EDI, EBX
		DEC EDX
		JNZ lbl_cycle

		XOR EAX, EAX
		JMP lbl_ret

		lbl_break:
		MOV EBX, p
		
		MOV [EBX], ECX

		DEC EDX
		MOV [EBX+4], EDX

		XOR EAX, EAX
		INC EAX

		lbl_ret:
		CLD
	}
}

DWORD __forceinline ForwardCompare(DWORD* ptr1, DWORD* ptr2, DWORD slice, DWORD width, DWORD height, DWORD pitch)
{
	__asm {
		MOV EAX, width
		MOV EDX, height

		MOV EBX, pitch
		SUB EBX, EAX
		SAL EBX, 2

		MOV ESI, ptr1
		MOV EDI, ptr2

		MOV ECX, slice
		SAL ECX, 2

		ADD ESI, ECX
		ADD EDI, ECX

		lbl_cycle:
			MOV ECX, EAX
			REPE CMPSD
			JZ lbl_inc
			
			SUB EAX, ECX
			DEC EAX
			JZ lbl_ret

			SAL ECX, 2
			ADD EBX, ECX
			ADD ESI, EBX
			ADD EDI, EBX
			ADD EBX, 4
			JMP lbl_cont

			lbl_inc:
			ADD ESI, EBX
			ADD EDI, EBX
			
			lbl_cont:
		DEC EDX
		JNZ lbl_cycle

		lbl_ret:
		MOV ECX, width
		SUB ECX, EAX
		MOV EAX, ECX
	}
}

DWORD __forceinline BackwardCompare(DWORD* ptr1, DWORD* ptr2, DWORD slice, DWORD width, DWORD height, DWORD pitch)
{
	__asm {
		MOV EAX, width
		MOV EDX, height

		MOV EBX, pitch
		SUB EBX, EAX
		SAL EBX, 2

		MOV ESI, ptr1
		MOV EDI, ptr2

		MOV ECX, slice
		SAL ECX, 2

		ADD ESI, ECX
		ADD EDI, ECX

		STD

		lbl_cycle:
			MOV ECX, EAX
			REPE CMPSD
			JZ lbl_inc
			
			SUB EAX, ECX
			DEC EAX
			JZ lbl_ret

			SAL ECX, 2
			ADD EBX, ECX
			SUB ESI, EBX
			SUB EDI, EBX
			ADD EBX, 4
			JMP lbl_cont

			lbl_inc:
			SUB ESI, EBX
			SUB EDI, EBX
			
			lbl_cont:
		DEC EDX
		JNZ lbl_cycle

		lbl_ret:
		MOV ECX, width
		SUB ECX, EAX
		MOV EAX, ECX

		CLD
	}
}

PixelBuffer::PixelBuffer(Size* size, BOOL isTrue)
{
	this->last = { 0, 0 };
	this->size = *size;
	this->isTrue = isTrue;

	DWORD length = size->width * size->height * (isTrue ? sizeof(DWORD) : sizeof(WORD));
	this->secondaryBuffer = (DWORD*)AlignedAlloc(length);

#ifdef BLOCK_DEBUG
	this->tempBuffer = AlignedAlloc(length);
	MemoryZero(this->tempBuffer, length);
#endif
}

PixelBuffer::~PixelBuffer()
{
	AlignedFree(this->secondaryBuffer);

#ifdef BLOCK_DEBUG
	AlignedFree(this->tempBuffer);
#endif
}

VOID PixelBuffer::Reset()
{
	this->last = { 0, 0 };
}

BOOL PixelBuffer::Check(StateBufferAligned* stateBuffer)
{
	BOOL res = FALSE;
	this->primaryBuffer = (DWORD*)stateBuffer->data;

	if (stateBuffer->size.width != this->last.width || stateBuffer->size.height != this->last.height)
	{
		this->last = stateBuffer->size;
		res = TRUE;
	}
	else if (MemoryCompare(this->secondaryBuffer, this->primaryBuffer, stateBuffer->size.width * stateBuffer->size.height * (this->isTrue ? sizeof(DWORD) : sizeof(WORD))))
		res = TRUE;

	if (res)
	{
		stateBuffer->data = this->secondaryBuffer;
		this->secondaryBuffer = this->primaryBuffer;
	}

	return res;
}

BOOL PixelBuffer::Update(StateBufferAligned* stateBuffer)
{
	BOOL res = FALSE;
	this->primaryBuffer = (DWORD*)stateBuffer->data;

	if (stateBuffer->size.width != this->last.width || stateBuffer->size.height != this->last.height)
	{
		this->last = stateBuffer->size;

		if (!this->isTrue)
			GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->last.width, this->last.height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (WORD*)this->primaryBuffer + ((this->size.height - this->last.height) >> 1) * this->size.width + ((this->size.width - this->last.width) >> 1));
		else
			GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->last.width, this->last.height, GL_RGBA, GL_UNSIGNED_BYTE, this->primaryBuffer + ((this->size.height - this->last.height) >> 1) * this->size.width + ((this->size.width - this->last.width) >> 1));

		res = TRUE;
	}
	else if (this->last.width == this->size.width && this->last.height == this->size.height)
	{
		DWORD width = this->last.width;
		if (!this->isTrue)
			width >>= 1;

		DWORD length = width * this->last.height;
		DWORD index = ForwardCompare((DWORD*)this->secondaryBuffer, (DWORD*)this->primaryBuffer, 0, length);
		if (index)
		{
#ifdef BLOCK_DEBUG
			if (!this->isTrue)
				GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->last.width, this->last.height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, this->tempBuffer);
			else
				GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->last.width, this->last.height, GL_RGBA, GL_UNSIGNED_BYTE, this->tempBuffer);
#endif

			POINT offset = { 0, 0 };

			DWORD top = (length - index) / width;
			for (DWORD y = top; y < this->last.height; y += BLOCK_SIZE)
			{
				DWORD bottom = y + BLOCK_SIZE;
				if (bottom > this->last.height)
					bottom = this->last.height;

				for (DWORD x = 0; x < this->last.width; x += BLOCK_SIZE)
				{
					DWORD right = x + BLOCK_SIZE;
					if (right > this->last.width)
						right = this->last.width;

					RECT rc = { *(LONG*)&x, *(LONG*)&y, *(LONG*)&right, *(LONG*)&bottom };
					this->UpdateBlock(&rc, &offset);
				}
			}
		}

		res = TRUE;
	}
	else
	{
#ifdef BLOCK_DEBUG
		if (!this->isTrue)
			GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->last.width, this->last.height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, this->tempBuffer);
		else
			GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->last.width, this->last.height, GL_RGBA, GL_UNSIGNED_BYTE, this->tempBuffer);
#endif

		RECT offset;
		offset.left = (this->size.width - this->last.width) >> 1;
		offset.top = (this->size.height - this->last.height) >> 1;
		offset.right = offset.left + this->last.width;
		offset.bottom = offset.top + this->last.height;

		for (LONG y = offset.top; y < offset.bottom; y += BLOCK_SIZE)
		{
			LONG bottom = y + BLOCK_SIZE;
			if (bottom > offset.bottom)
				bottom = offset.bottom;

			for (LONG x = offset.left; x < offset.right; x += BLOCK_SIZE)
			{
				LONG right = x + BLOCK_SIZE;
				if (right > offset.right)
					right = offset.right;

				RECT rc = { x, y, right, bottom };
				if (this->UpdateBlock(&rc, (POINT*)&offset))
					res = TRUE;
			}
		}
	}

	if (res)
	{
		stateBuffer->data = this->secondaryBuffer;
		this->secondaryBuffer = this->primaryBuffer;
	}

	return res;
}

BOOL PixelBuffer::UpdateBlock(RECT* rect, POINT* offset)
{
	DWORD width = this->size.width;
	if (!this->isTrue)
	{
		width >>= 1;
		rect->left >>= 1;
		rect->right >>= 1;
	}

	RECT rc;
	Size dim = { rect->right - rect->left, rect->bottom - rect->top };
	if (ForwardCompare(this->primaryBuffer, this->secondaryBuffer,
			rect->top * width + rect->left,
			dim.width, dim.height, width, (POINT*)&rc.left)
		&& BackwardCompare(this->primaryBuffer, this->secondaryBuffer,
			(rect->bottom - 1) * width + (rect->right - 1),
			dim.width, dim.height, width, (POINT*)&rc.right))
	{
		if (rc.left > rc.right)
		{
			LONG p = rc.left;
			rc.left = rc.right;
			rc.right = p;
		}

		rc.left += rect->left;
		rc.right += rect->left;
		rc.top += rect->top;
		rc.bottom += rect->top;

		if (rc.bottom != rc.top)
		{
			if (rc.left != rect->left)
				rc.left -= ForwardCompare(this->primaryBuffer, this->secondaryBuffer,
					(rc.top + 1) * width + rect->left,
					rc.left - rect->left, rc.bottom - rc.top, width);

			if (rc.right != (rect->right - 1))
				rc.right += BackwardCompare(this->primaryBuffer, this->secondaryBuffer,
								(rc.bottom - 1) * width + (rect->right - 1),
								rect->right - rc.right, rc.bottom - rc.top, width)
					- 1;
		}

		Rect rect = { rc.left, rc.top, rc.right - rc.left + 1, rc.bottom - rc.top + 1 };
		DWORD* ptr = this->primaryBuffer + rect.y * width + rect.x;

		rect.x -= offset->x;
		rect.y -= offset->y;

		if (!this->isTrue)
			GLTexSubImage2D(GL_TEXTURE_2D, 0, rect.x << 1, rect.y, rect.width << 1, rect.height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, ptr);
		else
			GLTexSubImage2D(GL_TEXTURE_2D, 0, rect.x, rect.y, rect.width, rect.height, GL_RGBA, GL_UNSIGNED_BYTE, ptr);

		return TRUE;
	}

	return FALSE;
}
