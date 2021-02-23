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
	struct LOCK {
		DWORD unk_0;
		DWORD unk_1;
		LONG flags;
		DWORD unk_3;
	};

	typedef DWORD*(__stdcall* BEGINLOCK)(LOCK*);
	typedef VOID(__thiscall* ENDLOCK)(LOCK*, DWORD);
	typedef BOOL(__stdcall* PRINTSTRING)(StringObject*, const CHAR*, CHAR*);
	typedef CHAR*(__thiscall* COPYSTRING)(StringObject*, const CHAR*, DWORD);

	//======================================================================= 

	#define BINKCOPYALL           0x80000000L
 
	#define BINKSURFACEP8 0
	#define BINKSURFACE24 1
	#define BINKSURFACE24R 2
	#define BINKSURFACE32 3
	#define BINKSURFACE32R 4
	#define BINKSURFACE32A 5
	#define BINKSURFACE32RA 6
	#define BINKSURFACE4444 7
	#define BINKSURFACE5551 8
	#define BINKSURFACE555 9
	#define BINKSURFACE565 10
	#define BINKSURFACE655 11
	#define BINKSURFACE664 12
	#define BINKSURFACEYUY2 13
	#define BINKSURFACEUYVY 14
	#define BINKSURFACEYV12 15
	#define BINKSURFACEMASK 15

	struct AddressSpaceV1 {
		VERSION version_major;
		VERSION version_minor;

		DWORD check;

		DWORD scroll_speed;
		DWORD scroll_nop;
		DWORD scroll_hook;
		DWORD scroll_key_hook;

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

		DWORD mouse_pos_fix;

		DWORD speed_anim;
	};

	struct AddressSpaceV2 {
		VERSION version;
		VERSION version_major;
		VERSION version_minor;
		
		DWORD check;
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
		DWORD scroll_key_hook;

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
		DWORD btnItemsUseFix;
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

		DWORD summoner_fix;

		DWORD lock_begin;
		DWORD lock_end;
		DWORD lock_begin_2;
		DWORD lock_end_2;
		DWORD interlockFix;

		DWORD iso_view_interface;
		DWORD iso_view_info_interface;
		DWORD iso_view_events_interface;
		DWORD iso_view_map_interface;
		DWORD radio_objects_interface;
		DWORD radio_objects_create;

		DWORD mouse_mid_button;
		DWORD mouse_allow_flag;
		DWORD map_center_get;
		DWORD map_center_set;

		DWORD mouse_pos_fix;
		DWORD mouse_pos_nop_1;
		DWORD mouse_pos_nop_2;

		DWORD startAiTurn;
		DWORD endAiTurn;

		DWORD speed_anim;
		DWORD speed_map;

		DWORD async_spell;
		DWORD async_death;
		DWORD async_ruin;

		DWORD sphere_x;
		DWORD sphere_y;

		DWORD resources_set;
		DWORD resources_nop;
		DWORD resources_hook;

		DWORD banners_set;
		DWORD banners_nop;
		DWORD banners_hook;
		DWORD banners_owner_trigger;

		DWORD locale_fix_1;
		DWORD locale_fix_2;

		DWORD scene_sort_hook;
		DWORD scene_print_edit_hook;
		DWORD scene_print_delete_hook;
		DWORD str_copy;
		DWORD str_print;

		DWORD clouds_hook;
		DWORD clouds_init;
		DWORD clouds_check;
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

	INT __stdcall MessageBoxHook(HWND, LPCSTR, LPCSTR, UINT);
	VOID PrintText(CHAR* str);
	VOID SetGameSpeed();

	VOID Load();
}