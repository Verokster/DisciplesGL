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
#include "GDIRenderer.h"
#include "OpenDraw.h"
#include "Config.h"
#include "Main.h"
#include "Wingdi.h"

GDIRenderer::GDIRenderer(OpenDraw* ddraw)
	: Renderer(ddraw)
{
	this->hDc = NULL;
	this->isTrue = config.mode->bpp != 16 || config.bpp32Hooked;
}

BOOL GDIRenderer::Start()
{
	if (!Renderer::Start())
		return FALSE;

	this->hDc = GetDC(this->ddraw->hDraw);
	SetStretchBltMode(this->hDc, COLORONCOLOR);
	{
		this->hDcFront = CreateCompatibleDC(NULL);
		SetStretchBltMode(this->hDcFront, COLORONCOLOR);

		HDC hDcMain = GetDC(NULL);
		{
			this->hBmpFront = CreateCompatibleBitmap(hDcMain, config.mode->width, config.mode->height);
			SelectObject(this->hDcFront, this->hBmpFront);
				
			BITMAPINFO bmi = {};
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = *(LONG*)&config.mode->width;
			bmi.bmiHeader.biHeight = -*(LONG*)&config.mode->height;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biXPelsPerMeter = 1;
			bmi.bmiHeader.biYPelsPerMeter = 1;

			if (!this->isTrue)
			{
				this->hDcTemp = CreateCompatibleDC(hDcMain);
				this->hBmpTemp = CreateDIBSection(hDcMain, &bmi, DIB_RGB_COLORS, (VOID**)&this->tempData, NULL, 0);
				SelectObject(this->hDcTemp, this->hBmpTemp);
			}

			if (config.background.allowed)
			{
				this->hDcBack = CreateCompatibleDC(hDcMain);

				DWORD length = config.mode->width * config.mode->height * sizeof(DWORD);
				VOID* tempBuffer = MemoryAlloc(length);
				{
					MemoryZero(tempBuffer, length);
					Main::LoadBack(tempBuffer, config.mode->width, config.mode->height);

					this->hBmpBack = CreateDIBitmap(hDcMain, &bmi.bmiHeader, CBM_INIT, tempBuffer, &bmi, DIB_RGB_COLORS);
					SelectObject(this->hDcBack, this->hBmpBack);
				}
				MemoryFree(tempBuffer);
			}
		}
		ReleaseDC(NULL, hDcMain);
	}

	config.zoom.glallow = TRUE;
	PostMessage(this->ddraw->hWnd, config.msgMenu, NULL, NULL);

	return TRUE;
}

BOOL GDIRenderer::Stop()
{
	if (!Renderer::Stop())
		return FALSE;

	if (this->hDc)
	{
		ReleaseDC(this->ddraw->hDraw, this->hDc);

		if (this->hDcTemp)
		{
			if (this->hBmpTemp)
				DeleteObject(this->hBmpTemp);
			DeleteDC(this->hDcTemp);
		}

		if (this->hDcBack)
		{
			if (this->hBmpBack)
				DeleteObject(this->hBmpBack);
			DeleteDC(this->hDcBack);
		}

		if (this->hDcFront)
		{
			if (this->hBmpFront)
				DeleteObject(this->hBmpFront);
			DeleteDC(this->hDcFront);
		}
	}

	return TRUE;
}

VOID GDIRenderer::Clear()
{
	RECT rc = { 0, 0, this->ddraw->viewport.width, this->ddraw->viewport.height };
	FillRect(this->hDc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
}

VOID GDIRenderer::RenderFrame(BOOL ready, BOOL force, StateBufferAligned** lpStateBuffer)
{
	StateBufferAligned* stateBuffer = *lpStateBuffer;
	Size* frameSize = &stateBuffer->size;

	HDC hDcDraw;
	if (this->isTrue)
		hDcDraw = stateBuffer->hDc;
	else
	{
		hDcDraw = this->hDcTemp;

		WORD* src = (WORD*)stateBuffer->data;
		DWORD* dst = (DWORD*)this->tempData;
		DWORD count = config.mode->width * config.mode->height;
		do
		{
			WORD px = *src++;
			*dst++ = ((px & 0xF800) << 8) | ((px & 0x07E0) << 5) | ((px & 0x001F) << 3) | ALPHA_COMPONENT;
		} while (--count);
	}

	this->CheckView(FALSE);
	Rect* rect = &this->ddraw->viewport.rectangle;

	if (stateBuffer->isBack)
	{
		BitBlt(this->hDcFront, 0, 0, config.mode->width, config.mode->height, this->hDcBack, 0, 0, SRCCOPY);

		const BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
		AlphaBlend(this->hDcFront, 0, 0, config.mode->width, config.mode->height, hDcDraw, (config.mode->width - stateBuffer->size.width) >> 1, (config.mode->height - stateBuffer->size.height) >> 1, stateBuffer->size.width, stateBuffer->size.height, blend);

		if (rect->width != config.mode->width || rect->height != config.mode->height)
			StretchBlt(this->hDc, rect->x, rect->y, rect->width, rect->height, this->hDcFront, 0, 0, config.mode->width, config.mode->height, SRCCOPY);
		else
			BitBlt(this->hDc, rect->x, rect->y, rect->width, rect->height, this->hDcFront, 0, 0, SRCCOPY);
	}
	else
	{
		if (frameSize->width != rect->width || frameSize->height != rect->height)
			StretchBlt(this->hDc, rect->x, rect->y, rect->width, rect->height, hDcDraw, (config.mode->width - stateBuffer->size.width) >> 1, (config.mode->height - stateBuffer->size.height) >> 1, stateBuffer->size.width, stateBuffer->size.height, SRCCOPY);
		else
			BitBlt(this->hDc, rect->x, rect->y, rect->width, rect->height, hDcDraw, 0, 0, SRCCOPY);
	}

	if (this->ddraw->isTakeSnapshot)
		this->ddraw->TakeSnapshot(frameSize, stateBuffer->data, config.mode->bpp != 16 || config.bpp32Hooked);
}

VOID GDIRenderer::Redraw()
{
	this->Render();
}

BOOL GDIRenderer::ReadFrame(BYTE* dstData, Rect* rect, DWORD pitch, BOOL isBGR, BOOL* rev)
{
	BOOL res = FALSE;

	HDC hDcTemp = CreateCompatibleDC(this->hDc);
	if (hDcTemp)
	{
		BITMAPINFO bmi = {};
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = rect->width;
		bmi.bmiHeader.biHeight = rect->height;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 24;
		bmi.bmiHeader.biXPelsPerMeter = 1;
		bmi.bmiHeader.biYPelsPerMeter = 1;

		VOID* data;
		HBITMAP hBmp = CreateDIBSection(hDcTemp, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, &data, NULL, 0);
		if (hBmp)
		{
			SelectObject(hDcTemp, hBmp);
			if (BitBlt(hDcTemp, 0, 0, rect->width, rect->height, this->hDc, rect->x, rect->y, SRCCOPY))
			{
				MemoryCopy(dstData, data, rect->height * pitch);
				*rev = !isBGR;
				res = TRUE;
			}
			DeleteObject(hBmp);
		}

		DeleteDC(hDcTemp);
	}

	return res;
}
