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
#include "Mmsystem.h"
#include "FpsCounter.h"
#include "Config.h"

FpsState fpsState;
BOOL isFpsChanged;

const WORD counters[10][FPS_HEIGHT] = {
	{ // 0
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC },
	{ // 1
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000 },
	{ // 2
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0x003C,
		0x003C,
		0x003C,
		0x003C,
		0x003C,
		0x003C,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC },
	{ // 3
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC },
	{ // 4
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000 },
	{ // 5
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0x003C,
		0x003C,
		0x003C,
		0x003C,
		0x003C,
		0x003C,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC },
	{ // 6
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0x003C,
		0x003C,
		0x003C,
		0x003C,
		0x003C,
		0x003C,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC },
	{ // 7
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000 },
	{ // 8
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC },
	{ // 9
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xF03C,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xF000,
		0xFFFC,
		0xFFFC,
		0xFFFC,
		0xFFFC }
};

FpsCounter::FpsCounter(BOOL isTrue)
{
	this->isTrue = isTrue;
	this->count = FPS_ACCURACY * 10;
	this->tickQueue = (FrameItem*)MemoryAlloc(this->count * sizeof(FrameItem));
	this->Reset();
}

FpsCounter::~FpsCounter()
{
	MemoryFree(this->tickQueue);
}

VOID FpsCounter::Reset()
{
	this->checkIndex = 0;
	this->currentIndex = 0;
	this->summary = 0;
	this->lastTick = 0;
	this->value = 0;
	MemoryZero(this->tickQueue, this->count * sizeof(FrameItem));
}

VOID FpsCounter::Calculate()
{
	if (isFpsChanged)
	{
		isFpsChanged = FALSE;
		this->Reset();
	}

	FrameItem* tickItem = &tickQueue[this->currentIndex];
	tickItem->tick = timeGetTime();

	if (this->lastTick)
	{
		tickItem->span = tickItem->tick - this->lastTick;
		this->summary += tickItem->span;
	}
	this->lastTick = tickItem->tick;

	DWORD check = tickItem->tick - FPS_ACCURACY;

	DWORD total = 0;
	if (this->checkIndex > this->currentIndex)
	{
		FrameItem* checkPtr = &this->tickQueue[this->checkIndex];
		while (this->checkIndex < this->count)
		{
			if (checkPtr->tick > check)
			{
				total = this->count - this->checkIndex + this->currentIndex + 1;
				break;
			}

			this->summary -= checkPtr->span;

			++checkPtr;
			++this->checkIndex;
		}

		if (this->checkIndex == this->count)
			this->checkIndex = 0;
	}

	if (!total)
	{
		FrameItem* checkPtr = &this->tickQueue[this->checkIndex];
		while (this->checkIndex <= this->currentIndex)
		{
			if (checkPtr->tick > check)
			{
				total = this->currentIndex - this->checkIndex + 1;
				break;
			}

			this->summary -= checkPtr->span;

			++checkPtr;
			++this->checkIndex;
		}
	}

	if (this->currentIndex != this->count - 1)
		++this->currentIndex;
	else
		this->currentIndex = 0;

	this->value = this->summary ? 1000 * total / this->summary : 0;
}

VOID FpsCounter::Draw(StateBufferAligned* buffer)
{
	DWORD fps = this->value;
	DWORD digCount = 0;
	DWORD current = fps;
	do
	{
		++digCount;
		current = current / 10;
	} while (current);

	Size offset = { (config.mode->width - buffer->size.width) >> 1, (config.mode->height - buffer->size.height) >> 1 };
	DWORD slice = config.mode->width - FPS_WIDTH;
	if (this->isTrue)
	{
		DWORD fpsColor = fpsState == FpsBenchmark ? (config.renderer == RendererGDI ? 0xFFFFFF00 : 0xFF00FFFF) : 0xFFFFFFFF;
		DWORD dcount = digCount;
		do
		{
			WORD* lpDig = (WORD*)counters[fps % 10];
			DWORD* idx = (DWORD*)buffer->data + (offset.height + FPS_Y) * config.mode->width + offset.width + FPS_X + FPS_WIDTH * (dcount - 1);

			DWORD height = FPS_HEIGHT;
			do
			{
				WORD check = *lpDig++;
				DWORD width = FPS_WIDTH;
				do
				{
					if (check & 1)
						*idx = fpsColor;

					++idx;
					check >>= 1;
				} while (--width);

				idx += slice;
			} while (--height);

			fps = fps / 10;
		} while (--dcount);
	}
	else if (config.gl.version.value > GL_VER_1_1)
	{
		WORD fpsColor = fpsState == FpsBenchmark ? 0xFFE0 : 0xFFFF;
		DWORD dcount = digCount;
		do
		{
			WORD* lpDig = (WORD*)counters[fps % 10];
			WORD* idx = (WORD*)buffer->data + (offset.height + FPS_Y) * config.mode->width + offset.width + FPS_X + FPS_WIDTH * (dcount - 1);

			DWORD height = FPS_HEIGHT;
			do
			{
				WORD check = *lpDig++;
				DWORD width = FPS_WIDTH;
				do
				{
					if (check & 1)
						*idx = fpsColor;

					++idx;
					check >>= 1;
				} while (--width);

				idx += slice;
			} while (--height);

			fps = fps / 10;
		} while (--dcount);
	}
}