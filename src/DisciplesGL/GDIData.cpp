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

#include "stdafx.h"
#include "GDIData.h"
#include "Config.h"
#include "Main.h"

GDIData::GDIData()
{
	this->fpsCounter = new FpsCounter(config.mode->bpp != 16 || config.bpp32Hooked);

	if (!config.version && config.resHooked)
	{
		this->hDc = CreateCompatibleDC(NULL);
		this->hDcBack = CreateCompatibleDC(hDc);

		HDC hDc = GetDC(NULL);
		{
			this->hBmp = CreateCompatibleBitmap(hDc, config.mode->width, config.mode->height);
			SelectObject(this->hDc, this->hBmp);

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

				this->hBmpBack = CreateDIBitmap(hDc, &bmi.bmiHeader, CBM_INIT, tempBuffer, &bmi, DIB_RGB_COLORS);
				SelectObject(this->hDcBack, this->hBmpBack);
			}
			MemoryFree(tempBuffer);
		}
		ReleaseDC(NULL, hDc);
	}
	else
		this->hDc = NULL;
}

GDIData::~GDIData()
{
	if (this->hDc)
	{
		DeleteObject(this->hBmpBack);
		DeleteDC(this->hDcBack);

		DeleteObject(this->hBmp);
		DeleteDC(this->hDc);
	}

	delete this->fpsCounter;
}
