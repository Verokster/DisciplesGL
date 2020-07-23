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
#include "OpenDrawSurface.h"
#include "OpenDraw.h"
#include "Hooks.h"
#include "Main.h"
#include "Config.h"
#include "Resource.h"
#include "PngLib.h"

OpenDrawSurface::OpenDrawSurface(IDrawUnknown** list, OpenDraw* lpDD, SurfaceType type, OpenDrawSurface* attached)
	: IDrawSurface7(list)
{
	this->ddraw = lpDD;

	this->type = type;

	this->attachedPalette = NULL;
	this->attachedClipper = NULL;

	this->attachedSurface = attached;
	if (attached)
		attached->attachedSurface = this;

	this->indexBuffer = NULL;
	this->secondaryBuffer = NULL;
	this->backBuffer = NULL;

	this->mode.width = 0;
	this->mode.height = 0;
	this->mode.bpp = 0;

	this->drawEnabled = TRUE;
	if (type == SurfaceSecondary)
		config.drawEnabled = TRUE;

	this->colorKey.dwColorSpaceLowValue = 0;
	this->colorKey.dwColorSpaceHighValue = 0;
}

OpenDrawSurface::~OpenDrawSurface()
{
	if (this->ddraw->attachedSurface == this)
	{
		this->ddraw->RenderStop();
		this->ddraw->attachedSurface = NULL;
	}

	if (this->attachedPalette)
		this->attachedPalette->Release();

	if (this->attachedClipper)
		this->attachedClipper->Release();

	this->ReleaseBuffer();
}

StateBufferBorder* __fastcall LoadBufferImage(LPSTR resourceId)
{
	StateBufferBorder* buffer = NULL;

	Stream stream;
	MemoryZero(&stream, sizeof(Stream));
	if (Main::LoadResource(resourceId, &stream))
	{
		png_structp png_ptr = pnglib_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		png_infop info_ptr = pnglib_create_info_struct(png_ptr);

		if (info_ptr)
		{
			pnglib_set_read_fn(png_ptr, &stream, PngLib::ReadDataFromInputStream);
			pnglib_read_info(png_ptr, info_ptr);

			if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
			{
				BYTE* data = (BYTE*)MemoryAlloc(info_ptr->height * info_ptr->rowbytes);
				if (data)
				{
					BYTE** list = (BYTE**)MemoryAlloc(info_ptr->height * sizeof(BYTE*));
					if (list)
					{
						{
							BYTE** item = list;
							BYTE* row = data;
							DWORD count = info_ptr->height;
							while (count)
							{
								--count;
								*item++ = (BYTE*)row;
								row += info_ptr->rowbytes;
							}

							pnglib_read_image(png_ptr, list);
						}
						MemoryFree(list);

						DWORD palette[256];
						{
							BYTE* src = (BYTE*)info_ptr->palette;
							BYTE* trs = (BYTE*)info_ptr->trans;
							BYTE* dst = (BYTE*)palette;

							DWORD colors = (DWORD)info_ptr->num_palette;
							DWORD trans = (DWORD)info_ptr->num_trans;
							while (colors)
							{
								--colors;
								*dst++ = *src++;
								*dst++ = *src++;
								*dst++ = *src++;

								if (trans)
								{
									--trans;
									*dst++ = *trs++;
								}
								else
									*dst++ = 0xFF;
							}

							if (config.renderer == RendererGDI)
							{
								DWORD* pal = palette;
								colors = (DWORD)info_ptr->num_palette;
								while (colors)
								{
									--colors;
									*pal++ = _byteswap_ulong(_rotl(*pal, 8));
								}
							}
						}

						buffer = new StateBufferBorder(info_ptr->width, info_ptr->height, info_ptr->width * info_ptr->height * sizeof(DWORD));

						BYTE* srcData = data;
						DWORD* dst = (DWORD*)buffer->data;

						DWORD cHeight = info_ptr->height;
						do
						{
							BYTE* src = srcData;

							DWORD cWidth = info_ptr->width;
							if (info_ptr->pixel_depth == 4)
							{
								BOOL tick = FALSE;
								do
								{
									*dst++ = palette[!tick ? (*src >> 4) : (*src++ & 0xF)];
									tick = !tick;
								} while (--cWidth);
							}
							else
							{
								do
									*dst++ = palette[*src++];
								while (--cWidth);
							}

							srcData += info_ptr->rowbytes;
						} while (--cHeight);
					}
					MemoryFree(data);
				}
			}
		}

		pnglib_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	}

	return buffer;
}

StateBufferBorder* __fastcall CreateBufferImage(DWORD width, DWORD height)
{
	StateBufferBorder* buffer = new StateBufferBorder(width, height, width * height * sizeof(DWORD));

	DWORD* data = (DWORD*)buffer->data;
	DWORD count = width * height;
	do
		*data++ = ALPHA_COMPONENT;
	while (--count);

	return buffer;
}

VOID OpenDrawSurface::DrawBorders(VOID* data, DWORD width, DWORD height)
{
	if (!config.borders.allowed)
		return;

	BOOL wide = config.battle.active && config.battle.wide;

	StateBufferBorder** lpBuffer = (StateBufferBorder**)(!wide ? &this->secondaryBuffer : &this->backBuffer);
	if ((*lpBuffer)->type != config.borders.type)
	{
		delete *lpBuffer;
		*lpBuffer = NULL;

		switch (config.borders.type)
		{
		case BordersClassic:
			*lpBuffer = LoadBufferImage(!wide ? MAKEINTRESOURCE(IDR_BORDER) : MAKEINTRESOURCE(IDR_BORDER_WIDE));
			break;

		case BordersAlternative:
			*lpBuffer = LoadBufferImage(!wide ? MAKEINTRESOURCE(IDR_ALT) : MAKEINTRESOURCE(IDR_ALT_WIDE));
			break;

		default:
			break;
		}

		if (!*lpBuffer)
		{
			if (!wide)
				*lpBuffer = CreateBufferImage(GAME_WIDTH, GAME_HEIGHT);
			else
				*lpBuffer = CreateBufferImage(WIDE_WIDTH, WIDE_HEIGHT);
		}
	}

	StateBufferBorder* srcBuffer = *lpBuffer;

	LONG srcX, srcY, dstX, dstY;
	DWORD cWidth, cHeight;

	dstX = ((LONG)width - (LONG)srcBuffer->width) >> 1;
	if (dstX < 0)
	{
		srcX = -dstX;
		dstX = 0;
		cWidth = width;
	}
	else
	{
		srcX = 0;
		cWidth = srcBuffer->width;
	}

	dstY = ((LONG)height - (LONG)srcBuffer->height) >> 1;
	if (dstY < 0)
	{
		srcY = -dstY;
		dstY = 0;
		cHeight = height;
	}
	else
	{
		srcY = 0;
		cHeight = srcBuffer->height;
	}

	DWORD* dstData = (DWORD*)data + dstY * width + dstX;
	if (srcBuffer->isAllocated)
	{
		DWORD* srcData = (DWORD*)srcBuffer->data + srcY * srcBuffer->width + srcX;

		while (cHeight)
		{
			--cHeight;
			MemoryCopy(dstData, srcData, cWidth * sizeof(DWORD));

			srcData += srcBuffer->width;
			dstData += width;
		}
	}
	else
	{
		while (cHeight)
		{
			--cHeight;
			DWORD* dst = dstData;

			DWORD count = cWidth;
			while (count)
			{
				--count;
				*dst++ = ALPHA_COMPONENT;
			}

			dstData += width;
		}
	}
}

VOID OpenDrawSurface::CreateBuffer(DWORD width, DWORD height, DWORD bpp, VOID* buffer)
{
	this->ReleaseBuffer();

	this->mode.width = width;
	this->mode.height = height;
	this->mode.bpp = bpp;

	if (buffer)
		this->indexBuffer = new StateBuffer(buffer);
	else
	{
		if (this->type == SurfacePrimary || this->type == SurfaceSecondary)
		{
			StateBufferAligned* buffer = new StateBufferAligned();
			this->indexBuffer = buffer;

			buffer->isZoomed = Config::IsZoomed();
			buffer->borders = config.borders.active ? config.borders.type : BordersNone;
			buffer->isBack = config.background.enabled;
		}
		else
		{
			bpp >>= 3;
			if (bpp == sizeof(BYTE) && this->type == SurfacePrimary || bpp == sizeof(WORD) && config.bpp32Hooked)
				bpp = sizeof(DWORD);

			this->indexBuffer = new StateBuffer(width * height * bpp);
		}

		if (this->type == SurfacePrimary)
		{
			this->secondaryBuffer = new StateBufferAligned();
			this->backBuffer = new StateBufferAligned();
		}
		else if (this->type == SurfaceSecondary && config.borders.allowed)
		{
			delete this->secondaryBuffer;
			this->secondaryBuffer = NULL;

			delete this->backBuffer;
			this->backBuffer = NULL;

			switch (config.borders.type)
			{
			case BordersClassic:
				this->secondaryBuffer = LoadBufferImage(MAKEINTRESOURCE(IDR_BORDER));
				this->backBuffer = LoadBufferImage(MAKEINTRESOURCE(IDR_BORDER_WIDE));
				break;

			case BordersAlternative:
				this->secondaryBuffer = LoadBufferImage(MAKEINTRESOURCE(IDR_ALT));
				this->backBuffer = LoadBufferImage(MAKEINTRESOURCE(IDR_ALT_WIDE));
				break;

			default:
				break;
			}

			if (!this->secondaryBuffer)
				this->secondaryBuffer = CreateBufferImage(GAME_WIDTH, GAME_HEIGHT);

			if (!this->backBuffer)
				this->backBuffer = CreateBufferImage(WIDE_WIDTH, WIDE_HEIGHT);
		}
	}
}

VOID OpenDrawSurface::ReleaseBuffer()
{
	delete this->indexBuffer;
	this->indexBuffer = NULL;
	delete this->secondaryBuffer;
	this->secondaryBuffer = NULL;
	delete this->backBuffer;
	this->backBuffer = NULL;
}

VOID OpenDrawSurface::SwapBuffers()
{
	((StateBufferAligned*)this->backBuffer)->isReady = TRUE;

	StateBuffer** lpBuffer = !this->ddraw->bufferIndex ? &this->indexBuffer : &this->secondaryBuffer;
	StateBuffer* buffer = *lpBuffer;
	*lpBuffer = this->backBuffer;

	this->backBuffer = buffer;

	this->ddraw->Redraw();
}

VOID OpenDrawSurface::Flush()
{
	OpenDrawSurface* surface = this->attachedSurface;

	if (this->mode.bpp == 8)
	{
		BYTE* src = (BYTE*)surface->indexBuffer->data;
		DWORD* dest = (DWORD*)this->backBuffer->data;

		DWORD count = this->mode.width * this->mode.height;

		if (config.renderer == RendererGDI)
		{
			do
				*dest++ = _byteswap_ulong(_rotl(this->attachedPalette->entries[*src++], 8)) | ALPHA_COMPONENT;
			while (--count);
		}
		else
		{
			do
				*dest++ = this->attachedPalette->entries[*src++] | ALPHA_COMPONENT;
			while (--count);
		}

		StateBufferAligned* surfaceBuffer = (StateBufferAligned*)this->backBuffer;

		surfaceBuffer->isZoomed = Config::IsZoomed();
		surfaceBuffer->size = surfaceBuffer->isZoomed ? config.zoom.size : *(Size*)config.mode;

		this->SwapBuffers();
		Sleep(0);
	}
	else
	{
		StateBufferAligned* surfaceBuffer = (StateBufferAligned*)surface->indexBuffer;
		if (surface->drawEnabled)
		{
			surfaceBuffer->isZoomed = Config::IsZoomed();

			if (config.borders.active)
			{
				surface->redraw = surfaceBuffer->borders != config.borders.type;
				surfaceBuffer->borders = config.borders.type;
			}
			else
				surfaceBuffer->borders = BordersNone;

			surfaceBuffer->isBack = config.background.enabled;
		}

		BOOL isSync = !config.ai.thinking && !config.ai.waiting && config.coldCPU;
		config.drawEnabled = surface->drawEnabled = isSync || config.singleThread || this->drawEnabled;
		if (surface->drawEnabled)
		{
			LONGLONG qp;
			QueryPerformanceFrequency((LARGE_INTEGER*)&qp);
			DOUBLE time = (DOUBLE)qp * 0.001;
			QueryPerformanceCounter((LARGE_INTEGER*)&qp);
			time = (DOUBLE)qp / time;

			if (time >= this->ddraw->flushTime)
			{
				this->ddraw->flushTime = config.syncStep * (DWORD(time / config.syncStep) + 1);

				this->drawEnabled = FALSE;

				if (surface->redraw)
				{
					surface->redraw = FALSE;

					MemoryZero(this->backBuffer->data, this->mode.width * this->mode.height * sizeof(DWORD));
					surface->DrawBorders(this->backBuffer->data, config.mode->width, config.mode->height);

					DWORD left = (this->mode.width - config.mode->width) >> 1;
					DWORD top = (this->mode.height - config.mode->height) >> 1;

					DWORD sctPitch = surface->mode.width;
					DWORD* src = (DWORD*)surface->indexBuffer->data + top * sctPitch + left;
					sctPitch -= config.mode->width;

					DWORD* dst = (DWORD*)this->backBuffer->data;

					DWORD height = config.mode->height;
					do
					{
						DWORD width = config.mode->width;
						do
						{
							DWORD alpha = *src >> 24;
							if (alpha == 255)
								*dst = *src;
							else if (alpha)
							{
								++alpha;

								DWORD s = *src & 0x000000FF;
								DWORD d = *dst & 0x000000FF;
								DWORD res = (d + ((s - d) * alpha) / 256) & 0x000000FF;

								s = *src & 0x0000FF00;
								d = *dst & 0x0000FF00;
								res |= (d + ((s - d) * alpha) / 256) & 0x0000FF00;

								s = *src & 0x00FF0000;
								d = *dst & 0x00FF0000;
								res |= (d + ((s - d) * alpha) / 256) & 0x00FF0000;

								*dst = res;
							}

							++src;
							++dst;
						} while (--width);

						src += sctPitch;
					} while (--height);

					StateBufferAligned* back = (StateBufferAligned*)this->backBuffer;
					back->isZoomed = surfaceBuffer->isZoomed;
					back->borders = surfaceBuffer->borders;
					back->isBack = surfaceBuffer->isBack;
				}
				else
				{
					StateBuffer* buffer = this->backBuffer;
					this->backBuffer = surface->indexBuffer;
					surface->indexBuffer = buffer;
				}

				((StateBufferAligned*)this->backBuffer)->size = surfaceBuffer->isZoomed ? config.zoom.size : *(Size*)config.mode;

				this->SwapBuffers();

				if (isSync)
				{
					QueryPerformanceFrequency((LARGE_INTEGER*)&qp);
					time = (DOUBLE)qp * 0.001;
					QueryPerformanceCounter((LARGE_INTEGER*)&qp);
					time = (DOUBLE)qp / time;

					INT sleep = INT(this->ddraw->flushTime - time);
					Sleep(sleep > 0 ? *(DWORD*)&sleep : 0u);
				}
				else
					Sleep(0);
			}
			else
				config.drawEnabled = surface->drawEnabled = FALSE;
		}
	}
}

ULONG __stdcall OpenDrawSurface::Release()
{
	if (--this->refCount)
		return this->refCount;

	delete this;
	return 0;
}

HRESULT __stdcall OpenDrawSurface::GetDDInterface(LPVOID* lplpDD)
{
	this->ddraw->AddRef();
	*lplpDD = this->ddraw;
	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::GetPixelFormat(LPDDPIXELFORMAT lpDDPixelFormat)
{
	lpDDPixelFormat->dwRGBBitCount = this->mode.bpp;

	if (this->mode.bpp == 8)
		lpDDPixelFormat->dwFlags = DDPF_PALETTEINDEXED8;
	else
	{
		lpDDPixelFormat->dwFlags = DDPF_RGB;
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
	this->GetSurfaceDesc(lpDDSurfaceDesc);

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

	lpDDSurfaceDesc->lpSurface = this->type != SurfaceSecondary || this->drawEnabled ? this->indexBuffer->data : (config.bpp32Hooked ? NULL : this->attachedSurface->backBuffer->data);

	this->GetPixelFormat(&lpDDSurfaceDesc->ddpfPixelFormat);

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::GetAttachedSurface(LPDDSCAPS2 lpDDSCaps, IDrawSurface7** lplpDDAttachedSurface)
{
	if (!this->attachedSurface)
	{
		this->attachedSurface = new OpenDrawSurface((IDrawUnknown**)&this->ddraw->surfaceEntries, this->ddraw, SurfaceSecondary, this);
		this->attachedSurface->CreateBuffer(this->mode.width, this->mode.height, this->mode.bpp, NULL);
	}
	else
		this->attachedSurface->AddRef();

	*lplpDDAttachedSurface = this->attachedSurface;

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::Flip(IDrawSurface7* lpSurface, DWORD dwFlags)
{
	this->Flush();
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
	if (lpDDColorKey && lpDDColorKey->dwColorSpaceLowValue)
	{
		DWORD colorKey = lpDDColorKey->dwColorSpaceLowValue;
		this->colorKey.dwColorSpaceLowValue = colorKey;
		this->colorKey.dwColorSpaceHighValue = ((colorKey & 0x001F) << 3) | ((colorKey & 0x07E0) << 5) | ((colorKey & 0xF800) << 8) | ALPHA_COMPONENT;
	}
	else
	{
		this->colorKey.dwColorSpaceLowValue = 0;
		this->colorKey.dwColorSpaceHighValue = 0;
	}

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::GetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey)
{
	// 8 - DDCKEY_SRCBLT
	lpDDColorKey->dwColorSpaceLowValue = this->colorKey.dwColorSpaceLowValue;
	lpDDColorKey->dwColorSpaceHighValue = this->colorKey.dwColorSpaceLowValue;

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::Blt(LPRECT lpDestRect, IDrawSurface7* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx)
{
	// 16778240 - DDBLT_WAIT | DDBLT_COLORFILL
	if (dwFlags & DDBLT_COLORFILL)
	{
		if (this->drawEnabled)
		{
			DWORD size = this->mode.width * this->mode.height * (this->mode.bpp == 8 ? 1 : (config.bpp32Hooked ? sizeof(DWORD) : sizeof(WORD)));
			MemoryZero(this->indexBuffer->data, size);

			if (this->type == SurfaceSecondary)
			{
				((StateBufferAligned*)this->indexBuffer)->borders = config.borders.active ? config.borders.type : BordersNone;
				this->DrawBorders(this->indexBuffer->data, this->mode.width, this->mode.height);
			}
		}
	}
	else if (this->type == SurfacePrimary && lpDDSrcSurface == this->attachedSurface)
		this->Flush();
	else if (this->drawEnabled)
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

		RECT dst;
		if (!lpDestRect)
		{
			dst.left = 0;
			dst.top = 0;
			dst.right = this->mode.width;
			dst.bottom = this->mode.height;
			lpDestRect = &dst;
		}

		DWORD sWidth;
		if (surface->attachedClipper)
		{
			POINT offset = { 0, 0 };
			ClientToScreen(surface->attachedClipper->hWnd, &offset);
			OffsetRect(lpSrcRect, -offset.x, -offset.y);

			sWidth = config.mode->width;
		}
		else
			sWidth = surface->mode.width;

		DWORD dWidth;
		if (this->attachedClipper)
		{
			POINT offset = { 0, 0 };
			ClientToScreen(this->attachedClipper->hWnd, &offset);
			OffsetRect(lpDestRect, -offset.x, -offset.y);

			dWidth = config.mode->width;
		}
		else
			dWidth = this->mode.width;

		INT width = lpSrcRect->right - lpSrcRect->left;
		INT height = lpSrcRect->bottom - lpSrcRect->top;

		if (config.bpp32Hooked)
		{
			DWORD* src = (DWORD*)surface->indexBuffer->data + lpSrcRect->top * sWidth + lpSrcRect->left;
			DWORD* dst = (DWORD*)this->indexBuffer->data + lpDestRect->top * dWidth + lpDestRect->left;

			if (Hooks::isBink)
			{
				Hooks::isBink = FALSE;

				sWidth -= width;
				dWidth -= width;

				do
				{
					DWORD cWidth = width;
					do
						*dst++ = *src++ | ALPHA_COMPONENT;
					while (--cWidth);

					src += sWidth;
					dst += dWidth;
				} while (--height);
			}
			else
			{
				do
				{
					MemoryCopy(dst, src, width * sizeof(DWORD));
					src += sWidth;
					dst += dWidth;
				} while (--height);
			}
		}
		else
		{
			WORD* src = (WORD*)surface->indexBuffer->data + lpSrcRect->top * sWidth + lpSrcRect->left;
			WORD* dst = (WORD*)this->indexBuffer->data + lpDestRect->top * dWidth + lpDestRect->left;

			do
			{
				MemoryCopy(dst, src, width * sizeof(WORD));
				src += sWidth;
				dst += dWidth;
			} while (--height);
		}
	}

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::BltFast(DWORD dwX, DWORD dwY, IDrawSurface7* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans)
{
	// 17 - DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY
	if (this->drawEnabled)
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

		LPRECT lpDestRect;
		RECT dest;
		{
			dest.left = dwX;
			dest.top = dwY;
			dest.right = dwX + lpSrcRect->right - lpSrcRect->left;
			dest.bottom = dwY + lpSrcRect->bottom - lpSrcRect->top;
			lpDestRect = &dest;
		}

		DWORD sWidth = surface->mode.width;
		DWORD dWidth = this->mode.width;

		INT width = lpSrcRect->right - lpSrcRect->left;
		INT height = lpSrcRect->bottom - lpSrcRect->top;

		if (config.bpp32Hooked)
		{
			DWORD* src = (DWORD*)surface->indexBuffer->data + lpSrcRect->top * sWidth + lpSrcRect->left;
			DWORD* dst = (DWORD*)this->indexBuffer->data + lpDestRect->top * dWidth + lpDestRect->left;

			DWORD colorKey = surface->colorKey.dwColorSpaceHighValue;
			if (colorKey)
			{
				sWidth -= width;
				dWidth -= width;

				do
				{
					DWORD count = width;
					do
					{
						if ((*src & COLORKEY_AND) != colorKey)
							*dst = *src;

						++src;
						++dst;
					} while (--count);

					src += sWidth;
					dst += dWidth;
				} while (--height);
			}
			else if (width == this->mode.width && width == surface->mode.width)
				MemoryCopy(dst, src, width * height * sizeof(DWORD));
			else
				do
				{
					MemoryCopy(dst, src, width * sizeof(DWORD));

					src += sWidth;
					dst += dWidth;
				} while (--height);
		}
		else
		{
			WORD* src = (WORD*)surface->indexBuffer->data + lpSrcRect->top * sWidth + lpSrcRect->left;
			WORD* dst = (WORD*)this->indexBuffer->data + lpDestRect->top * dWidth + lpDestRect->left;

			WORD colorKey = LOWORD(surface->colorKey.dwColorSpaceLowValue);
			if (colorKey)
			{
				sWidth -= width;
				dWidth -= width;

				do
				{
					DWORD count = width;
					do
					{
						if (*src != colorKey)
							*dst = *src;

						++src;
						++dst;
					} while (--count);

					src += sWidth;
					dst += dWidth;
				} while (--height);
			}
			else if (width == this->mode.width && width == surface->mode.width)
				MemoryCopy(dst, src, width * height * sizeof(WORD));
			else
				do
				{
					MemoryCopy(dst, src, width * sizeof(WORD));
					src += sWidth;
					dst += dWidth;
				} while (--height);
		}
	}

	return DD_OK;
}