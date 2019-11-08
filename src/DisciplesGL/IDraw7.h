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

#include "IDrawUnknown.h"
#include "IDrawPalette.h"
#include "IDrawClipper.h"
#include "IDrawSurface7.h"

class IDraw7 : public IDrawUnknown {
public:
	IDraw7(IDrawUnknown** list);

	// Inherited via IDirectDraw7
	virtual HRESULT __stdcall Compact();
	virtual HRESULT __stdcall CreateClipper(DWORD, IDrawClipper**, IUnknown*);
	virtual HRESULT __stdcall CreatePalette(DWORD, LPPALETTEENTRY, IDrawPalette**, IUnknown*);
	virtual HRESULT __stdcall CreateSurface(LPDDSURFACEDESC2, IDrawSurface7**, IUnknown*);
	virtual HRESULT __stdcall DuplicateSurface(IDrawSurface7*, IDrawSurface7**);
	virtual HRESULT __stdcall EnumDisplayModes(DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMMODESCALLBACK2);
	virtual HRESULT __stdcall EnumSurfaces(DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMSURFACESCALLBACK7);
	virtual HRESULT __stdcall FlipToGDISurface();
	virtual HRESULT __stdcall GetCaps(LPDDCAPS, LPDDCAPS);
	virtual HRESULT __stdcall GetDisplayMode(LPDDSURFACEDESC2);
	virtual HRESULT __stdcall GetFourCCCodes(LPDWORD, LPDWORD);
	virtual HRESULT __stdcall GetGDISurface(IDrawSurface7**);
	virtual HRESULT __stdcall GetMonitorFrequency(LPDWORD);
	virtual HRESULT __stdcall GetScanLine(LPDWORD);
	virtual HRESULT __stdcall GetVerticalBlankStatus(LPBOOL);
	virtual HRESULT __stdcall Initialize(GUID*);
	virtual HRESULT __stdcall RestoreDisplayMode();
	virtual HRESULT __stdcall SetCooperativeLevel(HWND, DWORD);
	virtual HRESULT __stdcall SetDisplayMode(DWORD, DWORD, DWORD, DWORD, DWORD);
	virtual HRESULT __stdcall WaitForVerticalBlank(DWORD, HANDLE);
	virtual HRESULT __stdcall GetAvailableVidMem(LPDDSCAPS2, LPDWORD, LPDWORD);
	virtual HRESULT __stdcall GetSurfaceFromDC(HDC, IDrawSurface7**);
	virtual HRESULT __stdcall RestoreAllSurfaces();
	virtual HRESULT __stdcall TestCooperativeLevel();
	virtual HRESULT __stdcall GetDeviceIdentifier(LPDDDEVICEIDENTIFIER2, DWORD);
	virtual HRESULT __stdcall StartModeTest(LPSIZE, DWORD, DWORD);
	virtual HRESULT __stdcall EvaluateMode(DWORD, DWORD*);
};
