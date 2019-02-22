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
#include "IDrawSurface7.h"
#include "ExtraTypes.h"
#include "OpenDrawClipper.h"
#include "OpenDrawPalette.h"

class OpenDraw;

class OpenDrawSurface : public IDrawSurface7
{
public:
	OpenDraw* ddraw;
	DDSCAPS2 caps;
	DWORD index;
	DisplayMode mode;

	OpenDrawClipper* attachedClipper;
	OpenDrawPalette* attachedPalette;
	OpenDrawSurface* attachedSurface;

	VOID* indexBuffer;
	DWORD colorKey;

	OpenDrawSurface(IDrawUnknown**, OpenDraw*, DWORD, LPDDSCAPS2);
	~OpenDrawSurface();

	VOID CreateBuffer(DWORD, DWORD, DWORD);
	VOID ReleaseBuffer();

	// Inherited via IDrawSurface
	ULONG __stdcall Release();
	HRESULT __stdcall Blt(LPRECT, IDrawSurface7*, LPRECT, DWORD, LPDDBLTFX);
	HRESULT __stdcall BltFast(DWORD, DWORD, IDrawSurface7*, LPRECT, DWORD);
	HRESULT __stdcall GetPixelFormat(LPDDPIXELFORMAT);
	HRESULT __stdcall GetSurfaceDesc(LPDDSURFACEDESC2);
	HRESULT __stdcall Lock(LPRECT, LPDDSURFACEDESC2, DWORD, HANDLE);
	HRESULT __stdcall SetClipper(IDrawClipper*);
	HRESULT __stdcall GetClipper(IDrawClipper**);
	HRESULT __stdcall SetColorKey(DWORD, LPDDCOLORKEY);

	HRESULT __stdcall GetAttachedSurface(LPDDSCAPS2, IDrawSurface7**);
	HRESULT __stdcall GetColorKey(DWORD, LPDDCOLORKEY);
	HRESULT __stdcall Flip(IDrawSurface7*, DWORD);
	HRESULT __stdcall SetPalette(IDrawPalette*);
	HRESULT __stdcall GetPalette(IDrawPalette**);
};

