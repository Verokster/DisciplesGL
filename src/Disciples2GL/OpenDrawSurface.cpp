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
#include "OpenDrawSurface.h"
#include "OpenDraw.h"
#include "Main.h"

OpenDrawSurface::OpenDrawSurface(IDrawUnknown** list, OpenDraw* lpDD, DWORD index, LPDDSCAPS2 lpCaps)
{
	this->refCount = 1;
	this->list = list;
	this->last = *list;
	*list = this;

	this->ddraw = lpDD;

	this->attachedPalette = NULL;
	this->attachedClipper = NULL;
	this->attachedSurface = NULL;

	this->index = index;
	this->indexBuffer = NULL;
	this->caps = *lpCaps;

	this->mode.width = 0;
	this->mode.height = 0;
	this->mode.bpp = 0;

	this->colorKey = 0;
}

OpenDrawSurface::~OpenDrawSurface()
{
	this->ReleaseBuffer();
	IDrawDestruct(this);
}

VOID OpenDrawSurface::ReleaseBuffer()
{
	if (this->indexBuffer)
		MemoryFree(this->indexBuffer);
}

VOID OpenDrawSurface::CreateBuffer(DWORD width, DWORD height, DWORD bpp)
{
	this->ReleaseBuffer();
	this->mode.width = width;
	this->mode.height = height;
	this->mode.bpp = bpp;

	DWORD size = width * height * (bpp >> 3);
	this->indexBuffer = MemoryAlloc(size);
	MemoryZero(this->indexBuffer, size);
}

ULONG __stdcall OpenDrawSurface::Release()
{
	ULONG count = --this->refCount;
	if (!count)
	{
		if (this->ddraw->attachedSurface == this)
			this->ddraw->attachedSurface = NULL;

		if (this->attachedPalette)
			this->attachedPalette->Release();

		if (this->attachedClipper)
			this->attachedClipper->Release();

		if (this->attachedSurface)
			this->attachedSurface->Release();

		delete this;
	}

	return count;
}

HRESULT __stdcall OpenDrawSurface::GetPixelFormat(LPDDPIXELFORMAT lpDDPixelFormat)
{
	lpDDPixelFormat->dwRGBBitCount = this->mode.bpp;

	if (this->mode.bpp == 8)
		lpDDPixelFormat->dwFlags = DDPF_PALETTEINDEXED8;
	else
	{
		lpDDPixelFormat->dwFlags = DDPF_RGB;

		//lpDDPixelFormat->dwRBitMask = 0x7C00;
		//lpDDPixelFormat->dwGBitMask = 0x03E0;
		//lpDDPixelFormat->dwBBitMask = 0x001F;
		//lpDDPixelFormat->dwRGBAlphaBitMask = 0x8000;

		lpDDPixelFormat->dwRBitMask = 0xF800;
		lpDDPixelFormat->dwGBitMask = 0x07E0;
		lpDDPixelFormat->dwBBitMask = 0x001F;
		lpDDPixelFormat->dwRGBAlphaBitMask = 0x0000;
	}

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::Lock(LPRECT lpDestRect, LPDDSURFACEDESC2 lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent)
{
	// 2049 - DDLOCK_NOSYSLOCK | DDLOCK_WAIT 

	lpDDSurfaceDesc->dwWidth = this->mode.width;
	lpDDSurfaceDesc->dwHeight = this->mode.height;
	lpDDSurfaceDesc->lPitch = this->mode.width * (this->mode.bpp >> 3);
	lpDDSurfaceDesc->lpSurface = this->indexBuffer;

	this->GetPixelFormat(&lpDDSurfaceDesc->ddpfPixelFormat);

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::SetClipper(IDrawClipper* lpDDClipper)
{
	OpenDrawClipper* old = this->attachedClipper;
	this->attachedClipper = (OpenDrawClipper*)lpDDClipper;

	if (this->attachedClipper)
	{
		if (old != this->attachedClipper)
		{
			if (old)
				old->Release();

			this->attachedClipper->AddRef();
		}
	}
	else if (old)
		old->Release();

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::GetClipper(IDrawClipper** lplpDDClipper)
{
	if (this->attachedClipper)
		this->attachedClipper->AddRef();

	*lplpDDClipper = this->attachedClipper;

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::GetSurfaceDesc(LPDDSURFACEDESC2 lpDDSurfaceDesc)
{
	lpDDSurfaceDesc->dwWidth = this->mode.width;
	lpDDSurfaceDesc->dwHeight = this->mode.height;
	lpDDSurfaceDesc->lPitch = this->mode.width * (this->mode.bpp >> 3);
	lpDDSurfaceDesc->lpSurface = this->indexBuffer;

	this->GetPixelFormat(&lpDDSurfaceDesc->ddpfPixelFormat);

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::GetAttachedSurface(LPDDSCAPS2 lpDDSCaps, IDrawSurface7** lplpDDAttachedSurface)
{
	if (!this->attachedSurface)
	{
		this->attachedSurface = new OpenDrawSurface((IDrawUnknown**)&this->ddraw->surfaceEntries, this->ddraw, 1, lpDDSCaps);
		this->attachedSurface->CreateBuffer(this->mode.width, this->mode.height, this->mode.bpp);
	}

	this->attachedSurface->AddRef();
	*lplpDDAttachedSurface = this->attachedSurface;

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::Flip(IDrawSurface7* lpSurface, DWORD dwFlags)
{
	//if (this->attachedSurface)
	//{

	MemoryCopy(this->indexBuffer, this->attachedSurface->indexBuffer, this->mode.width * this->mode.height * (this->mode.bpp >> 3));
	SetEvent(this->ddraw->hDrawEvent);
	Sleep(0);

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::SetPalette(IDrawPalette* lpDDPalette)
{
	OpenDrawPalette* old = this->attachedPalette;
	this->attachedPalette = (OpenDrawPalette*)lpDDPalette;

	if (this->attachedPalette)
	{
		if (old != this->attachedPalette)
		{
			if (old)
				old->Release();

			this->attachedPalette->AddRef();
		}
	}
	else if (old)
		old->Release();

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::GetPalette(IDrawPalette** lplpDDPalette)
{
	if (this->attachedPalette)
		this->attachedPalette->AddRef();

	*lplpDDPalette = this->attachedPalette;

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::SetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey)
{
	// 8 - DDCKEY_SRCBLT 
	this->colorKey = lpDDColorKey ? lpDDColorKey->dwColorSpaceLowValue : 0;

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::GetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey)
{
	// 8 - DDCKEY_SRCBLT 
	lpDDColorKey->dwColorSpaceLowValue = this->colorKey;
	lpDDColorKey->dwColorSpaceHighValue = this->colorKey;

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::Blt(LPRECT lpDestRect, IDrawSurface7* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx)
{
	// 16778240 - DDBLT_WAIT | DDBLT_COLORFILL
	if (dwFlags & DDBLT_COLORFILL)
		MemoryZero(this->indexBuffer, this->mode.width * this->mode.height * (this->mode.bpp >> 3));
	else
	{
		if (!this->index && lpDDSrcSurface == this->attachedSurface)
		{
			MemoryCopy(this->indexBuffer, this->attachedSurface->indexBuffer, this->mode.width * this->mode.height * (this->mode.bpp >> 3));
			SetEvent(this->ddraw->hDrawEvent);
			Sleep(0);
		}
		else
		{
			OpenDrawSurface* surface = (OpenDrawSurface*)lpDDSrcSurface;

			RECT src;
			if (!lpSrcRect)
			{
				src.left = 0;
				src.top = 0;
				src.right = surface->mode.width;
				src.bottom = surface->mode.height;
				lpSrcRect = &src;
			}

			DWORD sWidth;
			if (surface->attachedClipper)
			{
				RECT clip;
				GetClientRect(surface->attachedClipper->hWnd, &clip);
				ClientToScreen(surface->attachedClipper->hWnd, (POINT*)&clip.left);

				lpSrcRect->left -= clip.left;
				lpSrcRect->top -= clip.top;
				lpSrcRect->right -= clip.left;
				lpSrcRect->bottom -= clip.top;

				sWidth = this->ddraw->mode.width;
			}
			else
				sWidth = surface->mode.width;

			DWORD dWidth;
			if (this->attachedClipper)
			{
				RECT clip;
				GetClientRect(this->attachedClipper->hWnd, &clip);
				ClientToScreen(this->attachedClipper->hWnd, (POINT*)&clip.left);

				lpDestRect->left -= clip.left;
				lpDestRect->top -= clip.top;
				lpDestRect->right -= clip.left;
				lpDestRect->bottom -= clip.top;

				dWidth = this->ddraw->mode.width;
			}
			else
				dWidth = this->mode.width;

			INT width = lpSrcRect->right - lpSrcRect->left;
			INT height = lpSrcRect->bottom - lpSrcRect->top;

			/*if (this->mode.bpp == 32)
			{
				DWORD* source = (DWORD*)surface->indexBuffer + lpSrcRect->top * sWidth + lpSrcRect->left;
				DWORD* destination = (DWORD*)this->indexBuffer + lpDestRect->top * dWidth + lpDestRect->left;

				do
				{
					MemoryCopy(destination, source, width << 2);
					source += sWidth;
					destination += dWidth;
				} while (--height);
			}
			else if (this->mode.bpp == 16)
			{*/
			WORD* source = (WORD*)surface->indexBuffer + lpSrcRect->top * sWidth + lpSrcRect->left;
			WORD* destination = (WORD*)this->indexBuffer + lpDestRect->top * dWidth + lpDestRect->left;

			do
			{
				MemoryCopy(destination, source, width << 1);
				source += sWidth;
				destination += dWidth;
			} while (--height);
			//}
		}
	}

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::BltFast(DWORD dwX, DWORD dwY, IDrawSurface7* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans)
{
	// 17 - DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY
	OpenDrawSurface* surface = (OpenDrawSurface*)lpDDSrcSurface;

	RECT src;
	if (!lpSrcRect)
	{
		src.left = 0;
		src.top = 0;
		src.right = surface->mode.width;
		src.bottom = surface->mode.height;
		lpSrcRect = &src;
	}

	LPRECT lpDestRect;
	RECT dest;
	{
		dest.left = dwX;
		dest.top = dwY;
		dest.right = dwX + lpSrcRect->right - lpSrcRect->left;
		dest.bottom = dwY + lpSrcRect->bottom - lpSrcRect->top;
		lpDestRect = &dest;
	}

	DWORD sWidth;
	if (surface->attachedClipper)
	{
		RECT clip;
		GetClientRect(surface->attachedClipper->hWnd, &clip);
		ClientToScreen(surface->attachedClipper->hWnd, (POINT*)&clip.left);

		lpSrcRect->left -= clip.left;
		lpSrcRect->top -= clip.top;
		lpSrcRect->right -= clip.left;
		lpSrcRect->bottom -= clip.top;

		sWidth = this->ddraw->mode.width;
	}
	else
		sWidth = surface->mode.width;

	DWORD dWidth;
	if (this->attachedClipper)
	{
		RECT clip;
		GetClientRect(this->attachedClipper->hWnd, &clip);
		ClientToScreen(this->attachedClipper->hWnd, (POINT*)&clip.left);

		lpDestRect->left -= clip.left;
		lpDestRect->top -= clip.top;
		lpDestRect->right -= clip.left;
		lpDestRect->bottom -= clip.top;

		dWidth = this->ddraw->mode.width;
	}
	else
		dWidth = this->mode.width;

	INT width = lpSrcRect->right - lpSrcRect->left;
	INT height = lpSrcRect->bottom - lpSrcRect->top;

	/*if (this->mode.bpp == 32)
	{
		DWORD* source = (DWORD*)surface->indexBuffer + lpSrcRect->top * sWidth + lpSrcRect->left;
		DWORD* destination = (DWORD*)this->indexBuffer + lpDestRect->top * dWidth + lpDestRect->left;

		if (surface->colorKey)
		{
			do
			{
				DWORD* src = source;
				DWORD* dest = destination;
				source += sWidth;
				destination += dWidth;

				DWORD count = width;
				do
				{
					if (*src != surface->colorKey)
						*dest = *src;

					++src;
					++dest;
				} while (--count);
			} while (--height);
		}
		else do
		{
			MemoryCopy(destination, source, width << 2);
			source += sWidth;
			destination += dWidth;
		} while (--height);
	}
	else if (this->mode.bpp == 16)
	{*/
	WORD* source = (WORD*)surface->indexBuffer + lpSrcRect->top * sWidth + lpSrcRect->left;
	WORD* destination = (WORD*)this->indexBuffer + lpDestRect->top * dWidth + lpDestRect->left;

	if (surface->colorKey)
	{
		do
		{
			WORD* src = source;
			WORD* dest = destination;
			source += sWidth;
			destination += dWidth;

			DWORD count = width;
			do
			{
				if (*src != LOWORD(surface->colorKey))
					*dest = *src;

				++src;
				++dest;
			} while (--count);
		} while (--height);
	}
	else do
	{
		MemoryCopy(destination, source, width << 1);
		source += sWidth;
		destination += dWidth;
	} while (--height);
	//}

	return DD_OK;
}