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
#include "OldRenderer.h"
#include "OpenDraw.h"
#include "Config.h"
#include "Main.h"
#include "Resource.h"

OldRenderer::OldRenderer(OpenDraw* ddraw)
	: OGLRenderer(ddraw)
{
}

VOID OldRenderer::Begin()
{
	this->isTrueColor = config.mode->bpp != 16 || config.bpp32Hooked;

	if (this->ddraw->filterState.interpolation > InterpolateLinear)
		this->ddraw->filterState.interpolation = InterpolateLinear;

	DWORD glMaxTexSize;
	GLGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&glMaxTexSize);
	if (glMaxTexSize < 256)
		glMaxTexSize = 256;

	DWORD maxAllow = GetPow2(config.mode->width > config.mode->height ? config.mode->width : config.mode->height);
	this->maxTexSize = maxAllow < glMaxTexSize ? maxAllow : glMaxTexSize;

	DWORD framePerWidth = config.mode->width / this->maxTexSize + (config.mode->width % this->maxTexSize ? 1 : 0);
	DWORD framePerHeight = config.mode->height / this->maxTexSize + (config.mode->height % this->maxTexSize ? 1 : 0);
	this->frameCount = framePerWidth * framePerHeight;

	config.zoom.glallow = this->frameCount == 1;
	PostMessage(this->ddraw->hWnd, config.msgMenu, NULL, NULL);

	this->frames = (Frame*)MemoryAlloc(this->frameCount * sizeof(Frame));
	{
		VOID* tempBuffer = NULL;
		if (this->allowBack)
		{
			DWORD length = config.mode->width * config.mode->height * sizeof(DWORD);
			tempBuffer = MemoryAlloc(length);
			MemoryZero(tempBuffer, length);
			Main::LoadBack(tempBuffer, config.mode->width, config.mode->height);
		}

		{
			Frame* frame = this->frames;
			for (DWORD y = 0; y < config.mode->height; y += this->maxTexSize)
			{
				DWORD height = config.mode->height - y;
				if (height > this->maxTexSize)
					height = this->maxTexSize;

				for (DWORD x = 0; x < config.mode->width; x += this->maxTexSize, ++frame)
				{
					DWORD width = config.mode->width - x;
					if (width > this->maxTexSize)
						width = this->maxTexSize;

					frame->point.x = x;
					frame->point.y = y;

					frame->rect.x = x;
					frame->rect.y = y;
					frame->rect.width = width;
					frame->rect.height = height;

					frame->vSize.width = x + width;
					frame->vSize.height = y + height;

					frame->tSize.width = width == this->maxTexSize ? 1.0f : (FLOAT)width / this->maxTexSize;
					frame->tSize.height = height == this->maxTexSize ? 1.0f : (FLOAT)height / this->maxTexSize;

					GLGenTextures(2, frame->id);

					if (this->allowBack)
					{
						GLBindTexture(GL_TEXTURE_2D, frame->id[1]);

						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, config.gl.caps.clampToEdge);
						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, config.gl.caps.clampToEdge);
						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

						GLTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

						GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->maxTexSize, this->maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
						GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->rect.width, frame->rect.height, GL_RGBA, GL_UNSIGNED_BYTE, (DWORD*)tempBuffer + frame->rect.y * config.mode->width + frame->rect.x);
					}

					GLBindTexture(GL_TEXTURE_2D, frame->id[0]);

					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, config.gl.caps.clampToEdge);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, config.gl.caps.clampToEdge);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

					GLTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

					if (this->isTrueColor || config.gl.version.value <= GL_VER_1_1)
						GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->maxTexSize, this->maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
					else
						GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->maxTexSize, this->maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
				}
			}
		}

		if (tempBuffer)
			MemoryFree(tempBuffer);

		this->pixelBuffer = new PixelBuffer(this->isTrueColor);
		{
			this->convert = !this->isTrueColor && config.gl.version.value <= GL_VER_1_1;
			this->frameBuffer = this->convert ? AlignedAlloc(this->maxTexSize * this->maxTexSize * sizeof(DWORD)) : NULL;
			{
				GLMatrixMode(GL_PROJECTION);
				GLLoadIdentity();
				GLOrtho(0.0, (GLdouble)config.mode->width, (GLdouble)config.mode->height, 0.0, 0.0, 1.0);
				GLMatrixMode(GL_MODELVIEW);
				GLLoadIdentity();

				GLBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				GLEnable(GL_TEXTURE_2D);

				GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);

				this->borderStatus = FALSE;
				this->backStatus = FALSE;
				this->isVSync = config.image.vSync;
				if (WGLSwapInterval)
					WGLSwapInterval(isVSync);
			}
		}
	}
}

VOID OldRenderer::End()
{
	{
		{
			if (this->frameBuffer)
				AlignedFree(this->frameBuffer);
		}
		delete this->pixelBuffer;

		Frame* frame = this->frames;
		DWORD count = this->frameCount;
		while (count)
		{
			--count;
			GLDeleteTextures(2, frame->id);
			++frame;
		}
	}
	MemoryFree(this->frames);
}

BOOL OldRenderer::RenderInner(BOOL ready, BOOL force, StateBufferAligned** lpStateBuffer)
{
	StateBufferAligned* stateBuffer = *lpStateBuffer;
	Size* frameSize = &stateBuffer->size;
	FilterState state = this->ddraw->filterState;
	if (this->pixelBuffer->Update(lpStateBuffer, ready, this->frameCount != 1 || this->convert) || state.flags || this->borderStatus != stateBuffer->borders || this->backStatus != stateBuffer->isBack)
	{
		if (this->isVSync != config.image.vSync)
		{
			this->isVSync = config.image.vSync;
			if (WGLSwapInterval)
				WGLSwapInterval(this->isVSync);
		}

		if (this->CheckView(TRUE))
			GLViewport(this->ddraw->viewport.rectangle.x, this->ddraw->viewport.rectangle.y + this->ddraw->viewport.offset, this->ddraw->viewport.rectangle.width, this->ddraw->viewport.rectangle.height);

		DWORD glFilter = 0;
		if (state.flags)
		{
			this->ddraw->filterState.flags = FALSE;
			glFilter = state.interpolation == InterpolateNearest ? GL_NEAREST : GL_LINEAR;
		}

		GLDisable(GL_BLEND);

		this->borderStatus = stateBuffer->borders;
		this->backStatus = stateBuffer->isBack;
		if (stateBuffer->isBack && this->allowBack)
		{
			DWORD count = this->frameCount;
			Frame* frame = this->frames;
			while (count)
			{
				--count;
				GLBindTexture(GL_TEXTURE_2D, frame->id[1]);

				if (glFilter)
				{
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glFilter);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glFilter);
				}

				GLBegin(GL_TRIANGLE_FAN);
				{
					GLTexCoord2f(0.0f, 0.0f);
					GLVertex2s((SHORT)frame->point.x, (SHORT)frame->point.y);

					GLTexCoord2f(frame->tSize.width, 0.0f);
					GLVertex2s((SHORT)frame->vSize.width, (SHORT)frame->point.y);

					GLTexCoord2f(frame->tSize.width, frame->tSize.height);
					GLVertex2s((SHORT)frame->vSize.width, (SHORT)frame->vSize.height);

					GLTexCoord2f(0.0f, frame->tSize.height);
					GLVertex2s((SHORT)frame->point.x, (SHORT)frame->vSize.height);
				}
				GLEnd();
				++frame;
			}
			GLEnable(GL_BLEND);
		}

		{
			DWORD count = this->frameCount;
			Frame* frame = this->frames;
			while (count)
			{
				--count;
				GLBindTexture(GL_TEXTURE_2D, frame->id[0]);

				if (glFilter)
				{
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glFilter);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glFilter);
				}

				if (this->frameCount == 1)
				{
					if (this->convert)
					{
						WORD* src = (WORD*)stateBuffer->data + ((config.mode->height - stateBuffer->size.height) >> 1) * config.mode->width + ((config.mode->width - stateBuffer->size.width) >> 1);
						DWORD* dst = (DWORD*)this->frameBuffer;

						DWORD slice = config.mode->width - frameSize->width;

						DWORD copyHeight = frameSize->height;
						do
						{
							DWORD copyWidth = frameSize->width;
							do
							{
								WORD px = *src++;
								*dst++ = ((px & 0xF800) >> 8) | ((px & 0x07E0) << 5) | ((px & 0x001F) << 19);
							} while (--copyWidth);

							src += slice;
							dst += slice;
						} while (--copyHeight);

						GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameSize->width, frameSize->height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
					}
				}
				else
				{
					if (this->isTrueColor)
						GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->rect.width, frame->rect.height, GL_RGBA, GL_UNSIGNED_BYTE, (DWORD*)stateBuffer->data + frame->rect.y * frameSize->width + frame->rect.x);
					else if (config.gl.version.value > GL_VER_1_1)
						GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->rect.width, frame->rect.height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (WORD*)stateBuffer->data + frame->rect.y * frameSize->width + frame->rect.x);
					else
					{
						WORD* src = (WORD*)stateBuffer->data + frame->rect.y * frameSize->width + frame->rect.x;
						DWORD* dst = (DWORD*)this->frameBuffer;

						DWORD slice = config.mode->width - frame->rect.width;

						DWORD copyHeight = frame->rect.height;
						do
						{
							DWORD copyWidth = frame->rect.width;
							do
							{
								WORD px = *src++;
								*dst++ = ((px & 0xF800) >> 8) | ((px & 0x07E0) << 5) | ((px & 0x001F) << 19);
							} while (--copyWidth);

							src += slice;
							dst += slice;
						} while (--copyHeight);

						GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->rect.width, frame->rect.height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
					}
				}

				GLBegin(GL_TRIANGLE_FAN);
				{
					if (!stateBuffer->isZoomed)
					{
						GLTexCoord2f(0.0f, 0.0f);
						GLVertex2s((SHORT)frame->point.x, (SHORT)frame->point.y);

						GLTexCoord2f(frame->tSize.width, 0.0f);
						GLVertex2s((SHORT)frame->vSize.width, (SHORT)frame->point.y);

						GLTexCoord2f(frame->tSize.width, frame->tSize.height);
						GLVertex2s((SHORT)frame->vSize.width, (SHORT)frame->vSize.height);

						GLTexCoord2f(0.0f, frame->tSize.height);
						GLVertex2s((SHORT)frame->point.x, (SHORT)frame->vSize.height);
					}
					else
					{
						FLOAT tw = (FLOAT)stateBuffer->size.width / this->maxTexSize;
						FLOAT th = (FLOAT)stateBuffer->size.height / this->maxTexSize;

						GLTexCoord2f(0.0f, 0.0f);
						GLVertex2s((SHORT)frame->point.x, (SHORT)frame->point.y);

						GLTexCoord2f(tw, 0.0f);
						GLVertex2s((SHORT)frame->vSize.width, (SHORT)frame->point.y);

						GLTexCoord2f(tw, th);
						GLVertex2s((SHORT)frame->vSize.width, (SHORT)frame->vSize.height);

						GLTexCoord2f(0.0f, th);
						GLVertex2s((SHORT)frame->point.x, (SHORT)frame->vSize.height);
					}
				}
				GLEnd();
				++frame;
			}
		}

		return TRUE;
	}

	return FALSE;
}