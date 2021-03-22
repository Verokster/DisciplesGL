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

#include "StdAfx.h"
#include "OpenDraw.h"
#include "Resource.h"
#include "CommCtrl.h"
#include "Main.h"
#include "Config.h"
#include "Window.h"
#include "Mods.h"
#include "Hooks.h"
#include "PngLib.h"
#include "Wingdi.h"
#include "ShaderProgram.h"
#include "GDIRenderer.h"
#include "OldRenderer.h"
#include "MidRenderer.h"
#include "NewRenderer.h"

VOID OpenDraw::ReadFrameBufer(BYTE* dstData, DWORD pitch, BOOL isBGR)
{
	BOOL res = FALSE;
	BOOL rev = FALSE;
	Rect* rect = &this->viewport.rectangle;

	if (this->renderer)
		res = this->renderer->ReadFrame(dstData, rect, pitch, isBGR, &rev);

	if (!res)
		MemoryZero(dstData, rect->height * pitch);
	else if (rev)
	{
		LONG height = rect->height;
		do
		{
			BYTE* src = dstData;
			LONG width = rect->width;
			do
			{
				BYTE dt = src[0];
				src[0] = src[2];
				src[2] = dt;

				src += 3;
			} while (--width);

			dstData += pitch;
		} while (--height);
	}
}

VOID OpenDraw::ReadDataBuffer(BYTE* dstData, VOID* srcData, Size* size, DWORD pitch, BOOL isTrueColor, BOOL isReverse)
{
	if (config.renderer == RendererGDI)
		isReverse = !isReverse;

	DWORD left = (config.mode->width - size->width) >> 1;
	DWORD top = (config.mode->height - size->height) >> 1;
	DWORD offset = (top + size->height - 1) * config.mode->width + left;

	if (!isTrueColor)
	{
		WORD* srcPtr = (WORD*)srcData + offset;
		DWORD height = size->height;
		do
		{
			WORD* src = srcPtr;
			BYTE* dst = dstData;
			DWORD width = size->width;
			do
			{
				WORD p = *src++;
				DWORD px = ((p & 0xF800) >> 8) | ((p & 0x07E0) << 5) | ((p & 0x001F) << 19);
				BYTE* c = (BYTE*)&px;

				if (!isReverse)
				{
					*dst++ = c[0];
					*dst++ = c[1];
					*dst++ = c[2];
				}
				else
				{
					*dst++ = c[2];
					*dst++ = c[1];
					*dst++ = c[0];
				}
			} while (--width);

			srcPtr -= config.mode->width;
			dstData += pitch;
		} while (--height);
	}
	else
	{
		DWORD* srcPtr = (DWORD*)srcData + offset;
		DWORD height = size->height;
		do
		{
			DWORD* src = srcPtr;
			BYTE* dst = dstData;
			DWORD width = size->width;
			do
			{
				DWORD px = *src++;
				BYTE* c = (BYTE*)&px;

				if (!isReverse)
				{
					*dst++ = c[0];
					*dst++ = c[1];
					*dst++ = c[2];
				}
				else
				{
					*dst++ = c[2];
					*dst++ = c[1];
					*dst++ = c[0];
				}
			} while (--width);

			srcPtr -= config.mode->width;
			dstData += pitch;
		} while (--height);
	}
}

VOID OpenDraw::TakeSnapshot(Size* size, VOID* stateData, BOOL isTrueColor)
{
	BOOL res = FALSE;
	BOOL isSurface = this->isTakeSnapshot == SnapshotSurfaceClipboard || this->isTakeSnapshot == SnapshotSurfaceFile;
	Size snapSize = isSurface ? *(Size*)size : *(Size*)&this->viewport.rectangle.width;

	switch (this->isTakeSnapshot)
	{
	case SnapshotClipboard:
	case SnapshotSurfaceClipboard: {
		if (OpenClipboard(NULL))
		{
			EmptyClipboard();

			DWORD pitch = snapSize.width * 3;
			if (pitch & 3)
				pitch = (pitch & 0xFFFFFFFC) + 4;

			DWORD dataSize = pitch * snapSize.height;
			{
				HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + dataSize);
				{
					VOID* data = GlobalLock(hMemory);
					{
						BITMAPINFOHEADER* bmiHeader = (BITMAPINFOHEADER*)data;
						MemoryZero(bmiHeader, sizeof(BITMAPINFOHEADER));

						bmiHeader->biSize = sizeof(BITMAPINFOHEADER);
						bmiHeader->biWidth = snapSize.width;
						bmiHeader->biHeight = snapSize.height;
						bmiHeader->biPlanes = 1;
						bmiHeader->biBitCount = 24;
						bmiHeader->biSizeImage = dataSize;
						bmiHeader->biXPelsPerMeter = 1;
						bmiHeader->biYPelsPerMeter = 1;

						BYTE* buffer = (BYTE*)data + sizeof(BITMAPINFOHEADER);
						if (isSurface)
							ReadDataBuffer(buffer, stateData, size, pitch, isTrueColor, TRUE);
						else
							ReadFrameBufer(buffer, pitch, TRUE);

						res = TRUE;
					}
					GlobalUnlock(hMemory);
					SetClipboardData(CF_DIB, hMemory);
				}
				GlobalFree(hMemory);
			}

			CloseClipboard();
		}
	}
	break;

	case SnapshotFile:
	case SnapshotSurfaceFile: {
		CHAR path[MAX_PATH];
		GetModuleFileName(NULL, path, sizeof(path));
		CHAR* ptr = StrLastChar(path, '\\');

		const CHAR* format;
		const CHAR* extension;
		DWORD offset;
		if (config.type.sacred)
		{
			StrCopy(ptr, "\\SDUMP???.*");
			format = "\\SDUMP%03d.%s";
			offset = 5;
		}
		else
		{
			StrCopy(ptr, "\\ScreenShots");
			ptr += sizeof("\\ScreenShots") - 1;

			DWORD dwAttrib = GetFileAttributes(path);
			if (dwAttrib == INVALID_FILE_ATTRIBUTES || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
				CreateDirectory(path, NULL);

			StrCopy(ptr, "\\ScreenShot???.*");
			format = "\\ScreenShot%03d.%s";
			offset = 10;
		}

		INT index = -1;
		WIN32_FIND_DATA fData;
		HANDLE hFind = FindFirstFile(path, &fData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				CHAR* p = fData.cFileName + offset;

				INT number = 0;
				DWORD count = 3;
				do
				{
					CHAR ch = *p++;
					if (ch < '0' || ch > '9')
					{
						number = 0;
						break;
					}

					INT dig = ch - '0';

					DWORD c = count;
					while (--c)
						dig *= 10;

					number += dig;
				} while (--count);

				if (index < number)
					index = number;
			} while (FindNextFile(hFind, &fData));

			FindClose(hFind);
		}

		DWORD pitch = snapSize.width * 3;
		if (pitch & 3)
			pitch = (pitch & 0xFFFFFFFC) + 4;

		Stream stream = { NULL, 0, 0 };
		if (config.snapshot.type == ImagePNG && pnglib_create_write_struct)
		{
			extension = "png";

			png_structp png_ptr = pnglib_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
			png_infop info_ptr = pnglib_create_info_struct(png_ptr);

			if (info_ptr)
			{
				pnglib_set_write_fn(png_ptr, &stream, PngLib::WriteDataIntoOutputStream, PngLib::FlushData);
				pnglib_set_filter(png_ptr, PNG_FILTER_TYPE_BASE, PNG_ALL_FILTERS);
				pnglib_set_IHDR(png_ptr, info_ptr, snapSize.width, snapSize.height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

				// png_set_compression_level
				png_ptr->flags |= PNG_FLAG_ZLIB_CUSTOM_LEVEL;
				png_ptr->zlib_level = config.snapshot.level;

				pnglib_write_info(png_ptr, info_ptr);

				BYTE* rowsData = (BYTE*)MemoryAlloc(pitch * snapSize.height);
				if (rowsData)
				{
					BYTE** list = (BYTE**)MemoryAlloc(snapSize.height * sizeof(BYTE*));
					if (list)
					{
						BYTE** item = list;
						DWORD count = snapSize.height;
						BYTE* row = rowsData + (count - 1) * pitch;
						while (count)
						{
							--count;
							*item++ = (BYTE*)row;
							row -= pitch;
						}

						if (isSurface)
							ReadDataBuffer(rowsData, stateData, size, pitch, isTrueColor, FALSE);
						else
							ReadFrameBufer(rowsData, pitch, FALSE);

						pnglib_write_image(png_ptr, list);
						pnglib_write_end(png_ptr, NULL);

						MemoryFree(list);
					}

					MemoryFree(rowsData);
				}
			}

			pnglib_destroy_write_struct(&png_ptr, &info_ptr);
		}
		else
		{
			extension = "bmp";

			stream.size = pitch * snapSize.height + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPCOREHEADER);
			stream.data = (BYTE*)MemoryAlloc(stream.size);
			if (stream.data)
			{
				stream.position = stream.size;

				BITMAPFILEHEADER* bmpHeader = (BITMAPFILEHEADER*)stream.data;
				bmpHeader->bfType = 0x4D42;
				bmpHeader->bfSize = stream.size;
				bmpHeader->bfReserved1 = NULL;
				bmpHeader->bfReserved2 = NULL;
				bmpHeader->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPCOREHEADER);

				BITMAPCOREHEADER* dibHeader = (BITMAPCOREHEADER*)(stream.data + sizeof(BITMAPFILEHEADER));
				dibHeader->bcSize = sizeof(BITMAPCOREHEADER);
				dibHeader->bcWidth = LOWORD(snapSize.width);
				dibHeader->bcHeight = LOWORD(snapSize.height);
				dibHeader->bcPlanes = 1;
				dibHeader->bcBitCount = 24;

				BYTE* dstData = stream.data + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPCOREHEADER);
				if (isSurface)
					ReadDataBuffer(dstData, stateData, size, pitch, isTrueColor, TRUE);
				else
					ReadFrameBufer(dstData, pitch, TRUE);
			}
		}

		if (stream.data)
		{
			StrPrint(ptr, format, ++index, extension);

			HANDLE hFile = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				DWORD write;
				DWORD written = 0;
				DWORD total = 0;

				do
				{
					total += written;
					write = stream.position - total;

					if (!write)
						break;

					if (write > 65536)
						write = 65536;
				} while (WriteFile(hFile, stream.data + total, write, &written, NULL));

				res = TRUE;

				CloseHandle(hFile);
			}

			MemoryFree(stream.data);
		}

		if (res)
		{
			StrCopy(snapshotName, ptr + 1);
			PostMessage(this->hWnd, config.msgSnapshot, FALSE, NULL);
		}
	}
	break;

	default:
		break;
	}

	this->isTakeSnapshot = SnapshotNone;

	if (res)
		MessageBeep(MB_ICONASTERISK);
}

VOID OpenDraw::Redraw()
{
	if (this->renderer)
		this->renderer->Redraw();
}

VOID OpenDraw::LoadFilterState()
{
	FilterState state;
	state.interpolation = config.image.interpolation;
	state.upscaling = config.image.upscaling;

	switch (state.upscaling)
	{
	case UpscaleScaleNx:
		state.value = config.image.scaleNx;
		break;

	case UpscaleScaleHQ:
		state.value = config.image.scaleHQ;
		break;

	case UpscaleXBRZ:
		state.value = config.image.xBRz;
		break;

	case UpscaleXSal:
		state.value = config.image.xSal;
		break;

	case UpscaleEagle:
		state.value = config.image.eagle;
		break;

	default:
		state.value = 0;
		break;
	}

	state.flags = TRUE;
	this->filterState = state;
}

VOID OpenDraw::RenderStart()
{
	this->RenderStop();

	if (!config.gl.version.real)
	{
		HWND hWnd = CreateWindowEx(NULL, WC_DRAW, NULL, WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, 1, 1, NULL, NULL, hDllModule, NULL);
		if (hWnd)
		{
			HDC hDc;
			HGLRC hRc = GL::Init(hWnd, &hDc);
			GL::Release(hWnd, &hDc, &hRc);

			DestroyWindow(hWnd);
		}
	}

	config.gl.version.value = config.gl.version.real;
	switch (config.renderer)
	{
	case RendererOpenGL1:
		if (config.gl.version.value > GL_VER_1_1)
			config.gl.version.value = GL_VER_1_2;
		break;

	case RendererOpenGL2:
		if (config.gl.version.value >= GL_VER_2_0)
			config.gl.version.value = GL_VER_2_0;
		else
			config.renderer = RendererAuto;
		break;

	case RendererOpenGL3:
		if (config.gl.version.value >= GL_VER_3_0)
			config.gl.version.value = GL_VER_3_0;
		else
			config.renderer = RendererAuto;
		break;

	case RendererGDI:
		config.gl.version.value = NULL;
		break;

	default:
		break;
	}

	if (config.gl.version.value >= GL_VER_3_0)
		(new NewRenderer(this))->Start();
	else if (config.gl.version.value >= GL_VER_2_0)
		(new MidRenderer(this))->Start();
	else if (config.gl.version.value >= GL_VER_1_1)
		(new OldRenderer(this))->Start();
	else
		(new GDIRenderer(this))->Start();
}

VOID OpenDraw::RenderStop()
{
	delete this->renderer;
	ClipCursor(NULL);
}

BOOL OpenDraw::CheckViewport(BOOL isDouble)
{
	if (this->viewport.refresh)
	{
		this->viewport.refresh = FALSE;

		this->viewport.rectangle.x = this->viewport.rectangle.y = 0;
		this->viewport.rectangle.width = this->viewport.width;
		this->viewport.rectangle.height = this->viewport.height;

		this->viewport.clipFactor.x = this->viewport.viewFactor.x = (FLOAT)this->viewport.width / config.mode->width;
		this->viewport.clipFactor.y = this->viewport.viewFactor.y = (FLOAT)this->viewport.height / config.mode->height;

		if (config.image.aspect && this->viewport.viewFactor.x != this->viewport.viewFactor.y)
		{
			if (this->viewport.viewFactor.x > this->viewport.viewFactor.y)
			{
				FLOAT fw = this->viewport.viewFactor.y * config.mode->width;
				this->viewport.rectangle.width = (INT)MathRound(fw);
				this->viewport.rectangle.x = (INT)MathRound(((FLOAT)this->viewport.width - fw) / 2.0f);
				this->viewport.clipFactor.x = this->viewport.viewFactor.y;
			}
			else
			{
				FLOAT fh = this->viewport.viewFactor.x * config.mode->height;
				this->viewport.rectangle.height = (INT)MathRound(fh);
				this->viewport.rectangle.y = (INT)MathRound(((FLOAT)this->viewport.height - fh) / 2.0f);
				this->viewport.clipFactor.y = this->viewport.viewFactor.x;
			}
		}

		HWND hActive;
		if (config.image.aspect && !config.windowedMode && config.borderless.mode == config.borderless.real && (hActive = GetForegroundWindow(), hActive == this->hWnd || hActive == this->hDraw))
		{
			RECT clipRect;
			GetClientRect(this->hWnd, &clipRect);

			clipRect.left = this->viewport.rectangle.x;
			clipRect.right = clipRect.left + this->viewport.rectangle.width;
			clipRect.bottom -= this->viewport.rectangle.y + this->viewport.offset;
			clipRect.top = clipRect.bottom - this->viewport.rectangle.height;

			ClientToScreen(this->hWnd, (POINT*)&clipRect.left);
			ClientToScreen(this->hWnd, (POINT*)&clipRect.right);

			ClipCursor(&clipRect);
		}
		else
			ClipCursor(NULL);

		this->clearStage = 0;
	}

	return this->clearStage++ <= *(DWORD*)&isDouble;
}

VOID OpenDraw::ScaleMouse(LPPOINT p)
{
	SIZE* sz = (SIZE*)&config.mode->width;
	if (!Config::IsZoomed())
	{
		p->x = LONG((FLOAT)((p->x - this->viewport.rectangle.x) * sz->cx) / this->viewport.rectangle.width);
		p->y = LONG((FLOAT)((p->y - this->viewport.rectangle.y) * sz->cy) / this->viewport.rectangle.height);
	}
	else
	{
		FLOAT kx = config.zoom.sizeFloat.width / config.mode->width;
		FLOAT ky = config.zoom.sizeFloat.height / config.mode->height;

		p->x = LONG((FLOAT)((p->x - this->viewport.rectangle.x) * sz->cx) / this->viewport.rectangle.width * kx) + ((config.mode->width - config.zoom.size.width) >> 1);
		p->y = LONG((FLOAT)((p->y - this->viewport.rectangle.y) * sz->cy) / this->viewport.rectangle.height * ky) + ((config.mode->height - config.zoom.size.height) >> 1);
	}
}

OpenDraw::OpenDraw(IDrawUnknown** list)
	: IDraw7(list)
{
	this->surfaceEntries = NULL;
	this->clipperEntries = NULL;
	this->paletteEntries = NULL;

	this->attachedSurface = NULL;

	this->hWnd = NULL;
	this->hDraw = NULL;

	this->renderer = NULL;

	this->isTakeSnapshot = SnapshotNone;
	this->bufferIndex = FALSE;

	this->flushTime = 0.0;

	MemoryZero(&this->windowPlacement, sizeof(WINDOWPLACEMENT));
}

OpenDraw::~OpenDraw()
{
	this->RenderStop();
}

VOID OpenDraw::SetFullscreenMode()
{
	if (config.windowedMode)
		GetWindowPlacement(hWnd, &this->windowPlacement);

	this->viewport.offset = config.borderless.mode ? BORDERLESS_OFFSET : 0;
	SetMenu(hWnd, NULL);

	SetWindowLong(this->hWnd, GWL_STYLE, WS_FULLSCREEN);
	SetWindowPos(this->hWnd, NULL, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) + this->viewport.offset, NULL);
}

VOID OpenDraw::SetWindowedMode()
{
	if (!this->windowPlacement.length)
	{
		this->windowPlacement.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(hWnd, &this->windowPlacement);

		RECT rect = { 0, 0, (LONG)config.mode->width, (LONG)config.mode->height };
		AdjustWindowRect(&rect, WS_WINDOWED, TRUE);

		LONG nWidth = rect.right - rect.left;
		LONG nHeight = rect.bottom - rect.top;

		RECT* rc = &this->windowPlacement.rcNormalPosition;
		rc->left = (GetSystemMetrics(SM_CXSCREEN) - nWidth) >> 1;
		if (rc->left < 0)
			rc->left = 0;

		rc->top = (GetSystemMetrics(SM_CYSCREEN) - nHeight) >> 1;
		if (rc->top < 0)
			rc->top = 0;

		rc->right = rc->left + nWidth;
		rc->bottom = rc->top + nHeight;

		this->windowPlacement.ptMinPosition.x = this->windowPlacement.ptMinPosition.y = -1;
		this->windowPlacement.ptMaxPosition.x = this->windowPlacement.ptMaxPosition.y = -1;

		this->windowPlacement.flags = NULL;
		this->windowPlacement.showCmd = SW_SHOWNORMAL;
	}

	this->viewport.offset = 0;
	SetMenu(hWnd, config.menu);

	SetWindowLong(this->hWnd, GWL_STYLE, WS_WINDOWED);
	SetWindowPlacement(this->hWnd, &this->windowPlacement);
}

HRESULT __stdcall OpenDraw::QueryInterface(REFIID id, LPVOID* lpObj)
{
	*lpObj = new OpenDraw((IDrawUnknown**)&drawList);
	return DD_OK;
}

ULONG __stdcall OpenDraw::Release()
{
	if (--this->refCount)
		return this->refCount;

	delete this;
	return 0;
}

HRESULT __stdcall OpenDraw::SetCooperativeLevel(HWND hWnd, DWORD dwFlags)
{
	this->hWnd = hWnd;
	Mods::SetHWND(hWnd);

	if (dwFlags & DDSCL_NORMAL)
	{
		this->SetWindowedMode();
		this->RenderStart();
	}

	return DD_OK;
}

HRESULT __stdcall OpenDraw::SetDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP, DWORD dwRefreshRate, DWORD dwFlags)
{
	config.mode->width = dwWidth;
	config.mode->height = dwHeight;
	config.mode->bpp = dwBPP;

	this->SetFullscreenMode();
	this->RenderStart();

	return DD_OK;
}

HRESULT __stdcall OpenDraw::CreateSurface(LPDDSURFACEDESC2 lpDDSurfaceDesc, IDrawSurface7** lplpDDSurface, IUnknown* pUnkOuter)
{
	// 1 - DDSD_CAPS // 512 - DDSCAPS_PRIMARYSURFACE
	// 33 - DDSD_BACKBUFFERCOUNT | DDSD_CAPS // 536 - DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX
	// 4103 - DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS // 2112 - DDSCAPS_SYSTEMMEMORY | DDSCAPS_OFFSCREENPLAIN
	// 7 - DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS // 64 - DDSCAPS_OFFSCREENPLAIN
	// 7 - DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS // 2112 - DDSCAPS_SYSTEMMEMORY | DDSCAPS_OFFSCREENPLAIN
	// 4103 - DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS // 64 - // 268451904 - DDSCAPS_LOCALVIDMEM | DDSCAPS_VIDEOMEMORY | DDSCAPS_OFFSCREENPLAIN
	// 4103 - DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS // 64 - // 0
	// 6159 - DDSD_PIXELFORMAT | DDSD_LPSURFACE | DDSD_PITCH | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS // 2112 - DDSCAPS_SYSTEMMEMORY | DDSCAPS_OFFSCREENPLAIN

	VOID* buffer = NULL;
	if (lpDDSurfaceDesc->dwFlags & DDSD_LPSURFACE)
		buffer = lpDDSurfaceDesc->lpSurface;

	OpenDrawSurface* surface;

	if (lpDDSurfaceDesc->dwFlags == (DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS))
	{
		surface = new OpenDrawSurface((IDrawUnknown**)&this->surfaceEntries, this, SurfaceSecondary, this->attachedSurface);
		surface->CreateBuffer(lpDDSurfaceDesc->dwWidth, lpDDSurfaceDesc->dwHeight, config.mode->bpp, buffer);
	}
	else
	{
		surface = new OpenDrawSurface((IDrawUnknown**)&this->surfaceEntries, this, (lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) ? SurfacePrimary : SurfaceOther, NULL);

		if (surface->type == SurfacePrimary)
			this->attachedSurface = surface;

		if (lpDDSurfaceDesc->dwFlags & (DDSD_WIDTH | DDSD_HEIGHT))
			surface->CreateBuffer(lpDDSurfaceDesc->dwWidth, lpDDSurfaceDesc->dwHeight, (lpDDSurfaceDesc->dwFlags & DDSD_PIXELFORMAT) ? lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount : config.mode->bpp, buffer);
		else
			surface->CreateBuffer(config.mode->width, config.mode->height, config.mode->bpp, buffer);
	}

	*lplpDDSurface = surface;

	return DD_OK;
}

HRESULT __stdcall OpenDraw::CreateClipper(DWORD dwFlags, IDrawClipper** lplpDDClipper, IUnknown* pUnkOuter)
{
	OpenDrawClipper* clipper = new OpenDrawClipper((IDrawUnknown**)&this->clipperEntries, this);
	*lplpDDClipper = clipper;

	return DD_OK;
}

HRESULT __stdcall OpenDraw::EnumDisplayModes(DWORD dwFlags, LPDDSURFACEDESC2 lpDDSurfaceDesc, LPVOID lpContext, LPDDENUMMODESCALLBACK2 lpEnumModesCallback)
{
	DDSURFACEDESC2 ddSurfaceDesc;
	MemoryZero(&ddSurfaceDesc, sizeof(DDSURFACEDESC2));

	DisplayMode* mode = modesList;
	DWORD count = sizeof(modesList) / sizeof(DisplayMode);
	do
	{
		ddSurfaceDesc.dwWidth = mode->width;
		ddSurfaceDesc.dwHeight = mode->height;
		ddSurfaceDesc.ddpfPixelFormat.dwRGBBitCount = mode->bpp;
		ddSurfaceDesc.ddpfPixelFormat.dwFlags = DDSD_ZBUFFERBITDEPTH;
		if (!lpEnumModesCallback(&ddSurfaceDesc, lpContext))
			break;

		++mode;
	} while (--count);

	return DD_OK;
}

HRESULT __stdcall OpenDraw::GetDisplayMode(LPDDSURFACEDESC2 lpDDSurfaceDesc)
{
	lpDDSurfaceDesc->dwWidth = config.mode->width;
	lpDDSurfaceDesc->dwHeight = config.mode->height;
	lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount = config.mode->bpp;
	lpDDSurfaceDesc->ddpfPixelFormat.dwFlags = DDPF_PALETTEINDEXED8;
	return DD_OK;
}

HRESULT __stdcall OpenDraw::CreatePalette(DWORD dwFlags, LPPALETTEENTRY lpDDColorArray, IDrawPalette** lplpDDPalette, IUnknown* pUnkOuter)
{
	OpenDrawPalette* palette = new OpenDrawPalette((IDrawUnknown**)&this->paletteEntries, this);
	*lplpDDPalette = palette;
	MemoryCopy(palette->entries, lpDDColorArray, 256 * sizeof(PALETTEENTRY));

	return DD_OK;
}