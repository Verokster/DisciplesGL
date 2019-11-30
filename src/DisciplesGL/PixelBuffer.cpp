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
#include "Config.h"

DWORD __declspec(naked) __fastcall ForwardCompare(DWORD count, DWORD slice, VOID* ptr1, VOID* ptr2)
{
	__asm {
		push ebp
		mov ebp, esp
		push esi
		push edi

		mov esi, ptr1
		mov edi, ptr2

		sal edx, 2
		add esi, edx
		add edi, edx

		repe cmpsd
		jz lbl_ret
		inc ecx

		lbl_ret:
		mov eax, ecx

		pop edi
		pop esi
		mov esp, ebp
		pop ebp

		retn 8
	}
}

DWORD __declspec(naked) __fastcall BackwardCompare(DWORD count, DWORD slice, VOID* ptr1, VOID* ptr2)
{
	__asm {
		push ebp
		mov ebp, esp
		push esi
		push edi

		mov esi, ptr1
		mov edi, ptr2

		sal edx, 2
		add esi, edx
		add edi, edx

		std
		repe cmpsd
		cld
		jz lbl_ret
		inc ecx

		lbl_ret:
		mov eax, ecx

		pop edi
		pop esi
		mov esp, ebp
		pop ebp

		retn 8
	}
}

BOOL __declspec(naked) __fastcall ForwardCompare(DWORD width, DWORD height, DWORD pitch, DWORD slice, VOID* ptr1, VOID* ptr2, POINT* p)
{
	__asm {
		push ebp
		mov ebp, esp
		sub esp, __LOCAL_SIZE
		push ebx
		push esi
		push edi

		mov height, edx
		mov eax, ecx

		mov ebx, pitch
		sub ebx, eax
		sal ebx, 2

		mov esi, ptr1
		mov edi, ptr2

		mov ecx, slice
		sal ecx, 2

		add esi, ecx
		add edi, ecx

		lbl_cycle:
			mov ecx, eax
			repe cmpsd
			jne lbl_break

			add esi, ebx
			add edi, ebx
		dec edx
		jnz lbl_cycle

		xor eax, eax
		jmp lbl_ret

		lbl_break:
		mov ebx, p
		
		inc ecx
		sub eax, ecx
		mov [ebx], eax

		mov eax, height
		sub eax, edx
		mov [ebx+4], eax

		xor eax, eax
		inc eax

		lbl_ret:
		pop edi
		pop esi
		pop ebx
		mov esp, ebp
		pop ebp

		retn 20
	}
}

BOOL __declspec(naked) __fastcall BackwardCompare(DWORD width, DWORD height, DWORD pitch, DWORD slice, VOID* ptr1, VOID* ptr2, POINT* p)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi

		mov eax, ecx

		mov ebx, pitch
		sub ebx, eax
		sal ebx, 2

		mov esi, ptr1
		mov edi, ptr2

		mov ecx, slice
		sal ecx, 2

		add esi, ecx
		add edi, ecx

		std

		lbl_cycle:
			mov ecx, eax
			repe cmpsd
			jnz lbl_break

			sub esi, ebx
			sub edi, ebx
		dec edx
		jnz lbl_cycle

		xor eax, eax
		jmp lbl_ret

		lbl_break:
		mov ebx, p
		
		mov [ebx], ecx

		dec edx
		mov [ebx+4], edx

		xor eax, eax
		inc eax

		lbl_ret:
		cld
		pop edi
		pop esi
		pop ebx
		mov esp, ebp
		pop ebp

		retn 20
	}
}

DWORD __declspec(naked) __fastcall ForwardCompare(DWORD width, DWORD height, DWORD pitch, DWORD slice, VOID* ptr1, VOID* ptr2)
{
	__asm {
		push ebp
		mov ebp, esp
		sub esp, __LOCAL_SIZE
		push ebx
		push esi
		push edi

		mov width, ecx
		mov eax, ecx

		mov ebx, pitch
		sub ebx, eax
		sal ebx, 2

		mov esi, ptr1
		mov edi, ptr2

		mov ecx, slice
		sal ecx, 2

		add esi, ecx
		add edi, ecx

		lbl_cycle:
			mov ecx, eax
			repe cmpsd
			jz lbl_inc
			
			sub eax, ecx
			dec eax
			jz lbl_ret

			sal ecx, 2
			add ebx, ecx
			add esi, ebx
			add edi, ebx
			add ebx, 4
			jmp lbl_cont

			lbl_inc:
			add esi, ebx
			add edi, ebx
			
			lbl_cont:
		dec edx
		jnz lbl_cycle

		lbl_ret:
		mov ecx, width
		sub ecx, eax
		mov eax, ecx

		pop edi
		pop esi
		pop ebx
		mov esp, ebp
		pop ebp

		retn 16
	}
}

DWORD __declspec(naked) __fastcall BackwardCompare(DWORD width, DWORD height, DWORD pitch, DWORD slice, VOID* ptr1, VOID* ptr2)
{
	__asm {
		push ebp
		mov ebp, esp
		sub esp, __LOCAL_SIZE
		push ebx
		push esi
		push edi

		mov width, ecx
		mov eax, ecx

		mov ebx, pitch
		sub ebx, eax
		sal ebx, 2

		mov esi, ptr1
		mov edi, ptr2

		mov ecx, slice
		sal ecx, 2

		add esi, ecx
		add edi, ecx

		std

		lbl_cycle:
			mov ecx, eax
			repe cmpsd
			jz lbl_inc
			
			sub eax, ecx
			dec eax
			jz lbl_ret

			sal ecx, 2
			add ebx, ecx
			sub esi, ebx
			sub edi, ebx
			add ebx, 4
			jmp lbl_cont

			lbl_inc:
			sub esi, ebx
			sub edi, ebx
			
			lbl_cont:
		dec edx
		jnz lbl_cycle

		lbl_ret:
		mov ecx, width
		sub ecx, eax
		mov eax, ecx

		cld

		pop edi
		pop esi
		pop ebx
		mov esp, ebp
		pop ebp

		retn 16
	}
}

PixelBuffer::PixelBuffer(BOOL isTrue)
{
	this->last = { 0, 0 };
	this->isTrue = isTrue;
	this->pitch = config.mode->width;
	if (!isTrue)
		this->pitch >>= 1;

	this->secondaryBuffer = new StateBufferAligned();

	this->fpsCounter = new FpsCounter(isTrue);
}

PixelBuffer::~PixelBuffer()
{
	delete this->secondaryBuffer;
	delete fpsCounter;
}

VOID PixelBuffer::Reset()
{
	this->last = { 0, 0 };
}

BOOL PixelBuffer::Update(StateBufferAligned** lpStateBuffer, BOOL checkOnly)
{
	this->primaryBuffer = *lpStateBuffer;

	if (fpsState)
		fpsCounter->Draw(this->primaryBuffer);

	BOOL res = FALSE;
	if (checkOnly)
	{
		if (this->primaryBuffer->size.width != this->last.width || this->primaryBuffer->size.height != this->last.height)
		{
			this->last = this->primaryBuffer->size;
			res = TRUE;
		}
		else if (this->last.width == config.mode->width && this->last.height == config.mode->height)
		{
			if (MemoryCompare(this->secondaryBuffer->data, this->primaryBuffer->data, config.mode->width * config.mode->height * (this->isTrue ? sizeof(DWORD) : sizeof(WORD))))
				res = TRUE;
		}
		else
		{
			DWORD left = (config.mode->width - this->last.width) >> 1;
			DWORD top = (config.mode->height - this->last.height) >> 1;

			DWORD width = this->last.width;
			if (!this->isTrue)
			{
				width >>= 1;
				left >>= 1;
			}

			POINT p;
			if (ForwardCompare(width, this->last.height, this->pitch, 
				top * this->pitch + left,
				this->primaryBuffer->data, this->secondaryBuffer->data, &p))
				res = TRUE;
		}
	}
	else if (this->primaryBuffer->size.width != this->last.width || this->primaryBuffer->size.height != this->last.height)
	{
		this->last = this->primaryBuffer->size;

		if (!this->isTrue)
			GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->last.width, this->last.height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (WORD*)this->primaryBuffer->data + ((config.mode->height - this->last.height) >> 1) * config.mode->width + ((config.mode->width - this->last.width) >> 1));
		else
			GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->last.width, this->last.height, GL_RGBA, GL_UNSIGNED_BYTE, (DWORD*)this->primaryBuffer->data + ((config.mode->height - this->last.height) >> 1) * config.mode->width + ((config.mode->width - this->last.width) >> 1));

		res = TRUE;
	}
	else if (this->last.width == config.mode->width && this->last.height == config.mode->height)
	{
		DWORD left, right;
		DWORD length = this->pitch * config.mode->height;
		if ((left = ForwardCompare(length, 0, (DWORD*)this->primaryBuffer->data, (DWORD*)this->secondaryBuffer->data))
			&& (right = BackwardCompare(length, length - 1, (DWORD*) this->primaryBuffer->data, (DWORD*)this->secondaryBuffer->data)))
		{
			DWORD top = (length - left) / this->pitch;
			DWORD bottom = (right - 1) / this->pitch + 1;

			POINT offset = { 0, 0 };
			for (DWORD y = top; y < bottom; y += BLOCK_SIZE)
			{
				DWORD bt = y + BLOCK_SIZE;
				if (bt > bottom)
					bt = bottom;

				for (DWORD x = 0; x < this->last.width; x += BLOCK_SIZE)
				{
					DWORD rt = x + BLOCK_SIZE;
					if (rt > this->last.width)
						rt = this->last.width;

					RECT rc = { *(LONG*)&x, *(LONG*)&y, *(LONG*)&rt, *(LONG*)&bt };
					this->UpdateBlock(&rc, &offset);
				}
			}

			res = TRUE;
		}
	}
	else
	{
		RECT offset;
		offset.left = (config.mode->width - this->last.width) >> 1;
		offset.top = (config.mode->height - this->last.height) >> 1;
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
		*lpStateBuffer = this->secondaryBuffer;
		this->secondaryBuffer = this->primaryBuffer;

		if (fpsState)
			fpsCounter->Calculate();
		return TRUE;
	}
	else if (fpsState == FpsBenchmark)
	{
		fpsCounter->Calculate();
		return TRUE;
	}

	return FALSE;
}

BOOL PixelBuffer::UpdateBlock(RECT* rect, POINT* offset)
{
	if (!this->isTrue)
	{
		rect->left >>= 1;
		rect->right >>= 1;
	}

	RECT rc;
	Size dim = { rect->right-- - rect->left, rect->bottom-- - rect->top };
	if (ForwardCompare(dim.width, dim.height, this->pitch, 
		rect->top * this->pitch + rect->left,
		this->primaryBuffer->data, this->secondaryBuffer->data, (POINT*)&rc.left)
		&& BackwardCompare(dim.width, dim.height, this->pitch, 
			rect->bottom * this->pitch + rect->right,
			this->primaryBuffer->data, this->secondaryBuffer->data, (POINT*)&rc.right))
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

		LONG height = rc.bottom - rc.top;
		if (height > 1)
		{
			if (rc.left != rect->left)
				rc.left -= ForwardCompare(rc.left - rect->left, height, this->pitch,
					(rc.top + 1) * this->pitch + rect->left,
					this->primaryBuffer->data, this->secondaryBuffer->data);

			if (rc.right != rect->right)
				rc.right += BackwardCompare(rect->right - rc.right, height, this->pitch,
					(rc.bottom - 1) * this->pitch + rect->right,
					this->primaryBuffer->data, this->secondaryBuffer->data);
		}

		Rect rect = { rc.left, rc.top, rc.right - rc.left + 1, height + 1 };
		DWORD* ptr = (DWORD*)this->primaryBuffer->data + rect.y * this->pitch + rect.x;

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
