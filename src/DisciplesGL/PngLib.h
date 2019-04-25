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

#include "png.h"

typedef png_structp(__cdecl *PNG_CREATE_READ_STRUCT)(png_const_charp user_png_ver, png_voidp error_ptr, png_error_ptr error_fn, png_error_ptr warn_fn);
typedef png_infop(__cdecl *PNG_CREATE_INFO_STRUCT)(png_structp png_ptr);
typedef VOID(__cdecl *PNG_SET_READ_FN)(png_structp png_ptr, png_voidp io_ptr, png_rw_ptr read_data_fn);
typedef VOID(__cdecl *PNG_DESTROY_READ_STRUCT)(png_structpp png_ptr_ptr, png_infopp info_ptr_ptr, png_infopp end_info_ptr_ptr);
typedef VOID(__cdecl *PNG_READ_INFO)(png_structp png_ptr, png_infop info_ptr);
typedef VOID(__cdecl *PNG_READ_IMAGE)(png_structp png_ptr, png_bytepp image);

extern PNG_CREATE_READ_STRUCT pnglib_create_read_struct;
extern PNG_CREATE_INFO_STRUCT pnglib_create_info_struct;
extern PNG_SET_READ_FN pnglib_set_read_fn;
extern PNG_DESTROY_READ_STRUCT pnglib_destroy_read_struct;
extern PNG_READ_INFO pnglib_read_info;
extern PNG_READ_IMAGE pnglib_read_image;

namespace PngLib
{
	VOID __cdecl ReadDataFromInputStream(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead);
}