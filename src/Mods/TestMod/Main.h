/*
	MIT License

	Copyright (c) 2021 Oleksiy Ryabchun

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

#include "ExtraTypes.h"



namespace Main
{
	extern "C"
	{
		/// <summary>
		/// Get mod name. Uses for menu
		/// </summary>
		/// <returns></returns>
		const CHAR* __stdcall GetName();

		/// <summary>
		/// Get mod menu handle
		/// </summary>
		/// <returns>if returns NULL, menu will not be visible </returns>
		HMENU __stdcall GetMenu();

		/// <summary>
		/// Set window handle for window manipulations
		/// </summary>
		/// <param name="hWnd"></param>
		/// <returns></returns>
		VOID __stdcall SetHWND(HWND hWnd);

		/// <summary>
		/// Pre-rendering frame callback, for any blend operations
		/// </summary>
		/// <param name="width">width in pixels</param>
		/// <param name="height">height in pixels</param>
		/// <param name="pitch">row pitch in bytes</param>
		/// <param name="pixelFormat">pixel format flags</param>
		/// <param name="buffer">frame buffer pointer</param>
		/// <returns></returns>
		VOID __stdcall DrawFrame(DWORD width, DWORD height, LONG pitch, DWORD pixelFormat, VOID* buffer);
	}
}