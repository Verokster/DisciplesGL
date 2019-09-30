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
#include "PngLib.h"

PNG_CREATE_READ_STRUCT pnglib_create_read_struct;
PNG_CREATE_INFO_STRUCT pnglib_create_info_struct;
PNG_SET_READ_FN pnglib_set_read_fn;
PNG_DESTROY_READ_STRUCT pnglib_destroy_read_struct;
PNG_READ_INFO pnglib_read_info;
PNG_READ_IMAGE pnglib_read_image;

PNG_CREATE_WRITE_STRUCT pnglib_create_write_struct;
PNG_SET_WRITE_FN pnglib_set_write_fn;
PNG_DESTROY_WRITE_STRUCT pnglib_destroy_write_struct;
PNG_WRITE_INFO pnglib_write_info;
PNG_WRITE_IMAGE pnglib_write_image;
PNG_WRITE_END pnglib_write_end;
PNG_SET_FILTER pnglib_set_filter;
PNG_SET_IHDR pnglib_set_IHDR;

namespace PngLib
{
	VOID __cdecl ReadDataFromInputStream(png_structp png_ptr, png_bytep outBytes, png_size_t length)
	{
		Stream* stream = (Stream*)png_ptr->io_ptr;
		MemoryCopy(outBytes, stream->data + stream->position, length);
		stream->position += length;
	}

	VOID __cdecl WriteDataIntoOutputStream(png_structp png_ptr, png_bytep inBytes, png_size_t length)
	{
		Stream* stream = (Stream*)png_ptr->io_ptr;

		DWORD size = stream->position + length;
		if (stream->size < size)
		{
			stream->size = size;

			if (stream->size & 0xFFFF)
				stream->size = (stream->size & 0xFFFF0000) + 65536;

			VOID* data = MemoryAlloc(stream->size);
			if (stream->data)
			{
				MemoryCopy(data, stream->data, stream->position);
				MemoryFree(stream->data);
			}
			stream->data = (BYTE*)data;
		}

		MemoryCopy(stream->data + stream->position, inBytes, length);
		stream->position += length;
	}

	VOID __cdecl FlushData(png_structp png_ptr) {}
}