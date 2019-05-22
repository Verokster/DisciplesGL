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
#include "Config.h"
#include "Resource.h"
#include "PngLib.h"

OpenDrawSurface::OpenDrawSurface(IDrawUnknown** list, OpenDraw* lpDD, SurfaceType type)
{
	this->refCount = 1;
	this->list = list;
	this->last = *list;
	*list = this;

	this->ddraw = lpDD;

	this->attachedPalette = NULL;
	this->attachedClipper = NULL;
	this->attachedSurface = NULL;

	this->indexBuffer = NULL;
	this->secondaryBuffer = NULL;
	this->bufferIndex = FALSE;
	this->isCreated = FALSE;

	this->mode.width = 0;
	this->mode.height = 0;
	this->mode.bpp = 0;

	this->drawEnabled = TRUE;
	this->drawIndex = 0;

	BOOL isZoomed = config.zoomImage && config.isBorder;
	this->isZoomed[0] = isZoomed;
	this->isZoomed[1] = isZoomed;

	this->type = type;

	this->colorKey.dwColorSpaceLowValue = 0;
	this->colorKey.dwColorSpaceHighValue = 0;
}

OpenDrawSurface::~OpenDrawSurface()
{
	if (this->ddraw->attachedSurface == this)
		this->ddraw->attachedSurface = NULL;

	if (this->attachedPalette)
		this->attachedPalette->Release();

	if (this->attachedClipper)
		this->attachedClipper->Release();

	if (this->attachedSurface)
		this->attachedSurface->Release();

	this->ReleaseBuffer();
}

VOID OpenDrawSurface::ReleaseBuffer()
{
	if (this->isCreated)
	{
		this->isCreated = FALSE;

		if (this->indexBuffer)
		{
			AlignedFree(this->indexBuffer);
			this->indexBuffer = NULL;
		}

		if (this->secondaryBuffer)
		{
			AlignedFree(this->secondaryBuffer);
			this->secondaryBuffer = NULL;
		}

		this->bufferIndex = FALSE;
	}
}

VOID OpenDrawSurface::CreateBuffer(DWORD width, DWORD height, DWORD bpp, VOID* buffer)
{
	this->ReleaseBuffer();

	this->mode.width = width;
	this->mode.height = height;
	this->mode.bpp = bpp;

	if (buffer)
	{
		this->indexBuffer = buffer;
		this->secondaryBuffer = NULL;
	}
	else
	{
		bpp >>= 3;
		if (bpp == sizeof(BYTE) && this->type == SurfacePrimary || bpp == sizeof(WORD) && config.bpp32Hooked)
			bpp = sizeof(DWORD);

		DWORD size = width * height * bpp;
		this->indexBuffer = AlignedAlloc(size);
		MemoryZero(this->indexBuffer, size);

		if (!config.version && (this->type == SurfacePrimary || this->type == SurfaceSecondary && config.resHooked))
		{
			this->secondaryBuffer = AlignedAlloc(size);
			MemoryZero(this->secondaryBuffer, size);

			if (this->type == SurfaceSecondary)
			{
				ResourceStream stream;
				DWORD palette[256];

				{
					HRSRC hResource = FindResource(hDllModule, MAKEINTRESOURCE(IDR_BACK), RT_RCDATA);
					if (hResource)
					{
						HGLOBAL hResourceData = LoadResource(hDllModule, hResource);
						if (hResourceData)
						{
							stream.data = LockResource(hResourceData);
							if (stream.data)
							{
								stream.position = 0;

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
												BYTE** item = list;
												BYTE* row = data;
												DWORD count = info_ptr->height;
												while (count--)
												{
													*item++ = (BYTE*)row;
													row += info_ptr->rowbytes;
												}

												pnglib_read_image(png_ptr, list);
												MemoryFree(list);

												{
													BYTE* src = (BYTE*)info_ptr->palette;
													BYTE* dst = (BYTE*)palette;
													DWORD count = (DWORD)info_ptr->num_palette;
													if (count > 256)
														count = 256;
													while (count--)
													{
														*dst++ = *src++;
														*dst++ = *src++;
														*dst++ = *src++;
														*dst++ = 0;
													}
												}

												DWORD divX = width / (DWORD)info_ptr->width;
												DWORD modX = width % (DWORD)info_ptr->width;
												if (modX)
													++divX;

												DWORD divY = height / (DWORD)info_ptr->height;
												DWORD modY = height % (DWORD)info_ptr->height;
												if (modY)
													++divY;

												DWORD* dstH = (DWORD*)this->secondaryBuffer;

												DWORD divH = divY;
												while (divH--)
												{
													DWORD cheight = !divH && modY ? modY : (DWORD)info_ptr->height;

													DWORD* dstW = dstH;

													DWORD divW = divX;
													while (divW--)
													{
														DWORD cwidth = !divW && modX ? modX : (DWORD)info_ptr->width;

														BYTE* srcData = data;
														DWORD* dstData = dstW;

														DWORD copyHeight = cheight;
														while (copyHeight--)
														{
															BYTE* src = srcData;
															DWORD* dst = dstData;

															DWORD copyWidth = cwidth;
															if (info_ptr->pixel_depth == 4)
															{
																BOOL tick = FALSE;
																while (copyWidth--)
																{
																	BYTE c = tick ? (*src++ >> 4) : (*src & 0xF);
																	tick = !tick;
																	*dst++ = palette[c];
																}
															}
															else
															{
																while (copyWidth--)
																	*dst++ = palette[*src++];
															}

															srcData += info_ptr->rowbytes;
															dstData += width;
														}

														dstW += info_ptr->width;
													}

													dstH += info_ptr->height * width;
												}
											}
											MemoryFree(data);
										}
									}
								}

								pnglib_destroy_read_struct(&png_ptr, NULL, NULL);
							}
						}
					}
				}

				if (config.gameBorders)
				{
					HRSRC hResource = FindResource(hDllModule, MAKEINTRESOURCE(IDR_BORDER), RT_RCDATA);
					if (hResource)
					{
						HGLOBAL hResourceData = LoadResource(hDllModule, hResource);
						if (hResourceData)
						{
							stream.data = LockResource(hResourceData);
							if (stream.data)
							{
								stream.position = 0;

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
													while (count--)
													{
														*item++ = (BYTE*)row;
														row += info_ptr->rowbytes;
													}

													pnglib_read_image(png_ptr, list);
												}
												MemoryFree(list);

												{
													BYTE* src = (BYTE*)info_ptr->palette;
													BYTE* dst = (BYTE*)palette;
													DWORD count = (DWORD)info_ptr->num_palette;
													if (count > 256)
														count = 256;
													while (count--)
													{
														*dst++ = *src++;
														*dst++ = *src++;
														*dst++ = *src++;
														*dst++ = 0;
													}
												}

												LONG srcX, srcY, dstX, dstY;
												DWORD cWidth, cHeight;

												dstX = ((LONG)width - (LONG)info_ptr->width) >> 1;
												if (dstX < 0)
												{
													srcX = -dstX;
													dstX = 0;
													cWidth = width;
												}
												else
												{
													srcX = 0;
													cWidth = (DWORD)info_ptr->width;
												}

												dstY = ((LONG)height - (LONG)info_ptr->height) >> 1;
												if (dstY < 0)
												{
													srcY = -dstY;
													dstY = 0;
													cHeight = height;
												}
												else
												{
													srcY = 0;
													cHeight = (DWORD)info_ptr->height;
												}

												BYTE* srcData = data + srcY * info_ptr->width + srcX;
												DWORD* dstData = (DWORD*)this->secondaryBuffer + dstY * width + dstX;

												while (cHeight--)
												{
													BYTE* src = srcData;
													DWORD* dst = dstData;

													DWORD copyWidth = cWidth;
													if (info_ptr->pixel_depth == 4)
													{
														BOOL tick = FALSE;
														while (copyWidth--)
														{
															BYTE c = tick ? (*src++ >> 4) : (*src & 0xF);
															tick = !tick;
															*dst++ = palette[c];
														}
													}
													else
													{
														while (copyWidth--)
															*dst++ = palette[*src++];
													}

													srcData += info_ptr->rowbytes;
													dstData += width;
												}
											}
											MemoryFree(data);
										}
									}
								}

								pnglib_destroy_read_struct(&png_ptr, NULL, NULL);
							}
						}
					}
				}
			}
		}
		else
			this->secondaryBuffer = NULL;

		this->isCreated = TRUE;
	}

	this->bufferIndex = FALSE;
}

VOID OpenDrawSurface::Flush()
{
	OpenDrawSurface* surface = this->attachedSurface;

	if (this->mode.bpp == 8)
	{
		BYTE* src = (BYTE*)surface->indexBuffer;
		DWORD* dest = (DWORD*)this->indexBuffer;

		DWORD count = this->mode.width * this->mode.height;
		do
			*dest++ = this->attachedPalette->entries[*src++];
		while (--count);

		SetEvent(this->ddraw->hDrawEvent);
	}
	else
	{
		if (surface->drawEnabled && this->drawIndex < 1)
		{
			++this->drawIndex;

			BOOL index = this->bufferIndex;
			BOOL zoommed = surface->isZoomed[0];
			this->isZoomed[index] = zoommed;

			if (!zoommed)
			{
				VOID** lpBuffer = !index ? &this->indexBuffer : &this->secondaryBuffer;
				VOID* buffer = *lpBuffer;
				*lpBuffer = surface->indexBuffer;

				if (index != this->bufferIndex)
				{
					*lpBuffer = buffer;
					lpBuffer = index ? &this->indexBuffer : &this->secondaryBuffer;
					buffer = *lpBuffer;
				}

				surface->indexBuffer = buffer;
			}
			else
			{
				DWORD width, height;

				FLOAT k = (FLOAT)this->mode.width / this->mode.height;
				if (k >= 4.0f / 3.0f)
				{
					width = DWORD(GAME_HEIGHT_FLOAT * k);
					height = GAME_HEIGHT;
				}
				else
				{
					width = GAME_WIDTH;
					height = DWORD(GAME_WIDTH_FLOAT * this->mode.height / this->mode.width);
				}

				DWORD top = (this->mode.height - height) >> 1;
				DWORD left = (this->mode.width - width) >> 1;

				DWORD sctPitch = surface->mode.width;
				DWORD* srcData = (DWORD*)surface->indexBuffer + top * sctPitch + left;
				DWORD* dstData = (DWORD*)(!index ? this->indexBuffer : this->secondaryBuffer);

				DWORD count = width << 2;
				do
				{
					MemoryCopy(dstData, srcData, count);

					srcData += sctPitch;
					dstData += width;
				} while (--height);
			}

			SetEvent(this->ddraw->hDrawEvent);
		}

		surface->drawEnabled = this->drawIndex < 1;
	}

	Sleep(0);
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
	lpDDSurfaceDesc->lpSurface = this->indexBuffer;

	this->GetPixelFormat(&lpDDSurfaceDesc->ddpfPixelFormat);

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::GetAttachedSurface(LPDDSCAPS2 lpDDSCaps, IDrawSurface7** lplpDDAttachedSurface)
{
	if (!this->attachedSurface)
	{
		this->attachedSurface = new OpenDrawSurface((IDrawUnknown**)&this->ddraw->surfaceEntries, this->ddraw, SurfaceSecondary);
		this->attachedSurface->CreateBuffer(this->mode.width, this->mode.height, this->mode.bpp, NULL);
	}

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
	if (lpDDColorKey)
	{
		DWORD colorKey = lpDDColorKey->dwColorSpaceLowValue;
		this->colorKey.dwColorSpaceLowValue = colorKey;
		this->colorKey.dwColorSpaceHighValue = ((colorKey & 0x001F) << 3) | ((colorKey & 0x07E0) << 5) | ((colorKey & 0xF800) << 8);
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

			if (this->type == SurfaceSecondary)
			{
				this->isZoomed[0] = config.zoomImage && config.isBorder;

				if (config.resHooked && config.showBackBorder && config.isBorder)
					MemoryCopy(this->indexBuffer, this->secondaryBuffer, size);
				else
					MemoryZero(this->indexBuffer, size);
			}
			else
				MemoryZero(this->indexBuffer, size);
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
			DWORD* source = (DWORD*)surface->indexBuffer + lpSrcRect->top * sWidth + lpSrcRect->left;
			DWORD* destination = (DWORD*)this->indexBuffer + lpDestRect->top * dWidth + lpDestRect->left;

			do
			{
				MemoryCopy(destination, source, width * sizeof(DWORD));
				source += sWidth;
				destination += dWidth;
			} while (--height);
		}
		else
		{
			WORD* source = (WORD*)surface->indexBuffer + lpSrcRect->top * sWidth + lpSrcRect->left;
			WORD* destination = (WORD*)this->indexBuffer + lpDestRect->top * dWidth + lpDestRect->left;

			do
			{
				MemoryCopy(destination, source, width * sizeof(WORD));
				source += sWidth;
				destination += dWidth;
			} while (--height);
		}
	}

	return DD_OK;
}

HRESULT __stdcall OpenDrawSurface::BltFast(DWORD dwX, DWORD dwY, IDrawSurface7* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans)
{
	if (this->drawEnabled)
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
			DWORD* source = (DWORD*)surface->indexBuffer + lpSrcRect->top * sWidth + lpSrcRect->left;
			DWORD* destination = (DWORD*)this->indexBuffer + lpDestRect->top * dWidth + lpDestRect->left;

			DWORD colorKey = surface->colorKey.dwColorSpaceHighValue;
			if (colorKey)
			{
				do
				{
					DWORD* src = source;
					DWORD* dest = destination;

					DWORD count = width;
					do
					{
						if ((*src & 0xF8FCF8) != colorKey)
							*dest = *src;

						++src;
						++dest;
					} while (--count);

					source += sWidth;
					destination += dWidth;
				} while (--height);
			}
			else if (width == this->mode.width && width == surface->mode.width)
				MemoryCopy(destination, source, width * height * sizeof(DWORD));
			else
				do
				{
					MemoryCopy(destination, source, width * sizeof(DWORD));
					source += sWidth;
					destination += dWidth;
				} while (--height);
		}
		else
		{
			WORD* source = (WORD*)surface->indexBuffer + lpSrcRect->top * sWidth + lpSrcRect->left;
			WORD* destination = (WORD*)this->indexBuffer + lpDestRect->top * dWidth + lpDestRect->left;

			WORD colorKey = LOWORD(surface->colorKey.dwColorSpaceLowValue);
			if (colorKey)
			{
				do
				{
					WORD* src = source;
					WORD* dest = destination;

					DWORD count = width;
					do
					{
						if (*src != colorKey)
							*dest = *src;

						++src;
						++dest;
					} while (--count);

					source += sWidth;
					destination += dWidth;
				} while (--height);
			}
			else if (width == this->mode.width && width == surface->mode.width)
				MemoryCopy(destination, source, width * height * sizeof(WORD));
			else
				do
				{
					MemoryCopy(destination, source, width * sizeof(WORD));
					source += sWidth;
					destination += dWidth;
				} while (--height);
		}
	}

	return DD_OK;
}