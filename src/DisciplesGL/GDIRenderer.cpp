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
}

GDIRenderer::~GDIRenderer()
{
	if (this->Stop())
	{
		if (this->hDc)
		{
			ReleaseDC(this->ddraw->hDraw, this->hDc);

			if (this->hDcFront)
			{
				DeleteObject(this->hBmpBack);
				DeleteDC(this->hDcBack);

				DeleteObject(this->hBmpFront);
				DeleteDC(this->hDcFront);
			}
		}

		delete this->fpsCounter;
	}
}

BOOL GDIRenderer::Start()
{
	if (!Renderer::Start())
		return FALSE;

	this->fpsCounter = new FpsCounter(config.mode->bpp != 16 || config.bpp32Hooked);

	if (!config.version && config.resHooked)
	{
		this->hDc = GetDC(this->ddraw->hDraw);
		SetStretchBltMode(this->hDc, COLORONCOLOR);
		{
			this->hDcFront = CreateCompatibleDC(NULL);
			SetStretchBltMode(this->hDcFront, COLORONCOLOR);

			this->hDcBack = CreateCompatibleDC(this->hDcFront);

			HDC hDcTemp = GetDC(NULL);
			{
				this->hBmpFront = CreateCompatibleBitmap(hDcTemp, config.mode->width, config.mode->height);
				SelectObject(this->hDcFront, this->hBmpFront);

				DWORD length = config.mode->width * config.mode->height * sizeof(DWORD);
				VOID* tempBuffer = MemoryAlloc(length);
				{
					MemoryZero(tempBuffer, length);
					Main::LoadBack(tempBuffer, config.mode->width, config.mode->height);

					BITMAPINFO bmi;
					MemoryZero(&bmi, sizeof(BITMAPINFO));
					bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
					bmi.bmiHeader.biWidth = *(LONG*)&config.mode->width;
					bmi.bmiHeader.biHeight = -*(LONG*)&config.mode->height;
					bmi.bmiHeader.biPlanes = 1;
					bmi.bmiHeader.biBitCount = 32;
					bmi.bmiHeader.biXPelsPerMeter = 1;
					bmi.bmiHeader.biYPelsPerMeter = 1;

					this->hBmpBack = CreateDIBitmap(hDcTemp, &bmi.bmiHeader, CBM_INIT, tempBuffer, &bmi, DIB_RGB_COLORS);
					SelectObject(this->hDcBack, this->hBmpBack);
				}
				MemoryFree(tempBuffer);
			}
			ReleaseDC(NULL, hDcTemp);
		}
	}
	else
		this->hDcFront = NULL;

	config.zoom.glallow = TRUE;
	PostMessage(this->ddraw->hWnd, config.msgMenu, NULL, NULL);

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

	this->CheckView(FALSE);

	if (fpsState)
	{
		this->fpsCounter->Draw(stateBuffer);
		this->fpsCounter->Calculate();
	}

	Rect* rect = &this->ddraw->viewport.rectangle;
	if (stateBuffer->isBack)
	{
		BitBlt(this->hDcFront, 0, 0, config.mode->width, config.mode->height, this->hDcBack, 0, 0, SRCCOPY);

		const BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
		AlphaBlend(this->hDcFront, 0, 0, config.mode->width, config.mode->height, stateBuffer->hDc, (config.mode->width - stateBuffer->size.width) >> 1, (config.mode->height - stateBuffer->size.height) >> 1, stateBuffer->size.width, stateBuffer->size.height, blend);

		if (rect->width != config.mode->width || rect->height != config.mode->height)
			StretchBlt(this->hDc, rect->x, rect->y, rect->width, rect->height, this->hDcFront, 0, 0, config.mode->width, config.mode->height, SRCCOPY);
		else
			BitBlt(this->hDc, rect->x, rect->y, rect->width, rect->height, this->hDcFront, 0, 0, SRCCOPY);
	}
	else
	{
		if (frameSize->width != rect->width || frameSize->height != rect->height)
			StretchBlt(this->hDc, rect->x, rect->y, rect->width, rect->height, stateBuffer->hDc, (config.mode->width - stateBuffer->size.width) >> 1, (config.mode->height - stateBuffer->size.height) >> 1, stateBuffer->size.width, stateBuffer->size.height, SRCCOPY);
		else
			BitBlt(this->hDc, rect->x, rect->y, rect->width, rect->height, stateBuffer->hDc, 0, 0, SRCCOPY);
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
		BITMAPINFO bmi;
		MemoryZero(&bmi, sizeof(BITMAPINFO));
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
