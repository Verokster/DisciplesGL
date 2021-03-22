/*
	MIT License

	Copyright (c) 2021 Oleksiy Ryabchun

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
#include "TextRenderer.h"
#include "Config.h"

TextRenderer* txt;

TextRenderer::TextRenderer(HFONT hFont)
{
	this->hDc = NULL;
	this->hBmp = NULL;
	this->hFont = hFont;
	this->width = 0;
	this->height = 0;
	this->bmBuffer = NULL;
}

TextRenderer::~TextRenderer()
{
	if (this->hBmp)
		DeleteObject(this->hBmp);

	if (this->hDc)
		DeleteDC(this->hDc);
}

VOID TextRenderer::Draw(const CHAR* text, COLORREF color, COLORREF chroma, DWORD align, const RECT* padding, const FrameType* frame)
{
	if (!this->hFont)
		return;

	if (!this->hDc)
	{
		this->hDc = CreateCompatibleDC(NULL);
		if (this->hDc)
		{
			SetBkMode(hDc, OPAQUE);
			SelectObject(hDc, this->hFont); // select font onto DC
		}
	}

	if (!this->hDc)
		return;

	if (!this->hBmp || this->width != frame->width || this->height != frame->height)
	{
		if (this->hBmp)
		{
			DeleteObject(this->hBmp);
			this->hBmp = NULL;
		}

		this->width = frame->width;
		this->height = frame->height;

		// define and create temp bitmap
		BITMAPINFO bmi = {};
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = *(LONG*)&this->width;
		bmi.bmiHeader.biHeight = -*(LONG*)&this->height;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biXPelsPerMeter = 1;
		bmi.bmiHeader.biYPelsPerMeter = 1;

		this->hBmp = CreateDIBSection(hDc, &bmi, DIB_RGB_COLORS, &this->bmBuffer, NULL, 0);
		if (this->hBmp)
			SelectObject(hDc, hBmp);
	}

	if (this->hBmp)
	{
		// calculate text bounds
		RECT rc = { 0, 0, *(LONG*)&this->width, *(LONG*)&this->height };
		if (padding)
		{
			rc.left += padding->left;
			rc.top += padding->top;
			rc.right -= padding->right;
			rc.bottom -= padding->bottom;
		}

		RECT old = rc;
		DrawText(hDc, text, -1, &rc, DT_WORDBREAK | DT_CALCRECT);

		POINT off = {
			align & DT_CENTER ? (((old.right - old.left) - (rc.right - rc.left)) / 2) : (align & DT_RIGHT ? old.right - rc.right : 0),
			align & DT_VCENTER ? (((old.bottom - old.top) - (rc.bottom - rc.top)) / 2) : (align & DT_BOTTOM ? old.bottom - rc.bottom : 0)
		};

		OffsetRect(&rc, off.x, off.y);

		LONG bmWidth = rc.right - rc.left;
		LONG bmHeight = rc.bottom - rc.top;
		LONG bmPitch = this->width * 4;

		SetBkColor(hDc, chroma);
		SetTextColor(hDc, color);

		// draw text onto bitmap
		DrawText(hDc, text, -1, &rc, align & 0xF | DT_WORDBREAK);

		// draw back to frame
		BYTE* srcData = (BYTE*)this->bmBuffer + rc.top * bmPitch + rc.left * sizeof(DWORD);
		if (frame->pixelFormat & PIXEL_TRUECOLOR)
		{
			BYTE* dstData = (BYTE*)frame->buffer + rc.top * frame->pitch + rc.left * sizeof(DWORD);
			for (LONG j = 0; j < bmHeight; ++j, srcData += bmPitch, dstData += frame->pitch)
			{
				DWORD* src = (DWORD*)srcData;
				DWORD* dst = (DWORD*)dstData;

				for (LONG i = 0; i < bmWidth; ++i)
				{
					DWORD pix = src[i];
					if ((pix & 0x00FFFFFF) != *(DWORD*)&chroma) // check if remove black
						dst[i] = pix | 0xFF000000;
				}
			}
		}
		else
		{
			BYTE* dstData = (BYTE*)frame->buffer + rc.top * frame->pitch + rc.left * sizeof(WORD);
			for (LONG j = 0; j < bmHeight; ++j, srcData += bmPitch, dstData += frame->pitch)
			{
				DWORD* src = (DWORD*)srcData;
				WORD* dst = (WORD*)dstData;

				for (LONG i = 0; i < bmWidth; ++i)
				{
					DWORD pix = src[i];
					if ((pix & 0x00FFFFFF) != *(DWORD*)&chroma) // check if remove black
						dst[i] = LOWORD(((pix & 0x0000F8) >> 3) | ((pix & 0x00FC00) >> 5) | ((pix & 0xF80000) >> 8));
				}
			}
		}
	}
}
