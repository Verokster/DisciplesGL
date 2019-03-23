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

namespace Hooks
{
	struct AddressSpace
	{
		DWORD check;
		DWORD memory_1;
		DWORD memory_2;
		DWORD memory_3;
		DWORD memory_4;
		DWORD memory_5;
		DWORD pixel;

		DWORD fillColor;
		DWORD minimapGround;
		DWORD minimapObjects;
		DWORD clearGround;
		DWORD mapGround;
		DWORD waterBorders;

		DWORD symbol;
		DWORD faces;
		DWORD buildings;
		DWORD horLine;
		DWORD verLine;

		DWORD line_1;
		DWORD line_2;
		DWORD unknown_1;
		DWORD unknown_2;
	};

	struct BlendData
	{
		DWORD* srcData;
		DWORD* dstData;
		DWORD length;
		VOID* mskData;
	};

	struct BlitObject
	{
		BYTE isTrueColor;
		VOID* data;
		RECT rect;
		LONG pitch;
		DWORD color;
	};

	struct OffsetLength
	{
		LONG offset;
		LONG length;
	};

	struct PixObject
	{
		BYTE exists;
		DWORD color;
	};

	BOOL Load();
}