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
#include "IDraw7.h"
#include "ExtraTypes.h"
#include "OpenDrawSurface.h"
#include "OpenDrawClipper.h"
#include "OpenDrawPalette.h"
#include "PixelBuffer.h"
#include "Renderer.h"

class OpenDraw : public IDraw7 {
private:
	OpenDrawClipper* clipperEntries;
	OpenDrawPalette* paletteEntries;

	WINDOWPLACEMENT windowPlacement;
	DWORD clearStage;

public:
	OpenDrawSurface* surfaceEntries;
	OpenDrawSurface* attachedSurface;

	HWND hWnd;
	HWND hDraw;
	Renderer* renderer;
	Viewport viewport;
	FilterState filterState;
	SnapshotType isTakeSnapshot;
	BOOL bufferIndex;
	DOUBLE flushTime;

	OpenDraw(IDrawUnknown**);
	~OpenDraw();

	BOOL CheckViewport(BOOL);
	VOID ScaleMouse(LPPOINT);

	VOID SetFullscreenMode();
	VOID SetWindowedMode();

	VOID RenderStart();
	VOID RenderStop();

	VOID Redraw();
	VOID LoadFilterState();

	VOID ReadFrameBufer(BYTE*, DWORD, BOOL);
	VOID ReadDataBuffer(BYTE*, VOID*, Size*, DWORD, BOOL, BOOL);
	VOID TakeSnapshot(Size*, VOID*, BOOL);

	// Inherited via  IDraw
	HRESULT __stdcall OpenDraw::QueryInterface(REFIID, LPVOID*) override;
	ULONG __stdcall Release() override;
	HRESULT __stdcall CreateClipper(DWORD, IDrawClipper**, IUnknown*) override;
	HRESULT __stdcall CreateSurface(LPDDSURFACEDESC2, IDrawSurface7**, IUnknown*) override;
	HRESULT __stdcall SetCooperativeLevel(HWND, DWORD) override;
	HRESULT __stdcall SetDisplayMode(DWORD, DWORD, DWORD, DWORD, DWORD) override;
	HRESULT __stdcall EnumDisplayModes(DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMMODESCALLBACK2) override;
	HRESULT __stdcall GetDisplayMode(LPDDSURFACEDESC2) override;
	HRESULT __stdcall CreatePalette(DWORD, LPPALETTEENTRY, IDrawPalette**, IUnknown*) override;
};