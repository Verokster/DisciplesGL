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
#include "IDraw7.h"
#include "ExtraTypes.h"
#include "OpenDrawSurface.h"
#include "OpenDrawClipper.h"
#include "OpenDrawPalette.h"

class OpenDraw : public IDraw7
{
public:
	OpenDrawSurface* surfaceEntries;
	OpenDrawClipper* clipperEntries;
	OpenDrawPalette* paletteEntries;

	OpenDrawSurface* attachedSurface;

	HWND hDraw;
	HDC hDc;
	HWND hWnd;

	DisplayMode mode;
	WINDOWPLACEMENT windowPlacement;

	HANDLE hDrawThread;
	HANDLE hDrawEvent;

	Viewport viewport;
	DWORD clearStage;
	BOOL isFinish;
	BOOL isStateChanged;
	BOOL isTakeSnapshot;

	OpenDraw(IDrawUnknown**);
	~OpenDraw();

	BOOL CheckView();
	VOID __fastcall ScaleMouse(LPPOINT);

	VOID SetFullscreenMode();
	VOID SetWindowedMode();

	VOID RenderStart();
	VOID RenderStop();

	VOID RenderOld();
	VOID RenderMid();
	VOID RenderNew();

	// Inherited via  IDraw
	HRESULT __stdcall OpenDraw::QueryInterface(REFIID, LPVOID*);
	ULONG __stdcall Release();
	HRESULT __stdcall CreateClipper(DWORD, IDrawClipper**, IUnknown*);
	HRESULT __stdcall CreateSurface(LPDDSURFACEDESC2, IDrawSurface7**, IUnknown*);
	HRESULT __stdcall SetCooperativeLevel(HWND, DWORD);
	HRESULT __stdcall SetDisplayMode(DWORD, DWORD, DWORD, DWORD, DWORD);
	HRESULT __stdcall EnumDisplayModes(DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMMODESCALLBACK2);
	HRESULT __stdcall GetDisplayMode(LPDDSURFACEDESC2);
	HRESULT __stdcall CreatePalette(DWORD, LPPALETTEENTRY, IDrawPalette**, IUnknown*);
};