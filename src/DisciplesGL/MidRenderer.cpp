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
#include "MidRenderer.h"
#include "OpenDraw.h"
#include "Config.h"
#include "Main.h"
#include "Resource.h"

MidRenderer::MidRenderer(OpenDraw* ddraw)
	: OGLRenderer(ddraw)
{
}

VOID MidRenderer::Begin()
{
	config.zoom.glallow = TRUE;
	PostMessage(this->ddraw->hWnd, config.msgMenu, NULL, NULL);

	this->isTrueColor = config.mode->bpp != 16 || config.bpp32Hooked;
	this->maxTexSize = GetPow2(config.mode->width > config.mode->height ? config.mode->width : config.mode->height);

	FLOAT texWidth = config.mode->width == this->maxTexSize ? 1.0f : (FLOAT)config.mode->width / this->maxTexSize;
	FLOAT texHeight = config.mode->height == this->maxTexSize ? 1.0f : (FLOAT)config.mode->height / this->maxTexSize;

	this->texSize = (this->maxTexSize & 0xFFFF) | (this->maxTexSize << 16);

	this->shaders = {
		new ShaderGroup(GLSL_VER_1_10, IDR_LINEAR_VERTEX, IDR_LINEAR_FRAGMENT, SHADER_DOUBLE | SHADER_LEVELS),
		new ShaderGroup(GLSL_VER_1_10, IDR_HERMITE_VERTEX, IDR_HERMITE_FRAGMENT, SHADER_DOUBLE | SHADER_LEVELS),
		new ShaderGroup(GLSL_VER_1_10, IDR_CUBIC_VERTEX, IDR_CUBIC_FRAGMENT, SHADER_DOUBLE | SHADER_LEVELS),
		new ShaderGroup(GLSL_VER_1_10, IDR_LANCZOS_VERTEX, IDR_LANCZOS_FRAGMENT, SHADER_DOUBLE | SHADER_LEVELS)
	};

	{
		GLGenBuffers(1, &this->bufferName);
		{
			GLBindBuffer(GL_ARRAY_BUFFER, this->bufferName);
			{
				FLOAT bf[4][8] = {
					{ 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
					{ (FLOAT)config.mode->width, 0.0f, 0.0f, 1.0f, texWidth, 0.0f, texWidth, 0.0f },
					{ (FLOAT)config.mode->width, (FLOAT)config.mode->height, 0.0f, 1.0f, texWidth, texHeight, texWidth, texHeight },
					{ 0.0f, (FLOAT)config.mode->height, 0.0f, 1.0f, 0.0f, texHeight, 0.0f, texHeight }
				};
				MemoryCopy(this->buffer, bf, sizeof(this->buffer));

				{
					FLOAT mvp[4][4] = {
						{ FLOAT(2.0f / config.mode->width), 0.0f, 0.0f, 0.0f },
						{ 0.0f, FLOAT(-2.0f / config.mode->height), 0.0f, 0.0f },
						{ 0.0f, 0.0f, 2.0f, 0.0f },
						{ -1.0f, 1.0f, -1.0f, 1.0f }
					};

					for (DWORD i = 0; i < 4; ++i)
					{
						FLOAT* vector = &this->buffer[i][0];
						for (DWORD j = 0; j < 4; ++j)
						{
							FLOAT sum = 0.0f;
							for (DWORD v = 0; v < 4; ++v)
								sum += mvp[v][j] * vector[v];

							vector[j] = sum;
						}
					}

					GLBufferData(GL_ARRAY_BUFFER, sizeof(this->buffer) << 1, NULL, GL_STATIC_DRAW);
					GLBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(this->buffer), this->buffer);
					GLBufferSubData(GL_ARRAY_BUFFER, sizeof(this->buffer), sizeof(this->buffer), this->buffer);
				}

				{
					GLEnableVertexAttribArray(0);
					GLVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 32, (GLvoid*)0);

					GLEnableVertexAttribArray(1);
					GLVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 32, (GLvoid*)16);
				}

				if (config.background.allowed)
					GLGenTextures(2, &this->textureId.back);
				else
					GLGenTextures(1, &this->textureId.primary);
				{
					if (config.background.allowed)
					{
						GLActiveTexture(GL_TEXTURE1);
						GLBindTexParameter(this->textureId.back, GL_LINEAR);

						DWORD length = config.mode->width * config.mode->height * sizeof(DWORD);
						VOID* tempBuffer = MemoryAlloc(length);
						{
							MemoryZero(tempBuffer, length);
							Main::LoadBack(tempBuffer, config.mode->width, config.mode->height);
							DWORD dds = *(DWORD*)tempBuffer;
							GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->maxTexSize, this->maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
							GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, config.mode->width, config.mode->height, GL_RGBA, GL_UNSIGNED_BYTE, tempBuffer);
						}
						MemoryFree(tempBuffer);
					}

					{
						GLActiveTexture(GL_TEXTURE0);
						GLBindTexParameter(this->textureId.primary, GL_LINEAR);

						if (this->isTrueColor)
							GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->maxTexSize, this->maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
						else
							GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->maxTexSize, this->maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
					}

					this->pixelBuffer = new PixelBuffer(this->isTrueColor);
					{
						GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);

						this->program = NULL;
						this->borderStatus = FALSE;
						this->backStatus = FALSE;
						this->zoomSize = { 0, 0 };
						this->isVSync = config.image.vSync;

						if (WGLSwapInterval)
							WGLSwapInterval(this->isVSync);
					}
				}
			}
		}
	}
}

VOID MidRenderer::End()
{
	{
		{
			{
				{
					delete this->pixelBuffer;
				}

				if (config.background.allowed)
					GLDeleteTextures(2, &this->textureId.back);
				else
					GLDeleteTextures(1, &this->textureId.primary);
			}
			GLBindBuffer(GL_ARRAY_BUFFER, NULL);
		}
		GLDeleteBuffers(1, &this->bufferName);
	}
	GLUseProgram(NULL);

	ShaderGroup** shader = (ShaderGroup**)&this->shaders;
	DWORD count = sizeof(this->shaders) / sizeof(ShaderGroup*);
	do
		delete *shader++;
	while (--count);
}

BOOL MidRenderer::RenderInner(BOOL ready, BOOL force, StateBufferAligned** lpStateBuffer)
{
	StateBufferAligned* stateBuffer = *lpStateBuffer;
	Size frameSize = stateBuffer->size;
	FilterState state = this->ddraw->filterState;
	if (pixelBuffer->Update(lpStateBuffer, ready) || state.flags || this->borderStatus != stateBuffer->borders || backStatus != stateBuffer->isBack)
	{
		if (this->isVSync != config.image.vSync)
		{
			this->isVSync = config.image.vSync;
			if (WGLSwapInterval)
				WGLSwapInterval(this->isVSync);
		}

		if (this->CheckView(TRUE))
			GLViewport(this->ddraw->viewport.rectangle.x, this->ddraw->viewport.rectangle.y + this->ddraw->viewport.offset, this->ddraw->viewport.rectangle.width, this->ddraw->viewport.rectangle.height);

		if (state.interpolation > InterpolateLinear && frameSize.width == this->ddraw->viewport.rectangle.width && frameSize.height == this->ddraw->viewport.rectangle.height)
		{
			state.interpolation = InterpolateLinear;
			if (this->program != this->shaders.linear)
				state.flags = TRUE;
		}

		if (force || state.flags || this->borderStatus != stateBuffer->borders || this->backStatus != stateBuffer->isBack)
		{
			switch (state.interpolation)
			{
			case InterpolateHermite:
				this->program = this->shaders.hermite;
				break;
			case InterpolateCubic:
				this->program = this->shaders.cubic;
				break;
			case InterpolateLanczos:
				this->program = this->shaders.lanczos;
				break;
			default:
				this->program = this->shaders.linear;
				break;
			}
			this->program->Use(this->texSize, stateBuffer->isBack);

			if (state.flags || this->backStatus != stateBuffer->isBack)
			{
				this->ddraw->filterState.flags = FALSE;

				if (stateBuffer->isBack)
				{
					GLActiveTexture(GL_TEXTURE1);
					GLBindTexFilter(this->textureId.back, state.interpolation != InterpolateNearest ? GL_LINEAR : GL_NEAREST);
				}

				GLActiveTexture(GL_TEXTURE0);
				GLBindTexFilter(this->textureId.primary, state.interpolation == InterpolateLinear || state.interpolation == InterpolateHermite ? GL_LINEAR : GL_NEAREST);
			}

			this->borderStatus = stateBuffer->borders;
			this->backStatus = stateBuffer->isBack;
		}

		if (stateBuffer->isZoomed)
		{
			if (this->zoomSize.width != frameSize.width || this->zoomSize.height != frameSize.height)
			{
				this->zoomSize = frameSize;

				FLOAT tw = (FLOAT)frameSize.width / this->maxTexSize;
				FLOAT th = (FLOAT)frameSize.height / this->maxTexSize;

				this->buffer[1][4] = tw;
				this->buffer[2][4] = tw;
				this->buffer[2][5] = th;
				this->buffer[3][5] = th;

				GLBufferSubData(GL_ARRAY_BUFFER, 5 * 8 * sizeof(FLOAT), 3 * 8 * sizeof(FLOAT), &this->buffer[1]);
			}

			GLDrawArrays(GL_TRIANGLE_FAN, 4, 4);
		}
		else
			GLDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		return TRUE;
	}

	return FALSE;
}
