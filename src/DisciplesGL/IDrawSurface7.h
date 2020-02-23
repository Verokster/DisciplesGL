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

#include "IDrawUnknown.h"
#include "IDrawPalette.h"
#include "IDrawClipper.h"

class IDrawSurface7 : public IDrawUnknown {
public:
	IDrawSurface7(IDrawUnknown** list);

	// Inherited via IDirectDrawSurface7
	virtual HRESULT __stdcall AddAttachedSurface(IDrawSurface7*);
	virtual HRESULT __stdcall AddOverlayDirtyRect(LPRECT);
	virtual HRESULT __stdcall Blt(LPRECT, IDrawSurface7*, LPRECT, DWORD, LPDDBLTFX);
	virtual HRESULT __stdcall BltBatch(LPDDBLTBATCH, DWORD, DWORD);
	virtual HRESULT __stdcall BltFast(DWORD, DWORD, IDrawSurface7*, LPRECT, DWORD);
	virtual HRESULT __stdcall DeleteAttachedSurface(DWORD, IDrawSurface7*);
	virtual HRESULT __stdcall EnumAttachedSurfaces(LPVOID, LPDDENUMSURFACESCALLBACK7);
	virtual HRESULT __stdcall EnumOverlayZOrders(DWORD, LPVOID, LPDDENUMSURFACESCALLBACK7);
	virtual HRESULT __stdcall Flip(IDrawSurface7*, DWORD);
	virtual HRESULT __stdcall GetAttachedSurface(LPDDSCAPS2, IDrawSurface7**);
	virtual HRESULT __stdcall GetBltStatus(DWORD);
	virtual HRESULT __stdcall GetCaps(LPDDSCAPS2);
	virtual HRESULT __stdcall GetClipper(IDrawClipper**);
	virtual HRESULT __stdcall GetColorKey(DWORD, LPDDCOLORKEY);
	virtual HRESULT __stdcall GetDC(HDC*);
	virtual HRESULT __stdcall GetFlipStatus(DWORD);
	virtual HRESULT __stdcall GetOverlayPosition(LPLONG, LPLONG);
	virtual HRESULT __stdcall GetPalette(IDrawPalette**);
	virtual HRESULT __stdcall GetPixelFormat(LPDDPIXELFORMAT);
	virtual HRESULT __stdcall GetSurfaceDesc(LPDDSURFACEDESC2);
	virtual HRESULT __stdcall Initialize(LPDIRECTDRAW, LPDDSURFACEDESC2);
	virtual HRESULT __stdcall IsLost();
	virtual HRESULT __stdcall Lock(LPRECT, LPDDSURFACEDESC2, DWORD, HANDLE);
	virtual HRESULT __stdcall ReleaseDC(HDC);
	virtual HRESULT __stdcall Restore();
	virtual HRESULT __stdcall SetClipper(IDrawClipper*);
	virtual HRESULT __stdcall SetColorKey(DWORD, LPDDCOLORKEY);
	virtual HRESULT __stdcall SetOverlayPosition(LONG, LONG);
	virtual HRESULT __stdcall SetPalette(IDrawPalette*);
	virtual HRESULT __stdcall Unlock(LPRECT);
	virtual HRESULT __stdcall UpdateOverlay(LPRECT, IDrawSurface7*, LPRECT, DWORD, LPDDOVERLAYFX);
	virtual HRESULT __stdcall UpdateOverlayDisplay(DWORD);
	virtual HRESULT __stdcall UpdateOverlayZOrder(DWORD, IDrawSurface7*);
	virtual HRESULT __stdcall GetDDInterface(LPVOID*);
	virtual HRESULT __stdcall PageLock(DWORD);
	virtual HRESULT __stdcall PageUnlock(DWORD);
	virtual HRESULT __stdcall SetSurfaceDesc(LPDDSURFACEDESC2, DWORD);
	virtual HRESULT __stdcall SetPrivateData(REFGUID, LPVOID, DWORD, DWORD);
	virtual HRESULT __stdcall GetPrivateData(REFGUID, LPVOID, LPDWORD);
	virtual HRESULT __stdcall FreePrivateData(REFGUID);
	virtual HRESULT __stdcall GetUniquenessValue(LPDWORD);
	virtual HRESULT __stdcall ChangeUniquenessValue();
	virtual HRESULT __stdcall SetPriority(DWORD);
	virtual HRESULT __stdcall GetPriority(LPDWORD);
	virtual HRESULT __stdcall SetLOD(DWORD);
	virtual HRESULT __stdcall GetLOD(LPDWORD);
};