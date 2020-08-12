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
#include "NewRenderer.h"
#include "OpenDraw.h"
#include "Config.h"
#include "Main.h"
#include "Resource.h"

NewRenderer::NewRenderer(OpenDraw* ddraw)
	: OGLRenderer(ddraw)
{
}

VOID NewRenderer::Begin()
{
	config.zoom.glallow = TRUE;
	PostMessage(this->ddraw->hWnd, config.msgMenu, NULL, NULL);

	this->isTrueColor = config.mode->bpp != 16 || config.bpp32Hooked;
	this->maxTexSize = GetPow2(config.mode->width > config.mode->height ? config.mode->width : config.mode->height);

	FLOAT texWidth = config.mode->width == this->maxTexSize ? 1.0f : (FLOAT)config.mode->width / this->maxTexSize;
	FLOAT texHeight = config.mode->height == this->maxTexSize ? 1.0f : (FLOAT)config.mode->height / this->maxTexSize;

	this->texSize = (this->maxTexSize & 0xFFFF) | (this->maxTexSize << 16);

	DWORD flags = config.bpp32Hooked ? SHADER_ALPHA : NULL;
	this->shaders = {
		new ShaderGroup(GLSL_VER_1_30, IDR_LINEAR_VERTEX, IDR_LINEAR_FRAGMENT, SHADER_DOUBLE | SHADER_LEVELS),
		new ShaderGroup(GLSL_VER_1_30, IDR_HERMITE_VERTEX, IDR_HERMITE_FRAGMENT, SHADER_DOUBLE | SHADER_LEVELS),
		new ShaderGroup(GLSL_VER_1_30, IDR_CUBIC_VERTEX, IDR_CUBIC_FRAGMENT, SHADER_DOUBLE | SHADER_LEVELS),
		new ShaderGroup(GLSL_VER_1_30, IDR_LANCZOS_VERTEX, IDR_LANCZOS_FRAGMENT, SHADER_DOUBLE | SHADER_LEVELS),
		new ShaderGroup(GLSL_VER_1_30, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_2X, flags),
		new ShaderGroup(GLSL_VER_1_30, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_3X, flags),
		new ShaderGroup(GLSL_VER_1_30, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_4X, flags),
		new ShaderGroup(GLSL_VER_1_30, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_5X, flags),
		new ShaderGroup(GLSL_VER_1_30, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_6X, flags),
		new ShaderGroup(GLSL_VER_1_30, IDR_SCALEHQ_VERTEX_2X, IDR_SCALEHQ_FRAGMENT_2X, NULL),
		new ShaderGroup(GLSL_VER_1_30, IDR_SCALEHQ_VERTEX_4X, IDR_SCALEHQ_FRAGMENT_4X, NULL),
		new ShaderGroup(GLSL_VER_1_30, IDR_XSAL_VERTEX, IDR_XSAL_FRAGMENT, NULL),
		new ShaderGroup(GLSL_VER_1_30, IDR_EAGLE_VERTEX, IDR_EAGLE_FRAGMENT, NULL),
		new ShaderGroup(GLSL_VER_1_30, IDR_SCALENX_VERTEX_2X, IDR_SCALENX_FRAGMENT_2X, NULL),
		new ShaderGroup(GLSL_VER_1_30, IDR_SCALENX_VERTEX_3X, IDR_SCALENX_FRAGMENT_3X, NULL)
	};

	{
		GLGenVertexArrays(1, &this->arrayName);
		{
			GLBindVertexArray(this->arrayName);
			{
				GLGenBuffers(1, &this->bufferName);
				{
					GLBindBuffer(GL_ARRAY_BUFFER, this->bufferName);
					{
						FLOAT bf[8][8] = {
							{ 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
							{ (FLOAT)config.mode->width, 0.0f, 0.0f, 1.0f, texWidth, 0.0f, texWidth, 0.0f },
							{ (FLOAT)config.mode->width, (FLOAT)config.mode->height, 0.0f, 1.0f, texWidth, texHeight, texWidth, texHeight },
							{ 0.0f, (FLOAT)config.mode->height, 0.0f, 1.0f, 0.0f, texHeight, 0.0f, texHeight },

							{ 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f },
							{ (FLOAT)config.mode->width, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
							{ (FLOAT)config.mode->width, (FLOAT)config.mode->height, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f },
							{ 0.0f, (FLOAT)config.mode->height, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f }
						};
						MemoryCopy(this->buffer, bf, sizeof(this->buffer));

						{
							FLOAT mvp[4][4] = {
								{ FLOAT(2.0f / config.mode->width), 0.0f, 0.0f, 0.0f },
								{ 0.0f, FLOAT(-2.0f / config.mode->height), 0.0f, 0.0f },
								{ 0.0f, 0.0f, 2.0f, 0.0f },
								{ -1.0f, 1.0f, -1.0f, 1.0f }
							};

							for (DWORD i = 0; i < 8; ++i)
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
							GLBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(this->buffer) >> 1, this->buffer);
							GLBufferSubData(GL_ARRAY_BUFFER, sizeof(this->buffer) >> 1, sizeof(this->buffer) >> 1, this->buffer);
							GLBufferSubData(GL_ARRAY_BUFFER, sizeof(this->buffer), sizeof(this->buffer) >> 1, &this->buffer[4]);
							GLBufferSubData(GL_ARRAY_BUFFER, (sizeof(this->buffer) >> 1) * 3, sizeof(this->buffer) >> 1, &this->buffer[4]);
						}

						{
							GLEnableVertexAttribArray(0);
							GLVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 32, (GLvoid*)0);

							GLEnableVertexAttribArray(1);
							GLVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 32, (GLvoid*)16);
						}

						if (config.background.allowed)
							GLGenTextures(2, &textureId.back);
						else
							GLGenTextures(1, &textureId.primary);
						{
							if (config.background.allowed)
							{
								GLActiveTexture(GL_TEXTURE1);
								GLBindTexParameter(textureId.back, GL_LINEAR);

								DWORD length = config.mode->width * config.mode->height * sizeof(DWORD);
								VOID* tempBuffer = MemoryAlloc(length);
								{
									MemoryZero(tempBuffer, length);
									Main::LoadBack(tempBuffer, config.mode->width, config.mode->height);
									GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->maxTexSize, this->maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
									GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, config.mode->width, config.mode->height, GL_RGBA, GL_UNSIGNED_BYTE, tempBuffer);
								}
								MemoryFree(tempBuffer);
							}

							{
								GLActiveTexture(GL_TEXTURE0);
								GLBindTexParameter(textureId.primary, GL_LINEAR);

								if (this->isTrueColor)
									GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->maxTexSize, this->maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
								else
									GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->maxTexSize, this->maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
							}

							this->pixelBuffer = new PixelBuffer(this->isTrueColor);
							{
								this->viewSize = 0;
								this->fboId = 0;
								this->rboId = 0;
								this->frameBuffer = NULL;
								this->activeIndex = TRUE;
								this->zoomStatus = FALSE;
								this->borderStatus = FALSE;
								this->backStatus = FALSE;
								this->zoomStatus = FALSE;
								this->program = NULL;
								this->zoomSize = { 0, 0 };
								this->zoomFbSize = { 0, 0 };

								GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);

								this->isVSync = config.image.vSync;
								if (WGLSwapInterval)
									WGLSwapInterval(this->isVSync);
							}
						}
					}
				}
			}
		}
	}
}

VOID NewRenderer::End()
{
	{
		{
			{
				{
					{
						{
							{
								if (this->fboId)
								{
									GLDeleteTextures(config.background.allowed ? 3 : 2, (GLuint*)&textureId.secondary);
									GLDeleteRenderbuffers(1, &this->rboId);
									GLDeleteFramebuffers(1, &this->fboId);
									AlignedFree(this->frameBuffer);
								}
							}
							delete this->pixelBuffer;
						}
						if (config.background.allowed)
							GLDeleteTextures(2, &textureId.back);
						else
							GLDeleteTextures(1, &textureId.primary);
					}
					GLBindBuffer(GL_ARRAY_BUFFER, NULL);
				}
				GLDeleteBuffers(1, &this->bufferName);
			}
			GLBindVertexArray(NULL);
		}
		GLDeleteVertexArrays(1, &this->arrayName);
	}
	GLUseProgram(NULL);

	ShaderGroup** shader = (ShaderGroup**)&this->shaders;
	DWORD count = sizeof(this->shaders) / sizeof(ShaderGroup*);
	do
		delete *shader++;
	while (--count);
}

BOOL NewRenderer::RenderInner(BOOL ready, BOOL force, StateBufferAligned** lpStateBuffer)
{
	StateBufferAligned* stateBuffer = *lpStateBuffer;
	Size frameSize = stateBuffer->size;
	BOOL stateChanged = this->borderStatus != stateBuffer->borders || this->backStatus != stateBuffer->isBack || this->zoomStatus != stateBuffer->isZoomed;
	FilterState state = this->ddraw->filterState;
	this->ddraw->filterState.flags = FALSE;

	BOOL isSwap = FALSE;
	if (state.upscaling)
	{
		if (!this->fboId)
		{
			GLGenFramebuffers(1, &this->fboId);
			this->frameBuffer = AlignedAlloc(config.mode->width * config.mode->height * (this->isTrueColor ? sizeof(DWORD) : sizeof(WORD)));
		}

		DWORD newSize = MAKELONG(config.mode->width * state.value, config.mode->height * state.value);
		if (state.flags && newSize != this->viewSize)
			this->pixelBuffer->Reset();

		if (this->pixelBuffer->Update(lpStateBuffer, ready, TRUE) || state.flags || stateChanged)
		{
			if (this->isVSync != config.image.vSync)
			{
				this->isVSync = config.image.vSync;
				if (WGLSwapInterval)
					WGLSwapInterval(this->isVSync);
			}

			if (state.flags || stateChanged)
			{
				this->borderStatus = stateBuffer->borders;
				this->backStatus = stateBuffer->isBack;
				this->zoomStatus = stateBuffer->isZoomed;
				this->ddraw->viewport.refresh = TRUE;
			}

			FLOAT kw = (FLOAT)frameSize.width / config.mode->width;
			FLOAT kh = (FLOAT)frameSize.height / config.mode->height;

			GLBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->fboId);
			{
				if (state.flags)
				{
					switch (state.upscaling)
					{
					case UpscaleScaleNx:
						switch (state.value)
						{
						case 3:
							this->upscaleProgram = this->shaders.scaleNx_3x;
							break;
						default:
							this->upscaleProgram = this->shaders.scaleNx_2x;
							break;
						}

						break;

					case UpscaleScaleHQ:
						switch (state.value)
						{
						case 4:
							this->upscaleProgram = this->shaders.scaleHQ_4x;
							break;
						default:
							this->upscaleProgram = this->shaders.scaleHQ_2x;
							break;
						}

						break;

					case UpscaleXBRZ:
						switch (state.value)
						{
						case 6:
							this->upscaleProgram = this->shaders.xBRz_6x;
							break;
						case 5:
							this->upscaleProgram = this->shaders.xBRz_5x;
							break;
						case 4:
							this->upscaleProgram = this->shaders.xBRz_4x;
							break;
						case 3:
							this->upscaleProgram = this->shaders.xBRz_3x;
							break;
						default:
							this->upscaleProgram = this->shaders.xBRz_2x;
							break;
						}

						break;

					case UpscaleXSal:
						this->upscaleProgram = this->shaders.xSal_2x;
						break;

					default:
						this->upscaleProgram = this->shaders.eagle_2x;
						break;
					}

					if (newSize != this->viewSize)
					{
						this->viewSize = newSize;

						if (!this->rboId)
						{
							GLGenRenderbuffers(1, &this->rboId);
							{
								GLBindRenderbuffer(GL_RENDERBUFFER, this->rboId);
								GLRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, LOWORD(this->viewSize), HIWORD(this->viewSize));
								GLBindRenderbuffer(GL_RENDERBUFFER, NULL);
								GLFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboId);
							}

							GLGenTextures(config.background.allowed ? 3 : 2, (GLuint*)&textureId.secondary);
							{
								GLBindTexParameter(textureId.secondary, GL_LINEAR);

								if (this->isTrueColor)
									GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->maxTexSize, this->maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
								else
									GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->maxTexSize, this->maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
							}
						}

						// Gen texture
						DWORD idx = config.background.allowed ? 2 : 1;
						while (idx)
						{
							--idx;
							GLBindTexParameter(((GLuint*)&textureId.primaryBO)[idx], GL_LINEAR);
							GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, LOWORD(this->viewSize), HIWORD(this->viewSize), GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
						}
					}

					this->upscaleProgram->Use(this->texSize, FALSE);

					if (config.background.allowed)
					{
						GLViewport(0, 0, LOWORD(this->viewSize), HIWORD(this->viewSize));

						GLFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId.backBO, 0);

						GLActiveTexture(GL_TEXTURE1);
						GLBindTexFilter(((GLuint*)&textureId.primary)[this->activeIndex], GL_LINEAR);

						MemoryZero(this->frameBuffer, config.mode->width * config.mode->height * sizeof(DWORD));
						GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, config.mode->width, config.mode->height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);

						GLActiveTexture(GL_TEXTURE0);
						GLBindTexFilter(textureId.back, GL_LINEAR);

						GLDrawArrays(GL_TRIANGLE_FAN, 0, 4);
						GLFinish();
					}

					GLFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId.primaryBO, 0);
				}
				else
					this->upscaleProgram->Use(this->texSize, FALSE);

				if (stateBuffer->isZoomed)
					GLViewport(0, 0, DWORD(kw * LOWORD(this->viewSize)), DWORD(kh * HIWORD(this->viewSize)));
				else
					GLViewport(0, 0, LOWORD(this->viewSize), HIWORD(this->viewSize));

				GLActiveTexture(GL_TEXTURE1);
				GLBindTexFilter(((GLuint*)&textureId.primary)[this->activeIndex], GL_LINEAR);

				if (this->CheckView(FALSE) || this->zoomStatus != stateBuffer->isZoomed || stateBuffer->isZoomed && (this->zoomSize.width != frameSize.width || this->zoomSize.height != frameSize.height))
				{
					this->zoomStatus = stateBuffer->isZoomed;

					if (this->isTrueColor)
					{
						DWORD* ptr = (DWORD*)this->frameBuffer;
						DWORD count = frameSize.height * config.mode->width;
						do
							*ptr++ = 0x00FFFFFF;
						while (--count);
						GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameSize.width, frameSize.height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
					}
					else
					{
						MemoryZero(this->frameBuffer, frameSize.height * config.mode->width * (this->isTrueColor ? sizeof(DWORD) : sizeof(WORD)));
						GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameSize.width, frameSize.height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, frameBuffer);
					}
				}

				this->activeIndex = !this->activeIndex;
				GLActiveTexture(GL_TEXTURE0);
				GLBindTexFilter(((GLuint*)&textureId.primary)[this->activeIndex], GL_LINEAR);

				{
					GLPixelStorei(GL_UNPACK_ROW_LENGTH, config.mode->width);
					if (!this->isTrueColor)
						GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameSize.width, frameSize.height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (WORD*)stateBuffer->data + ((config.mode->height - frameSize.height) >> 1) * config.mode->width + ((config.mode->width - frameSize.width) >> 1));
					else
						GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameSize.width, frameSize.height, GL_RGBA, GL_UNSIGNED_BYTE, (DWORD*)stateBuffer->data + ((config.mode->height - frameSize.height) >> 1) * config.mode->width + ((config.mode->width - frameSize.width) >> 1));
					GLPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

					// Draw into FBO texture
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
				}

				GLFinish();
			}
			GLBindFramebuffer(GL_DRAW_FRAMEBUFFER, NULL);

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
			{
				GLViewport(this->ddraw->viewport.rectangle.x, this->ddraw->viewport.rectangle.y + this->ddraw->viewport.offset, this->ddraw->viewport.rectangle.width, this->ddraw->viewport.rectangle.height);

				if (stateBuffer->isBack)
				{
					GLActiveTexture(GL_TEXTURE1);
					GLBindTexFilter(textureId.backBO, state.interpolation != InterpolateNearest ? GL_LINEAR : GL_NEAREST);
				}

				GLActiveTexture(GL_TEXTURE0);
				GLBindTexFilter(textureId.primaryBO, state.interpolation == InterpolateLinear || state.interpolation == InterpolateHermite ? GL_LINEAR : GL_NEAREST);

				if (stateBuffer->isZoomed)
				{
					if (this->zoomFbSize.width != frameSize.width || this->zoomFbSize.height != frameSize.height)
					{
						this->zoomFbSize = frameSize;

						this->buffer[4][5] = kh;
						this->buffer[5][4] = kw;
						this->buffer[5][5] = kh;
						this->buffer[6][4] = kw;

						GLBufferSubData(GL_ARRAY_BUFFER, 12 * 8 * sizeof(FLOAT), 3 * 8 * sizeof(FLOAT), &this->buffer[4]);
					}

					GLDrawArrays(GL_TRIANGLE_FAN, 12, 4);
				}
				else
					GLDrawArrays(GL_TRIANGLE_FAN, 8, 4);
			}
			isSwap = TRUE;
		}
	}
	else
	{
		if (this->fboId)
		{
			GLDeleteTextures(config.background.allowed ? 3 : 2, (GLuint*)&textureId.secondary);
			GLDeleteRenderbuffers(1, &this->rboId);
			GLDeleteFramebuffers(1, &this->fboId);
			AlignedFree(this->frameBuffer);

			this->viewSize = 0;
			this->rboId = 0;
			this->fboId = 0;
			this->frameBuffer = NULL;

			this->pixelBuffer->Reset();

			if (config.background.allowed)
			{
				GLActiveTexture(GL_TEXTURE1);
				GLBindTexFilter(textureId.back, state.interpolation != InterpolateNearest ? GL_LINEAR : GL_NEAREST);
			}

			GLActiveTexture(GL_TEXTURE0);
			GLBindTexFilter(textureId.primary, state.interpolation == InterpolateLinear || state.interpolation == InterpolateHermite ? GL_LINEAR : GL_NEAREST);
		}

		if (this->pixelBuffer->Update(lpStateBuffer, ready) || state.flags || stateChanged)
		{
			if (this->isVSync != config.image.vSync)
			{
				this->isVSync = config.image.vSync;
				if (WGLSwapInterval)
					WGLSwapInterval(this->isVSync);
			}

			if (state.flags)
				this->ddraw->viewport.refresh = TRUE;

			if (this->CheckView(TRUE))
				GLViewport(this->ddraw->viewport.rectangle.x, this->ddraw->viewport.rectangle.y + this->ddraw->viewport.offset, this->ddraw->viewport.rectangle.width, this->ddraw->viewport.rectangle.height);

			if (state.interpolation > InterpolateLinear && frameSize.width == this->ddraw->viewport.rectangle.width && frameSize.height == this->ddraw->viewport.rectangle.height)
			{
				state.interpolation = InterpolateLinear;
				if (this->program != this->shaders.linear)
					state.flags = TRUE;
			}

			if (force || state.flags || stateChanged)
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
					if (stateBuffer->isBack)
					{
						GLActiveTexture(GL_TEXTURE1);
						GLBindTexFilter(textureId.back, state.interpolation != InterpolateNearest ? GL_LINEAR : GL_NEAREST);
					}

					GLActiveTexture(GL_TEXTURE0);
					GLBindTexFilter(textureId.primary, state.interpolation == InterpolateLinear || state.interpolation == InterpolateHermite ? GL_LINEAR : GL_NEAREST);
				}

				this->borderStatus = stateBuffer->borders;
				this->backStatus = stateBuffer->isBack;
				this->zoomStatus = stateBuffer->isZoomed;
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

			isSwap = TRUE;
		}
	}

	if (!isSwap && state.flags)
		this->ddraw->filterState.flags = TRUE;

	return isSwap;
}
