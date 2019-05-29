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
	struct AddressSpaceV1 {
		DWORD check;
		DWORD value;
		DWORD scroll_speed;
		DWORD scroll_nop;
		DWORD scroll_hook;
	};

	struct AddressSpaceV2 {
		DWORD check_1;
		DWORD value_1;
		DWORD check_2;
		DWORD value_2;

		DWORD memory_1;
		DWORD memory_2;
		DWORD memory_3;
		DWORD memory_4;
		DWORD memory_5;
		DWORD memory_6;
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

		DWORD res_hook;
		DWORD res_back;
		DWORD res_restriction_jmp;
		DWORD res_restriction_nop;

		DWORD border_nop;
		DWORD border_hook;

		DWORD blit_size;
		DWORD blit_patch_1;
		DWORD blit_patch_2;

		DWORD mini_rect_jmp;
		DWORD mini_rect_patch;

		DWORD right_curve;

		DWORD snapshot_size;
		DWORD snapshot_rect;
		DWORD snapshot_nop1;
		DWORD snapshot_hook;
		DWORD snapshot_nop2;

		DWORD maxSize_1;
		DWORD maxSize_2;
		DWORD maxSize_3;

		DWORD png_create_read_struct;
		DWORD png_create_info_struct;
		DWORD png_set_read_fn;
		DWORD png_destroy_read_struct;
		DWORD png_read_info;
		DWORD png_read_image;

		DWORD scroll_speed;
		DWORD scroll_check;
		DWORD scroll_nop_1;
		DWORD scroll_nop_2;
		DWORD scroll_hook;

		DWORD dblclick_hook;
	};

	struct BlendData {
		DWORD* srcData;
		DWORD* dstData;
		DWORD length;
		VOID* mskData;
	};

	struct BlitObject {
		BYTE isTrueColor;
		VOID* data;
		RECT rect;
		LONG pitch;
		DWORD color;
	};

	struct OffsetLength {
		LONG offset;
		LONG length;
	};

	struct PixObject {
		BYTE exists;
		DWORD color;
	};

	BOOL Load();
}