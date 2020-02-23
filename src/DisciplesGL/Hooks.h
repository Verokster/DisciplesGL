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

#pragma once

namespace Hooks
{
	//======================================================================= 
	#define BINKYAINVERT          0x00000800L // Reverse Y and A planes when blitting (for debugging) 
	#define BINKFRAMERATE         0x00001000L // Override fr (call BinkFrameRate first) 
	#define BINKPRELOADALL        0x00002000L // Preload the entire animation 
	#define BINKSNDTRACK          0x00004000L // Set the track number to play 
	#define BINKOLDFRAMEFORMAT    0x00008000L // using the old Bink frame format (internal use only) 
	#define BINKRBINVERT          0x00010000L // use reversed R and B planes (internal use only) 
	#define BINKGRAYSCALE         0x00020000L // Force Bink to use grayscale 
	#define BINKNOMMX             0x00040000L // Don't use MMX 
	#define BINKNOSKIP            0x00080000L // Don't skip frames if falling behind 
	#define BINKALPHA             0x00100000L // Decompress alpha plane (if present) 
	#define BINKNOFILLIOBUF       0x00200000L // Don't Fill the IO buffer (in BinkOpen and BinkCopyTo) 
	#define BINKSIMULATE          0x00400000L // Simulate the speed (call BinkSim first) 
	#define BINKFILEHANDLE        0x00800000L // Use when passing in a file handle 
	#define BINKIOSIZE            0x01000000L // Set an io size (call BinkIOSize first) 
	#define BINKIOPROCESSOR       0x02000000L // Set an io processor (call BinkIO first) 
	#define BINKFROMMEMORY        0x04000000L // Use when passing in a pointer to the file 
	#define BINKNOTHREADEDIO      0x08000000L // Don't use a background thread for IO 
	#define BINKNOFRAMEBUFFERS    0x00000400L // Don't allocate internal frame buffers - application must call BinkRegisterFrameBuffers 
	#define BINKNOYPLANE          0x00000200L // Don't decompress the Y plane (internal flag) 
	#define BINKRUNNINGASYNC      0x00000100L // This frame is decompressing asynchronously 
 
	#define BINKSURFACEFAST       0x00000000L 
	#define BINKSURFACESLOW       0x08000000L 
	#define BINKSURFACEDIRECT     0x04000000L 
 
	#define BINKCOPYALL           0x80000000L // copy all pixels (not just changed) 
	#define BINKCOPY2XH           0x10000000L // Force doubling height scaling 
	#define BINKCOPY2XHI          0x20000000L // Force interleaving height scaling 
	#define BINKCOPY2XW           0x30000000L // copy the width zoomed by two 
	#define BINKCOPY2XWH          0x40000000L // copy the width and height zoomed by two 
	#define BINKCOPY2XWHI         0x50000000L // copy the width and height zoomed by two 
	#define BINKCOPY1XI           0x60000000L // copy the width and height zoomed by two 
	#define BINKCOPYNOSCALING     0x70000000L // Force scaling off 
 
	#define BINKSURFACEP8          0 
	#define BINKSURFACE24          1 
	#define BINKSURFACE24R         2 
	#define BINKSURFACE32          3 
	#define BINKSURFACE32R         4 
	#define BINKSURFACE32A         5 
	#define BINKSURFACE32RA        6 
	#define BINKSURFACE4444        7 
	#define BINKSURFACE5551        8 
	#define BINKSURFACE555         9 
	#define BINKSURFACE565        10 
	#define BINKSURFACE655        11 
	#define BINKSURFACE664        12 
	#define BINKSURFACEYUY2       13 
	#define BINKSURFACEUYVY       14 
	#define BINKSURFACEYV12       15 
	#define BINKSURFACEMASK       15 

	extern BOOL isBink;

	struct AddressSpaceV1 {
		DWORD check;
		DWORD value;
		DWORD scroll_speed;
		DWORD scroll_nop;
		DWORD scroll_hook;

		DWORD random_nop;

		DWORD res_mode_1;
		DWORD res_mode_2;
		DWORD res_linelist_hook;

		DWORD res_CreateDialog;
		DWORD res_CreateIsoDialog;

		DWORD res_LoadImage;
		DWORD res_StartDecodeImage;
		DWORD res_EndDecodeImage;

		DWORD dlg_Create;
		DWORD dlg_Delete;

		DWORD interlockFix;

		DWORD speed_anim;
	};

	struct AddressSpaceV2 {
		DWORD version;
		DWORD check_1;
		DWORD value_1;
		DWORD check_2;
		DWORD value_2;

		DWORD mapSize;
		DWORD bitmaskFix;

		DWORD random_nop;

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
		DWORD minimap_fill;

		DWORD maxSize_1;
		DWORD maxSize_2;
		DWORD maxSize_3;

		DWORD png_create_read_struct;
		DWORD png_create_info_struct;
		DWORD png_set_read_fn;
		DWORD png_destroy_read_struct;
		DWORD png_read_info;
		DWORD png_read_image;

		DWORD png_create_write_struct;
		DWORD png_set_write_fn;
		DWORD png_destroy_write_struct;
		DWORD png_write_info;
		DWORD png_write_image;
		DWORD png_write_end;
		DWORD png_set_filter;
		DWORD png_set_IHDR;

		DWORD scroll_speed;
		DWORD scroll_check;
		DWORD scroll_nop_1;
		DWORD scroll_nop_2;
		DWORD scroll_hook;

		DWORD dblclick_hook;

		DWORD btlClass;
		DWORD btlCentrBack;
		DWORD btlCentrUnits;
		DWORD btlReverseGroup;
		DWORD btlMouseCheck;
		DWORD btlSwapGroup;
		DWORD btlGroupsActive;
		DWORD btlGroupsInactive;
		DWORD btlInitGroups1_1;
		DWORD btlInitGroups1_2;
		DWORD btlInitGroups1_3;
		DWORD btlInitGroups2_1;
		DWORD btlInitGroups2_2;
		DWORD btlInitGroups2_3;
		DWORD btlImgIndices;
		DWORD btlDialog_1;
		DWORD btlDialog_2;
		DWORD btlFileGetStr;

		DWORD btlLoadBack_1;
		DWORD btlLoadBack_2;
		DWORD btlLoadBack_3;
		DWORD btlBackHeight;

		DWORD pkg_size;
		DWORD pkg_load;
		DWORD pkg_sub;
		DWORD pkg_entry_load;
		DWORD pkg_entry_sub;

		DWORD waitClass;
		DWORD waitHook;
		DWORD waitLoadCursor;
		DWORD waitShowCursor;

		DWORD debugPosition;
		DWORD msgIconPosition;
		DWORD msgTextPosition;
		DWORD msgTimeHook;

		DWORD print_text;
		DWORD print_init;
		DWORD print_deinit;

		DWORD ai_list_hook_1;
		DWORD ai_list_hook_2;
		DWORD ai_list_get_type;

		DWORD draw_hook_1;
		DWORD draw_hook_2;

		DWORD interlockFix;
		DWORD startAiTurn;
		DWORD endAiTurn;

		DWORD speed_anim;
		DWORD speed_map;
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

	INT __stdcall MessageBoxHook(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);

	VOID __fastcall PrintText(CHAR* str);

	VOID __fastcall SetGameSpeed();

	VOID Load();
}