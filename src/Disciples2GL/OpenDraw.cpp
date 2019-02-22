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
#include "OpenDraw.h"
#include "Resource.h"
#include "CommCtrl.h"
#include "Main.h"
#include "FpsCounter.h"
#include "Config.h"
#include "Window.h"
#include "Hooks.h"

const GUID IID_IDirectDraw7 = { 0x15e65ec0, 0x3b9c, 0x11d2, 0xb9, 0x2f, 0x00, 0x60, 0x97, 0x97, 0xea, 0x5b };

VOID __fastcall UseShaderProgram(ShaderProgram* program, DWORD texSize)
{
	if (!program->id)
	{
		program->id = GLCreateProgram();

		GLuint vShader = GL::CompileShaderSource(program->vertexName, program->version, GL_VERTEX_SHADER);
		GLuint fShader = GL::CompileShaderSource(program->fragmentName, program->version, GL_FRAGMENT_SHADER);
		{
			GLAttachShader(program->id, vShader);
			GLAttachShader(program->id, fShader);
			{
				GLLinkProgram(program->id);
			}
			GLDetachShader(program->id, fShader);
			GLDetachShader(program->id, vShader);
		}
		GLDeleteShader(fShader);
		GLDeleteShader(vShader);

		GLUseProgram(program->id);
		GLUniformMatrix4fv(GLGetUniformLocation(program->id, "mvp"), 1, GL_FALSE, program->mvp);
		GLUniform1i(GLGetUniformLocation(program->id, "tex01"), 0);

		program->texSize.location = GLGetUniformLocation(program->id, "texSize");
		if (program->texSize.location >= 0)
		{
			program->texSize.value = texSize;
			GLUniform2f(program->texSize.location, (FLOAT)LOWORD(texSize), (FLOAT)HIWORD(texSize));
		}
	}
	else
	{
		GLUseProgram(program->id);

		if (program->texSize.location >= 0 && program->texSize.value != texSize)
		{
			program->texSize.value = texSize;
			FLOAT val = (FLOAT)texSize;
			GLUniform2f(program->texSize.location, (FLOAT)LOWORD(texSize), (FLOAT)HIWORD(texSize));
		}
	}
}

DWORD __stdcall RenderThread(LPVOID lpParameter)
{
	OpenDraw* ddraw = (OpenDraw*)lpParameter;
	ddraw->hDc = ::GetDC(ddraw->hDraw);
	{
		PIXELFORMATDESCRIPTOR pfd;
		GL::PreparePixelFormatDescription(&pfd);
		INT glPixelFormat = GL::PreparePixelFormat(&pfd);
		if (!glPixelFormat)
		{
			glPixelFormat = ChoosePixelFormat(ddraw->hDc, &pfd);
			if (!glPixelFormat)
				Main::ShowError("ChoosePixelFormat failed", __FILE__, __LINE__);
			else if (pfd.dwFlags & PFD_NEED_PALETTE)
				Main::ShowError("Needs palette", __FILE__, __LINE__);
		}

		if (!SetPixelFormat(ddraw->hDc, glPixelFormat, &pfd))
			Main::ShowError("SetPixelFormat failed", __FILE__, __LINE__);

		MemoryZero(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		if (DescribePixelFormat(ddraw->hDc, glPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd) == NULL)
			Main::ShowError("DescribePixelFormat failed", __FILE__, __LINE__);

		if ((pfd.iPixelType != PFD_TYPE_RGBA) ||
			(pfd.cRedBits < 5) || (pfd.cGreenBits < 5) || (pfd.cBlueBits < 5))
			Main::ShowError("Bad pixel type", __FILE__, __LINE__);

		HGLRC hRc = WGLCreateContext(ddraw->hDc);
		if (hRc)
		{
			if (WGLMakeCurrent(ddraw->hDc, hRc))
			{
				GL::CreateContextAttribs(ddraw->hDc, &hRc);
				if (glVersion >= GL_VER_2_0)
				{
					DWORD maxSize = ddraw->mode.width > ddraw->mode.height ? ddraw->mode.width : ddraw->mode.height;

					DWORD maxTexSize = 1;
					while (maxTexSize < maxSize)
						maxTexSize <<= 1;

					DWORD glMaxTexSize;
					GLGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&glMaxTexSize);
					if (maxTexSize > glMaxTexSize)
						glVersion = GL_VER_1_1;
				}

				Window::CheckMenu(ddraw->hWnd);
				if (glVersion >= GL_VER_3_0)
					ddraw->RenderNew();
				else if (glVersion >= GL_VER_2_0)
					ddraw->RenderMid();
				else
					ddraw->RenderOld();

				WGLMakeCurrent(ddraw->hDc, NULL);
			}

			WGLDeleteContext(hRc);
		}
	}
	::ReleaseDC(ddraw->hDraw, ddraw->hDc);
	ddraw->hDc = NULL;

	return NULL;
}

VOID OpenDraw::RenderOld()
{
	if (config.image.filter > FilterLinear)
		config.image.filter = FilterLinear;

	DWORD glMaxTexSize;
	GLGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&glMaxTexSize);
	if (glMaxTexSize < 256)
		glMaxTexSize = 256;

	DWORD size = this->mode.width > this->mode.height ? this->mode.width : this->mode.height;
	DWORD maxAllow = 1;
	while (maxAllow < size)
		maxAllow <<= 1;

	DWORD maxTexSize = maxAllow < glMaxTexSize ? maxAllow : glMaxTexSize;
	DWORD glFilter = config.image.filter == FilterNearest ? GL_NEAREST : GL_LINEAR;

	DWORD framePerWidth = this->mode.width / maxTexSize + (this->mode.width % maxTexSize ? 1 : 0);
	DWORD framePerHeight = this->mode.height / maxTexSize + (this->mode.height % maxTexSize ? 1 : 0);
	DWORD frameCount = framePerWidth * framePerHeight;
	Frame* frames = (Frame*)MemoryAlloc(frameCount * sizeof(Frame));
	{
		Frame* frame = frames;
		for (DWORD y = 0; y < this->mode.height; y += maxTexSize)
		{
			DWORD height = this->mode.height - y;
			if (height > maxTexSize)
				height = maxTexSize;

			for (DWORD x = 0; x < this->mode.width; x += maxTexSize, ++frame)
			{
				DWORD width = this->mode.width - x;
				if (width > maxTexSize)
					width = maxTexSize;

				frame->point.x = x;
				frame->point.y = y;

				frame->rect.x = x;
				frame->rect.y = y;
				frame->rect.width = width;
				frame->rect.height = height;

				frame->vSize.width = x + width;
				frame->vSize.height = y + height;

				frame->tSize.width = width == maxTexSize ? 1.0f : (FLOAT)width / maxTexSize;
				frame->tSize.height = height == maxTexSize ? 1.0f : (FLOAT)height / maxTexSize;

				GLGenTextures(1, &frame->id);

				GLBindTexture(GL_TEXTURE_2D, frame->id);

				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glCapsClampToEdge);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glCapsClampToEdge);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glFilter);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glFilter);

				GLTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

				if (this->mode.bpp == 16)
				{
					if (glVersion > GL_VER_1_1)
						GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
					else
						GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
				}
				else
					GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, maxTexSize, maxTexSize, GL_NONE, glCapsBGRA ? GL_BGRA_EXT : GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			}
		}

		GLMatrixMode(GL_PROJECTION);
		GLLoadIdentity();
		GLOrtho(0.0, (GLdouble)this->mode.width, (GLdouble)this->mode.height, 0.0, 0.0, 1.0);
		GLMatrixMode(GL_MODELVIEW);
		GLLoadIdentity();

		GLEnable(GL_TEXTURE_2D);
		GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		this->viewport.refresh = TRUE;

		VOID* frameBuffer = AlignedAlloc(maxTexSize * maxTexSize * (this->mode.bpp == 16 && glVersion > GL_VER_1_1 ? sizeof(WORD) : sizeof(DWORD)), 16);
		{
			FpsCounter* fpsCounter = new FpsCounter(FPS_ACCURACY);
			{
				BOOL isVSync = FALSE;
				if (WGLSwapInterval)
					WGLSwapInterval(0);

				do
				{
					OpenDrawSurface* surface = this->attachedSurface;
					if (this->attachedSurface)
					{
						if (WGLSwapInterval)
						{
							if (!isVSync)
							{
								if (config.image.vSync)
								{
									isVSync = TRUE;
									WGLSwapInterval(1);
								}
							}
							else
							{
								if (!config.image.vSync)
								{
									isVSync = FALSE;
									WGLSwapInterval(0);
								}
							}
						}

						if (fpsState)
						{
							if (isFpsChanged)
							{
								isFpsChanged = FALSE;
								fpsCounter->Reset();
							}

							fpsCounter->Calculate();
						}

						if (this->CheckView())
							GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height);

						glFilter = 0;
						if (this->isStateChanged)
						{
							this->isStateChanged = FALSE;
							glFilter = config.image.filter == FilterNearest ? GL_NEAREST : GL_LINEAR;
						}

						DWORD count = frameCount;
						frame = frames;
						while (count--)
						{
							if (frameCount == 1)
							{
								if (glFilter)
								{
									GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glFilter);
									GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glFilter);
								}

								if (this->mode.bpp == 32)
								{
									if (glCapsBGRA)
										GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->mode.width, this->mode.height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, surface->indexBuffer);
									else
									{
										DWORD* source = (DWORD*)surface->indexBuffer;
										DWORD* dest = (DWORD*)frameBuffer;
										DWORD copyWidth = this->mode.width;
										DWORD copyHeight = this->mode.height;
										do
										{
											DWORD* src = source;
											source += this->mode.width;

											DWORD count = copyWidth;
											do
												*dest++ = _byteswap_ulong(_rotl(*src++, 8));
											while (--count);
										} while (--copyHeight);

										GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->mode.width, this->mode.height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
									}
								}
								else
								{
									if (glVersion > GL_VER_1_1)
										GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->mode.width, this->mode.height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, surface->indexBuffer);
									else
									{
										WORD* source = (WORD*)surface->indexBuffer;
										DWORD* dest = (DWORD*)frameBuffer;
										DWORD copyWidth = this->mode.width;
										DWORD copyHeight = this->mode.height;
										do
										{
											WORD* src = source;
											source += this->mode.width;

											DWORD count = copyWidth;
											do
											{
												WORD px = *src++;
												*dest++ = ((px & 0xF800) >> 8) | ((px & 0x07E0) << 5) | ((px & 0x001F) << 19);
											} while (--count);
										} while (--copyHeight);

										GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->mode.width, this->mode.height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
									}
								}

							}
							else
							{
								GLBindTexture(GL_TEXTURE_2D, frame->id);

								if (glFilter)
								{
									GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glFilter);
									GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glFilter);
								}

								if (this->mode.bpp == 32)
								{
									if (glCapsBGRA)
									{
										DWORD* source = (DWORD*)surface->indexBuffer + frame->rect.y * this->mode.width + frame->rect.x;
										DWORD* dest = (DWORD*)frameBuffer;
										DWORD copyHeight = frame->rect.height;
										do
										{
											MemoryCopy(dest, source, frame->rect.width << 2);
											source += this->mode.width;
											dest += frame->rect.width;
										} while (--copyHeight);

										GLTexSubImage2D(GL_TEXTURE_2D, 0, frame->rect.x, frame->rect.y, frame->rect.width, frame->rect.height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, frameBuffer);
									}
									else
									{
										DWORD* source = (DWORD*)surface->indexBuffer + frame->rect.y * this->mode.width + frame->rect.x;
										DWORD* dest = (DWORD*)frameBuffer;
										DWORD copyHeight = frame->rect.height;
										do
										{
											DWORD* src = source;
											source += this->mode.width;

											DWORD count = frame->rect.width;
											do
												*dest++ = _byteswap_ulong(_rotl(*src++, 8));
											while (--count);
										} while (--copyHeight);

										GLTexSubImage2D(GL_TEXTURE_2D, 0, frame->rect.x, frame->rect.y, frame->rect.width, frame->rect.height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
									}
								}
								else
								{
									if (glVersion > GL_VER_1_1)
									{
										WORD* source = (WORD*)surface->indexBuffer + frame->rect.x * this->mode.width + frame->rect.x;
										WORD* dest = (WORD*)frameBuffer;
										DWORD copyHeight = frame->rect.height;
										do
										{
											MemoryCopy(dest, source, frame->rect.width << 1);
											source += this->mode.width;
											dest += frame->rect.width;
										} while (--copyHeight);

										GLTexSubImage2D(GL_TEXTURE_2D, 0, frame->rect.x, frame->rect.y, frame->rect.width, frame->rect.height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, frameBuffer);
									}
									else
									{
										WORD* source = (WORD*)surface->indexBuffer + frame->rect.x * this->mode.width + frame->rect.x;
										DWORD* dest = (DWORD*)frameBuffer;
										DWORD copyHeight = frame->rect.height;
										do
										{
											WORD* src = source;
											source += this->mode.width;

											DWORD count = frame->rect.width;
											do
											{
												WORD px = *src++;
												*dest++ = ((px & 0xF800) >> 8) | ((px & 0x07E0) << 5) | ((px & 0x001F) << 19);
											} while (--count);
										} while (--copyHeight);

										GLTexSubImage2D(GL_TEXTURE_2D, 0, frame->rect.x, frame->rect.y, frame->rect.width, frame->rect.height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
									}
								}
							}

							if (fpsState && frame == frames)
							{
								DWORD fps = fpsCounter->value;
								DWORD digCount = 0;
								DWORD current = fps;
								do
								{
									++digCount;
									current = current / 10;
								} while (current);

								if (this->mode.bpp == 32)
								{
									if (glCapsBGRA)
									{
										DWORD fpsColor = fpsState == FpsBenchmark ? 0xFFFFFF00 : 0xFFFFFFFF;
										DWORD dcount = digCount;
										do
										{
											WORD* lpDig = (WORD*)counters[fps % 10];

											for (DWORD y = 0; y < FPS_HEIGHT; ++y)
											{
												DWORD* idx = (DWORD*)surface->indexBuffer + (FPS_Y + y) * this->mode.width +
													FPS_X + FPS_WIDTH * (dcount - 1);

												DWORD* pix = (DWORD*)frameBuffer + y * FPS_WIDTH * 4 +
													FPS_WIDTH * (dcount - 1);

												WORD check = *lpDig++;
												DWORD width = FPS_WIDTH;
												do
												{
													*pix++ = (check & 1) ? fpsColor : *idx;
													++idx;
													check >>= 1;
												} while (--width);
											}

											fps = fps / 10;
										} while (--dcount);

										dcount = 4;
										while (dcount != digCount)
										{
											for (DWORD y = 0; y < FPS_HEIGHT; ++y)
											{
												DWORD* idx = (DWORD*)surface->indexBuffer + (FPS_Y + y) * this->mode.width +
													FPS_X + FPS_WIDTH * (dcount - 1);

												DWORD* pix = (DWORD*)frameBuffer + y * FPS_WIDTH * 4 +
													FPS_WIDTH * (dcount - 1);

												DWORD width = FPS_WIDTH;
												do
													*pix++ = *idx++;
												while (--width);
											}

											--dcount;
										}

										GLTexSubImage2D(GL_TEXTURE_2D, 0, FPS_X, FPS_Y, FPS_WIDTH * 4, FPS_HEIGHT, GL_BGRA_EXT, GL_UNSIGNED_BYTE, frameBuffer);
									}
									else
									{
										DWORD fpsColor = fpsState == FpsBenchmark ? 0xFF00FFFF : 0xFFFFFFFF;
										DWORD dcount = digCount;
										do
										{
											WORD* lpDig = (WORD*)counters[fps % 10];

											for (DWORD y = 0; y < FPS_HEIGHT; ++y)
											{
												DWORD* idx = (DWORD*)surface->indexBuffer + (FPS_Y + y) * this->mode.width +
													FPS_X + FPS_WIDTH * (dcount - 1);

												DWORD* pix = (DWORD*)frameBuffer + y * FPS_WIDTH * 4 +
													FPS_WIDTH * (dcount - 1);

												WORD check = *lpDig++;
												DWORD width = FPS_WIDTH;
												do
												{
													if (check & 1)
													{
														++idx;
														*pix++ = fpsColor;
													}
													else
														*pix++ = _byteswap_ulong(_rotl(*idx++, 8));

													check >>= 1;
												} while (--width);
											}

											fps = fps / 10;
										} while (--dcount);

										dcount = 4;
										while (dcount != digCount)
										{
											for (DWORD y = 0; y < FPS_HEIGHT; ++y)
											{
												DWORD* idx = (DWORD*)surface->indexBuffer + (FPS_Y + y) * this->mode.width +
													FPS_X + FPS_WIDTH * (dcount - 1);

												DWORD* pix = (DWORD*)frameBuffer + y * FPS_WIDTH * 4 +
													FPS_WIDTH * (dcount - 1);

												DWORD width = FPS_WIDTH;
												do
													*pix++ = _byteswap_ulong(_rotl(*idx++, 8));
												while (--width);
											}

											--dcount;
										}

										GLTexSubImage2D(GL_TEXTURE_2D, 0, FPS_X, FPS_Y, FPS_WIDTH * 4, FPS_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
									}
								}
								else
								{
									if (glVersion > GL_VER_1_1)
									{
										WORD fpsColor = fpsState == FpsBenchmark ? 0xFFE0 : 0xFFFF;
										DWORD dcount = digCount;
										do
										{
											WORD* lpDig = (WORD*)counters[fps % 10];

											for (DWORD y = 0; y < FPS_HEIGHT; ++y)
											{
												WORD* idx = (WORD*)surface->indexBuffer + (FPS_Y + y) * this->mode.width +
													FPS_X + FPS_WIDTH * (dcount - 1);

												WORD* pix = (WORD*)frameBuffer + y * FPS_WIDTH * 4 +
													FPS_WIDTH * (dcount - 1);

												WORD check = *lpDig++;
												DWORD width = FPS_WIDTH;
												do
												{
													*pix++ = (check & 1) ? fpsColor : *idx;
													++idx;
													check >>= 1;
												} while (--width);
											}

											fps = fps / 10;
										} while (--dcount);

										dcount = 4;
										while (dcount != digCount)
										{
											for (DWORD y = 0; y < FPS_HEIGHT; ++y)
											{
												WORD* idx = (WORD*)surface->indexBuffer + (FPS_Y + y) * this->mode.width +
													FPS_X + FPS_WIDTH * (dcount - 1);

												WORD* pix = (WORD*)frameBuffer + y * FPS_WIDTH * 4 +
													FPS_WIDTH * (dcount - 1);

												DWORD width = FPS_WIDTH;
												do
													*pix++ = *idx++;
												while (--width);
											}

											--dcount;
										}

										GLTexSubImage2D(GL_TEXTURE_2D, 0, FPS_X, FPS_Y, FPS_WIDTH * 4, FPS_HEIGHT, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, frameBuffer);
									}
									else
									{
										DWORD fpsColor = fpsState == FpsBenchmark ? 0xFF00FFFF : 0xFFFFFFFF;
										DWORD dcount = digCount;
										do
										{
											WORD* lpDig = (WORD*)counters[fps % 10];

											for (DWORD y = 0; y < FPS_HEIGHT; ++y)
											{
												WORD* idx = (WORD*)surface->indexBuffer + (FPS_Y + y) * this->mode.width +
													FPS_X + FPS_WIDTH * (dcount - 1);

												DWORD* pix = (DWORD*)frameBuffer + y * FPS_WIDTH * 4 +
													FPS_WIDTH * (dcount - 1);

												WORD check = *lpDig++;
												DWORD width = FPS_WIDTH;
												do
												{
													if (check & 1)
														*pix = fpsColor;
													else
													{
														WORD px = *idx;
														*pix = ((px & 0xF800) >> 8) | ((px & 0x07E0) << 5) | ((px & 0x001F) << 19);
													}

													++pix;
													++idx;
													check >>= 1;
												} while (--width);
											}

											fps = fps / 10;
										} while (--dcount);

										dcount = 4;
										while (dcount != digCount)
										{
											for (DWORD y = 0; y < FPS_HEIGHT; ++y)
											{
												WORD* idx = (WORD*)surface->indexBuffer + (FPS_Y + y) * this->mode.width +
													FPS_X + FPS_WIDTH * (dcount - 1);

												DWORD* pix = (DWORD*)frameBuffer + y * FPS_WIDTH * 4 +
													FPS_WIDTH * (dcount - 1);

												DWORD width = FPS_WIDTH;
												do
												{
													WORD px = *idx++;
													*pix++ = ((px & 0xF800) >> 8) | ((px & 0x07E0) << 5) | ((px & 0x001F) << 19);
												} while (--width);
											}

											--dcount;
										}

										GLTexSubImage2D(GL_TEXTURE_2D, 0, FPS_X, FPS_Y, FPS_WIDTH * 4, FPS_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
									}
								}
							}

							GLBegin(GL_TRIANGLE_FAN);
							{
								GLTexCoord2f(0.0f, 0.0f);
								GLVertex2s((SHORT)frame->point.x, (SHORT)frame->point.y);

								GLTexCoord2f(frame->tSize.width, 0.0f);
								GLVertex2s(frame->vSize.width, (SHORT)frame->point.y);

								GLTexCoord2f(frame->tSize.width, frame->tSize.height);
								GLVertex2s(frame->vSize.width, frame->vSize.height);

								GLTexCoord2f(0.0f, frame->tSize.height);
								GLVertex2s((SHORT)frame->point.x, frame->vSize.height);
							}
							GLEnd();
							++frame;
						}

						if (this->isTakeSnapshot)
						{
							this->isTakeSnapshot = FALSE;

							if (OpenClipboard(NULL))
							{
								EmptyClipboard();

								DWORD dataSize = this->mode.width * this->mode.height * sizeof(WORD);
								HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPV5HEADER) + dataSize);
								{
									VOID* data = GlobalLock(hMemory);
									{
										BITMAPV5HEADER* bmi = (BITMAPV5HEADER*)data;
										MemoryZero(bmi, sizeof(BITMAPINFOHEADER));
										bmi->bV5Size = sizeof(BITMAPV5HEADER);
										bmi->bV5Width = this->mode.width;
										bmi->bV5Height = -*(LONG*)&this->mode.height;
										bmi->bV5Planes = 1;
										bmi->bV5BitCount = 16;
										bmi->bV5Compression = BI_BITFIELDS;
										bmi->bV5XPelsPerMeter = 1;
										bmi->bV5YPelsPerMeter = 1;
										bmi->bV5RedMask = 0xF800;
										bmi->bV5GreenMask = 0x07E0;
										bmi->bV5BlueMask = 0x001F;

										MemoryCopy((BYTE*)data + sizeof(BITMAPV5HEADER), surface->indexBuffer, dataSize);
									}
									GlobalUnlock(hMemory);

									SetClipboardData(CF_DIBV5, hMemory);
								}
								GlobalFree(hMemory);

								CloseClipboard();
							}
						}

						SwapBuffers(this->hDc);
						if (fpsState != FpsBenchmark)
							WaitForSingleObject(this->hDrawEvent, INFINITE);
						if (isVSync)
							GLFinish();
					}
				} while (!this->isFinish);
			}
			delete fpsCounter;
		}
		AlignedFree(frameBuffer);

		frame = frames;
		DWORD count = frameCount;
		while (count--)
		{
			GLDeleteTextures(1, &frame->id);
			++frame;
		}
	}
	MemoryFree(frames);
}

VOID OpenDraw::RenderMid()
{
	if (config.image.filter > FilterCubic)
		config.image.filter = FilterLinear;

	DWORD maxSize = this->mode.width > this->mode.height ? this->mode.width : this->mode.height;
	DWORD maxTexSize = 1;
	while (maxTexSize < maxSize) maxTexSize <<= 1;
	FLOAT texWidth = this->mode.width == maxTexSize ? 1.0f : (FLOAT)this->mode.width / maxTexSize;
	FLOAT texHeight = this->mode.height == maxTexSize ? 1.0f : (FLOAT)this->mode.height / maxTexSize;

	DWORD texSize = (maxTexSize & 0xFFFF) | (maxTexSize << 16);

	FLOAT buffer[4][4] = {
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ (FLOAT)this->mode.width, 0.0f, texWidth, 0.0f },
		{ (FLOAT)this->mode.width, (FLOAT)this->mode.height, texWidth, texHeight },
		{ 0.0f, (FLOAT)this->mode.height, 0.0f, texHeight }
	};

	FLOAT mvpMatrix[4][4] = {
		{ FLOAT(2.0f / this->mode.width), 0.0f, 0.0f, 0.0f },
		{ 0.0f, FLOAT(-2.0f / this->mode.height), 0.0f, 0.0f },
		{ 0.0f, 0.0f, 2.0f, 0.0f },
		{ -1.0f, 1.0f, -1.0f, 1.0f }
	};

	struct {
		ShaderProgram linear;
		ShaderProgram cubic;
	} shaders = {
		{ 0, GLSL_VER_1_10, IDR_LINEAR_VERTEX, IDR_LINEAR_FRAGMENT, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_10, IDR_CUBIC_VERTEX, IDR_CUBIC_FRAGMENT, (GLfloat*)mvpMatrix }
	};

	ShaderProgram* filterProgram = &shaders.linear;
	{
		GLuint bufferName;
		GLGenBuffers(1, &bufferName);
		{
			GLBindBuffer(GL_ARRAY_BUFFER, bufferName);
			{
				GLBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_STATIC_DRAW);

				UseShaderProgram(filterProgram, texSize);
				GLint attrLoc = GLGetAttribLocation(filterProgram->id, "vCoord");
				GLEnableVertexAttribArray(attrLoc);
				GLVertexAttribPointer(attrLoc, 2, GL_FLOAT, GL_FALSE, 16, (GLvoid*)0);

				attrLoc = GLGetAttribLocation(filterProgram->id, "vTexCoord");
				GLEnableVertexAttribArray(attrLoc);
				GLVertexAttribPointer(attrLoc, 2, GL_FLOAT, GL_FALSE, 16, (GLvoid*)8);

				GLuint textureId;
				GLGenTextures(1, &textureId);
				{
					DWORD filter = config.image.filter == FilterLinear ? GL_LINEAR : GL_NEAREST;
					GLActiveTexture(GL_TEXTURE0);

					GLBindTexture(GL_TEXTURE_2D, textureId);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glCapsClampToEdge);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glCapsClampToEdge);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

					if (this->mode.bpp == 32)
						GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);
					else
						GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);

					GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);
					this->viewport.refresh = TRUE;

					VOID* frameBuffer = AlignedAlloc(this->mode.width * this->mode.height * (this->mode.bpp == 16 ? sizeof(WORD) : sizeof(DWORD)), 16);
					{
						FpsCounter* fpsCounter = new FpsCounter(FPS_ACCURACY);
						{
							this->isStateChanged = TRUE;

							BOOL isVSync = FALSE;
							if (WGLSwapInterval)
								WGLSwapInterval(0);

							do
							{
								OpenDrawSurface* surface = this->attachedSurface;
								if (surface)
								{
									if (WGLSwapInterval)
									{
										if (!isVSync)
										{
											if (config.image.vSync)
											{
												isVSync = TRUE;
												WGLSwapInterval(1);
											}
										}
										else
										{
											if (!config.image.vSync)
											{
												isVSync = FALSE;
												WGLSwapInterval(0);
											}
										}
									}

									if (fpsState)
									{
										if (isFpsChanged)
										{
											isFpsChanged = FALSE;
											fpsCounter->Reset();
										}

										fpsCounter->Calculate();
									}

									if (this->CheckView())
										GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height);

									if (this->isStateChanged)
									{
										this->isStateChanged = FALSE;

										ImageFilter frameFilter = config.image.filter;
										filterProgram = frameFilter == FilterCubic ? &shaders.cubic : &shaders.linear;
										UseShaderProgram(filterProgram, texSize);

										GLBindTexture(GL_TEXTURE_2D, textureId);
										filter = frameFilter == FilterLinear ? GL_LINEAR : GL_NEAREST;
										GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
										GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
									}

									// NEXT UNCHANGED
									{
										if (this->mode.bpp == 32)
											GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->mode.width, this->mode.height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, surface->indexBuffer);
										else
											GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->mode.width, this->mode.height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, surface->indexBuffer);

										// Update FPS
										if (fpsState && !this->isTakeSnapshot)
										{
											DWORD fps = fpsCounter->value;
											DWORD digCount = 0;
											DWORD current = fps;
											do
											{
												++digCount;
												current = current / 10;
											} while (current);

											if (this->mode.bpp == 32)
											{
												DWORD fpsColor = fpsState == FpsBenchmark ? 0xFFFFFF00 : 0xFFFFFFFF;
												DWORD dcount = digCount;
												do
												{
													WORD* lpDig = (WORD*)counters[fps % 10];

													for (DWORD y = 0; y < FPS_HEIGHT; ++y)
													{
														DWORD* idx = (DWORD*)surface->indexBuffer + (FPS_Y + y) * this->mode.width +
															FPS_X + FPS_WIDTH * (dcount - 1);

														DWORD* pix = (DWORD*)frameBuffer + y * FPS_WIDTH * 4 +
															FPS_WIDTH * (dcount - 1);

														WORD check = *lpDig++;
														DWORD width = FPS_WIDTH;
														do
														{
															*pix++ = (check & 1) ? fpsColor : *idx;
															++idx;
															check >>= 1;
														} while (--width);
													}

													fps = fps / 10;
												} while (--dcount);

												dcount = 4;
												while (dcount != digCount)
												{
													for (DWORD y = 0; y < FPS_HEIGHT; ++y)
													{
														DWORD* idx = (DWORD*)surface->indexBuffer + (FPS_Y + y) * this->mode.width +
															FPS_X + FPS_WIDTH * (dcount - 1);

														DWORD* pix = (DWORD*)frameBuffer + y * FPS_WIDTH * 4 +
															FPS_WIDTH * (dcount - 1);

														DWORD width = FPS_WIDTH;
														do
															*pix++ = *idx++;
														while (--width);
													}

													--dcount;
												}

												GLTexSubImage2D(GL_TEXTURE_2D, 0, FPS_X, FPS_Y, FPS_WIDTH * 4, FPS_HEIGHT, GL_BGRA_EXT, GL_UNSIGNED_BYTE, frameBuffer);
											}
											else
											{
												WORD fpsColor = fpsState == FpsBenchmark ? 0xFFE0 : 0xFFFF;
												DWORD dcount = digCount;
												do
												{
													WORD* lpDig = (WORD*)counters[fps % 10];

													for (DWORD y = 0; y < FPS_HEIGHT; ++y)
													{
														WORD* idx = (WORD*)surface->indexBuffer + (FPS_Y + y) * this->mode.width +
															FPS_X + FPS_WIDTH * (dcount - 1);

														WORD* pix = (WORD*)frameBuffer + y * FPS_WIDTH * 4 +
															FPS_WIDTH * (dcount - 1);

														WORD check = *lpDig++;
														DWORD width = FPS_WIDTH;
														do
														{
															*pix++ = (check & 1) ? fpsColor : *idx;
															++idx;
															check >>= 1;
														} while (--width);
													}

													fps = fps / 10;
												} while (--dcount);

												dcount = 4;
												while (dcount != digCount)
												{
													for (DWORD y = 0; y < FPS_HEIGHT; ++y)
													{
														WORD* idx = (WORD*)surface->indexBuffer + (FPS_Y + y) * this->mode.width +
															FPS_X + FPS_WIDTH * (dcount - 1);

														WORD* pix = (WORD*)frameBuffer + y * FPS_WIDTH * 4 +
															FPS_WIDTH * (dcount - 1);

														DWORD width = FPS_WIDTH;
														do
															*pix++ = *idx++;
														while (--width);
													}

													--dcount;
												}

												GLTexSubImage2D(GL_TEXTURE_2D, 0, FPS_X, FPS_Y, FPS_WIDTH * 4, FPS_HEIGHT, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, frameBuffer);
											}
										}

										GLDrawArrays(GL_TRIANGLE_FAN, 0, 4);
									}

									if (this->isTakeSnapshot)
									{
										this->isTakeSnapshot = FALSE;

										if (OpenClipboard(NULL))
										{
											EmptyClipboard();

											DWORD dataSize = this->mode.width * this->mode.height * sizeof(WORD);
											HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPV5HEADER) + dataSize);
											{
												VOID* data = GlobalLock(hMemory);
												{
													BITMAPV5HEADER* bmi = (BITMAPV5HEADER*)data;
													MemoryZero(bmi, sizeof(BITMAPINFOHEADER));
													bmi->bV5Size = sizeof(BITMAPV5HEADER);
													bmi->bV5Width = this->mode.width;
													bmi->bV5Height = -*(LONG*)&this->mode.height;
													bmi->bV5Planes = 1;
													bmi->bV5BitCount = 16;
													bmi->bV5Compression = BI_BITFIELDS;
													bmi->bV5XPelsPerMeter = 1;
													bmi->bV5YPelsPerMeter = 1;
													bmi->bV5RedMask = 0xF800;
													bmi->bV5GreenMask = 0x07E0;
													bmi->bV5BlueMask = 0x001F;

													MemoryCopy((BYTE*)data + sizeof(BITMAPV5HEADER), surface->indexBuffer, dataSize);
												}
												GlobalUnlock(hMemory);

												SetClipboardData(CF_DIBV5, hMemory);
											}
											GlobalFree(hMemory);

											CloseClipboard();
										}
									}

									// Swap
									SwapBuffers(this->hDc);
									if (fpsState != FpsBenchmark)
										WaitForSingleObject(this->hDrawEvent, INFINITE);
									if (isVSync)
										GLFinish();
								}
							} while (!this->isFinish);
						}
						delete fpsCounter;
					}
					AlignedFree(frameBuffer);
				}
				GLDeleteTextures(1, &textureId);
			}
			GLBindBuffer(GL_ARRAY_BUFFER, NULL);
		}
		GLDeleteBuffers(1, &bufferName);
	}
	GLUseProgram(NULL);

	ShaderProgram* shaderProgram = (ShaderProgram*)&shaders;
	DWORD count = sizeof(shaders) / sizeof(ShaderProgram);
	do
	{
		if (shaderProgram->id)
			GLDeleteProgram(shaderProgram->id);

	} while (--count);
}

VOID OpenDraw::RenderNew()
{
	DWORD maxSize = this->mode.width > this->mode.height ? this->mode.width : this->mode.height;
	DWORD maxTexSize = 1;
	while (maxTexSize < maxSize) maxTexSize <<= 1;
	FLOAT texWidth = this->mode.width == maxTexSize ? 1.0f : (FLOAT)this->mode.width / maxTexSize;
	FLOAT texHeight = this->mode.height == maxTexSize ? 1.0f : (FLOAT)this->mode.height / maxTexSize;

	DWORD texSize = (maxTexSize & 0xFFFF) | (maxTexSize << 16);

	FLOAT buffer[8][4] = {
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ (FLOAT)this->mode.width, 0.0f, texWidth, 0.0f },
		{ (FLOAT)this->mode.width, (FLOAT)this->mode.height, texWidth, texHeight },
		{ 0.0f, (FLOAT)this->mode.height, 0.0f, texHeight },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ (FLOAT)this->mode.width, 0.0f, 1.0f, 1.0f },
		{ (FLOAT)this->mode.width, (FLOAT)this->mode.height, 1.0f, 0.0f },
		{ 0.0f, (FLOAT)this->mode.height, 0.0f, 0.0f }
	};

	FLOAT mvpMatrix[4][4] = {
		{ FLOAT(2.0f / this->mode.width), 0.0f, 0.0f, 0.0f },
		{ 0.0f, FLOAT(-2.0f / this->mode.height), 0.0f, 0.0f },
		{ 0.0f, 0.0f, 2.0f, 0.0f },
		{ -1.0f, 1.0f, -1.0f, 1.0f }
	};

	struct {
		ShaderProgram linear;
		ShaderProgram cubic;
		ShaderProgram xBRz_2x;
		ShaderProgram xBRz_3x;
		ShaderProgram xBRz_4x;
		ShaderProgram xBRz_5x;
		ShaderProgram xBRz_6x;
		ShaderProgram scaleHQ_2x;
		ShaderProgram scaleHQ_4x;
		ShaderProgram xSal_2x;
		ShaderProgram eagle_2x;
		ShaderProgram scaleNx_2x;
		ShaderProgram scaleNx_3x;
	} shaders = {
		{ 0, GLSL_VER_1_30, IDR_LINEAR_VERTEX, IDR_LINEAR_FRAGMENT, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_CUBIC_VERTEX, IDR_CUBIC_FRAGMENT, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_2X, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_3X, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_4X, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_5X, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_6X, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_SCALEHQ_VERTEX_2X, IDR_SCALEHQ_FRAGMENT_2X, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_SCALEHQ_VERTEX_4X, IDR_SCALEHQ_FRAGMENT_4X, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_XSAL_VERTEX, IDR_XSAL_FRAGMENT, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_EAGLE_VERTEX, IDR_EAGLE_FRAGMENT, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_SCALENX_VERTEX_2X, IDR_SCALENX_FRAGMENT_2X, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_SCALENX_VERTEX_3X, IDR_SCALENX_FRAGMENT_3X, (GLfloat*)mvpMatrix }
	};

	ShaderProgram* filterProgram = &shaders.linear;
	ShaderProgram* filterProgram2 = &shaders.linear;
	{
		POINTFLOAT* stencil = NULL;
		GLuint stArrayName, stBufferName, arrayName;

		GLGenVertexArrays(1, &arrayName);
		{
			GLBindVertexArray(arrayName);
			{
				GLuint bufferName;
				GLGenBuffers(1, &bufferName);
				{
					GLBindBuffer(GL_ARRAY_BUFFER, bufferName);
					{
						GLBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_STATIC_DRAW);

						UseShaderProgram(filterProgram, texSize);
						GLint attrLoc = GLGetAttribLocation(filterProgram->id, "vCoord");
						GLEnableVertexAttribArray(attrLoc);
						GLVertexAttribPointer(attrLoc, 2, GL_FLOAT, GL_FALSE, 16, (GLvoid*)0);

						attrLoc = GLGetAttribLocation(filterProgram->id, "vTexCoord");
						GLEnableVertexAttribArray(attrLoc);
						GLVertexAttribPointer(attrLoc, 2, GL_FLOAT, GL_FALSE, 16, (GLvoid*)8);

						GLuint textureId;
						GLGenTextures(1, &textureId);
						{
							DWORD filter = config.image.filter == FilterLinear ? GL_LINEAR : GL_NEAREST;
							GLActiveTexture(GL_TEXTURE0);

							GLBindTexture(GL_TEXTURE_2D, textureId);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glCapsClampToEdge);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glCapsClampToEdge);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

							if (this->mode.bpp == 32)
								GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);
							else
								GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);

							GLuint fboId;
							GLGenFramebuffers(1, &fboId);
							{
								DWORD viewSize = 0;
								GLuint rboId = 0, tboId = 0;
								{
									GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);
									this->viewport.refresh = TRUE;

									VOID* frameBuffer = AlignedAlloc(this->mode.width * this->mode.height * (this->mode.bpp == 16 ? sizeof(WORD) : sizeof(DWORD)), 16);
									{
										FpsCounter* fpsCounter = new FpsCounter(FPS_ACCURACY);
										{
											this->isStateChanged = TRUE;

											BOOL isVSync = FALSE;
											if (WGLSwapInterval)
												WGLSwapInterval(0);

											do
											{
												OpenDrawSurface* surface = this->attachedSurface;
												if (surface)
												{
													if (WGLSwapInterval)
													{
														if (!isVSync)
														{
															if (config.image.vSync)
															{
																isVSync = TRUE;
																WGLSwapInterval(1);
															}
														}
														else
														{
															if (!config.image.vSync)
															{
																isVSync = FALSE;
																WGLSwapInterval(0);
															}
														}
													}

													if (this->isStateChanged)
														this->viewport.refresh = TRUE;

													if (fpsState)
													{
														if (isFpsChanged)
														{
															isFpsChanged = FALSE;
															fpsCounter->Reset();
														}

														fpsCounter->Calculate();
													}

													ImageFilter frameFilter = config.image.filter;
													if (frameFilter == FilterXRBZ || frameFilter == FilterScaleHQ ||
														frameFilter == FilterXSal || frameFilter == FilterEagle || frameFilter == FilterScaleNx)
													{
														GLBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboId);

														if (this->isStateChanged)
														{
															this->isStateChanged = FALSE;

															DWORD scaleValue;
															switch (frameFilter)
															{
															case FilterScaleNx:
																scaleValue = config.image.scaleNx.value;
																switch (scaleValue)
																{
																case 4:
																	filterProgram = &shaders.scaleNx_3x;
																	break;
																default:
																	filterProgram = &shaders.scaleNx_2x;
																	break;
																}
																filterProgram2 = config.image.scaleNx.type ? &shaders.cubic : &shaders.linear;
																break;

															case FilterScaleHQ:
																scaleValue = config.image.scaleHQ.value;
																switch (scaleValue)
																{
																case 4:
																	filterProgram = &shaders.scaleHQ_4x;
																	break;
																default:
																	filterProgram = &shaders.scaleHQ_2x;
																	break;
																}
																filterProgram2 = config.image.scaleHQ.type ? &shaders.cubic : &shaders.linear;
																break;

															case FilterXRBZ:
																scaleValue = config.image.xBRz.value;
																switch (scaleValue)
																{
																case 6:
																	filterProgram = &shaders.xBRz_6x;
																	break;
																case 5:
																	filterProgram = &shaders.xBRz_5x;
																	break;
																case 4:
																	filterProgram = &shaders.xBRz_4x;
																	break;
																case 3:
																	filterProgram = &shaders.xBRz_3x;
																	break;
																default:
																	filterProgram = &shaders.xBRz_2x;
																	break;
																}
																filterProgram2 = config.image.xBRz.type ? &shaders.cubic : &shaders.linear;
																break;

															case FilterXSal:
																scaleValue = config.image.xSal.value;
																filterProgram = &shaders.xSal_2x;
																filterProgram2 = config.image.xSal.type ? &shaders.cubic : &shaders.linear;
																break;

															default:
																scaleValue = config.image.eagle.value;
																filterProgram = &shaders.eagle_2x;
																filterProgram2 = config.image.eagle.type ? &shaders.cubic : &shaders.linear;
																break;
															}

															UseShaderProgram(filterProgram, texSize);

															DWORD newSize = ((this->mode.width * scaleValue) & 0xFFFF) | ((this->mode.height * scaleValue) << 16);
															if (newSize != viewSize)
															{
																if (!viewSize)
																{
																	GLGenTextures(1, &tboId);
																	GLGenRenderbuffers(1, &rboId);
																}

																viewSize = newSize;

																// Gen texture
																GLBindTexture(GL_TEXTURE_2D, tboId);
																GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glCapsClampToEdge);
																GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glCapsClampToEdge);
																GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
																GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
																GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
																GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
																GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, LOWORD(viewSize), HIWORD(viewSize), GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

																// Get storage
																GLBindRenderbuffer(GL_RENDERBUFFER, rboId);
																GLRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, LOWORD(viewSize), HIWORD(viewSize));
																GLBindRenderbuffer(GL_RENDERBUFFER, NULL);

																GLFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tboId, 0);
																GLFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboId);
															}
														}

														GLViewport(0, 0, LOWORD(viewSize), HIWORD(viewSize));
														this->CheckView();

														GLBindTexture(GL_TEXTURE_2D, textureId);
														GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
														GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
													}
													else
													{
														if (this->CheckView())
															GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height);

														if (this->isStateChanged)
														{
															this->isStateChanged = FALSE;

															filterProgram = frameFilter == FilterCubic ? &shaders.cubic : &shaders.linear;
															UseShaderProgram(filterProgram, texSize);

															if (viewSize)
															{
																GLDeleteTextures(1, &tboId);
																GLDeleteRenderbuffers(1, &rboId);
																viewSize = 0;
															}

															GLBindTexture(GL_TEXTURE_2D, textureId);
															filter = frameFilter == FilterLinear ? GL_LINEAR : GL_NEAREST;
															GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
															GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
														}
													}

													// NEXT UNCHANGED
													{
														if (this->mode.bpp == 32)
															GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->mode.width, this->mode.height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, surface->indexBuffer);
														else
															GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->mode.width, this->mode.height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, surface->indexBuffer);

														// Update FPS
														if (fpsState && !this->isTakeSnapshot)
														{
															DWORD fps = fpsCounter->value;
															DWORD digCount = 0;
															DWORD current = fps;
															do
															{
																++digCount;
																current = current / 10;
															} while (current);

															if (this->mode.bpp == 32)
															{
																DWORD fpsColor = fpsState == FpsBenchmark ? 0xFFFFFF00 : 0xFFFFFFFF;
																DWORD dcount = digCount;
																do
																{
																	WORD* lpDig = (WORD*)counters[fps % 10];

																	for (DWORD y = 0; y < FPS_HEIGHT; ++y)
																	{
																		DWORD* idx = (DWORD*)surface->indexBuffer + (FPS_Y + y) * this->mode.width +
																			FPS_X + FPS_WIDTH * (dcount - 1);

																		DWORD* pix = (DWORD*)frameBuffer + y * FPS_WIDTH * 4 +
																			FPS_WIDTH * (dcount - 1);

																		WORD check = *lpDig++;
																		DWORD width = FPS_WIDTH;
																		do
																		{
																			*pix++ = (check & 1) ? fpsColor : *idx;
																			++idx;
																			check >>= 1;
																		} while (--width);
																	}

																	fps = fps / 10;
																} while (--dcount);

																dcount = 4;
																while (dcount != digCount)
																{
																	for (DWORD y = 0; y < FPS_HEIGHT; ++y)
																	{
																		DWORD* idx = (DWORD*)surface->indexBuffer + (FPS_Y + y) * this->mode.width +
																			FPS_X + FPS_WIDTH * (dcount - 1);

																		DWORD* pix = (DWORD*)frameBuffer + y * FPS_WIDTH * 4 +
																			FPS_WIDTH * (dcount - 1);

																		DWORD width = FPS_WIDTH;
																		do
																			*pix++ = *idx++;
																		while (--width);
																	}

																	--dcount;
																}

																GLTexSubImage2D(GL_TEXTURE_2D, 0, FPS_X, FPS_Y, FPS_WIDTH * 4, FPS_HEIGHT, GL_BGRA_EXT, GL_UNSIGNED_BYTE, frameBuffer);
															}
															else
															{
																WORD fpsColor = fpsState == FpsBenchmark ? 0xFFE0 : 0xFFFF;
																DWORD dcount = digCount;
																do
																{
																	WORD* lpDig = (WORD*)counters[fps % 10];

																	for (DWORD y = 0; y < FPS_HEIGHT; ++y)
																	{
																		WORD* idx = (WORD*)surface->indexBuffer + (FPS_Y + y) * this->mode.width +
																			FPS_X + FPS_WIDTH * (dcount - 1);

																		WORD* pix = (WORD*)frameBuffer + y * FPS_WIDTH * 4 +
																			FPS_WIDTH * (dcount - 1);

																		WORD check = *lpDig++;
																		DWORD width = FPS_WIDTH;
																		do
																		{
																			*pix++ = (check & 1) ? fpsColor : *idx;
																			++idx;
																			check >>= 1;
																		} while (--width);
																	}

																	fps = fps / 10;
																} while (--dcount);

																dcount = 4;
																while (dcount != digCount)
																{
																	for (DWORD y = 0; y < FPS_HEIGHT; ++y)
																	{
																		WORD* idx = (WORD*)surface->indexBuffer + (FPS_Y + y) * this->mode.width +
																			FPS_X + FPS_WIDTH * (dcount - 1);

																		WORD* pix = (WORD*)frameBuffer + y * FPS_WIDTH * 4 +
																			FPS_WIDTH * (dcount - 1);

																		DWORD width = FPS_WIDTH;
																		do
																			*pix++ = *idx++;
																		while (--width);
																	}

																	--dcount;
																}

																GLTexSubImage2D(GL_TEXTURE_2D, 0, FPS_X, FPS_Y, FPS_WIDTH * 4, FPS_HEIGHT, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, frameBuffer);
															}
														}

														// Draw into FBO texture
														GLDrawArrays(GL_TRIANGLE_FAN, 0, 4);
													}

													// Draw from FBO
													if (frameFilter == FilterXRBZ || frameFilter == FilterScaleHQ ||
														frameFilter == FilterXSal || frameFilter == FilterEagle || frameFilter == FilterScaleNx)
													{
														//GLFinish();
														GLBindFramebuffer(GL_DRAW_FRAMEBUFFER, NULL);

														UseShaderProgram(filterProgram2, viewSize);
														{
															GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height);

															GLClear(GL_COLOR_BUFFER_BIT);
															GLBindTexture(GL_TEXTURE_2D, tboId);

															filter = filterProgram2 == &shaders.linear ? GL_LINEAR : GL_NEAREST;
															GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
															GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

															GLDrawArrays(GL_TRIANGLE_FAN, 4, 4);

															if (this->isTakeSnapshot)
															{
																this->isTakeSnapshot = FALSE;

																GLFinish();

																if (OpenClipboard(NULL))
																{
																	EmptyClipboard();

																	DWORD dataSize = LOWORD(viewSize) * HIWORD(viewSize) * 3;
																	HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + dataSize);
																	{
																		VOID* data = GlobalLock(hMemory);
																		{
																			BITMAPINFOHEADER* bmiHeader = (BITMAPINFOHEADER*)data;
																			MemoryZero(bmiHeader, sizeof(BITMAPINFOHEADER));
																			bmiHeader->biSize = sizeof(BITMAPINFOHEADER);
																			bmiHeader->biWidth = LOWORD(viewSize);
																			bmiHeader->biHeight = HIWORD(viewSize);
																			bmiHeader->biPlanes = 1;
																			bmiHeader->biBitCount = 24;
																			bmiHeader->biCompression = BI_RGB;
																			bmiHeader->biXPelsPerMeter = 1;
																			bmiHeader->biYPelsPerMeter = 1;

																			VOID* pixels = (BITMAPINFOHEADER*)((BYTE*)data + sizeof(BITMAPINFOHEADER));
																			GLGetTexImage(GL_TEXTURE_2D, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
																		}
																		GlobalUnlock(hMemory);

																		SetClipboardData(CF_DIB, hMemory);
																	}
																	GlobalFree(hMemory);

																	CloseClipboard();
																}
															}
														}
														UseShaderProgram(filterProgram, texSize);
													}
													else
													{
														if (isTakeSnapshot)
														{
															this->isTakeSnapshot = FALSE;

															if (OpenClipboard(NULL))
															{
																EmptyClipboard();

																DWORD dataSize = this->mode.width * this->mode.height * sizeof(WORD);
																HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPV5HEADER) + dataSize);
																{
																	VOID* data = GlobalLock(hMemory);
																	{
																		BITMAPV5HEADER* bmi = (BITMAPV5HEADER*)data;
																		MemoryZero(bmi, sizeof(BITMAPINFOHEADER));
																		bmi->bV5Size = sizeof(BITMAPV5HEADER);
																		bmi->bV5Width = this->mode.width;
																		bmi->bV5Height = -*(LONG*)&this->mode.height;
																		bmi->bV5Planes = 1;
																		bmi->bV5BitCount = 16;
																		bmi->bV5Compression = BI_BITFIELDS;
																		bmi->bV5XPelsPerMeter = 1;
																		bmi->bV5YPelsPerMeter = 1;
																		bmi->bV5RedMask = 0xF800;
																		bmi->bV5GreenMask = 0x07E0;
																		bmi->bV5BlueMask = 0x001F;

																		MemoryCopy((BYTE*)data + sizeof(BITMAPV5HEADER), surface->indexBuffer, dataSize);
																	}
																	GlobalUnlock(hMemory);

																	SetClipboardData(CF_DIBV5, hMemory);
																}
																GlobalFree(hMemory);

																CloseClipboard();
															}
														}
													}

													// Swap
													SwapBuffers(this->hDc);
													if (fpsState != FpsBenchmark)
														WaitForSingleObject(this->hDrawEvent, INFINITE);
													if (isVSync)
														GLFinish();
												}
											} while (!this->isFinish);
										}
										delete fpsCounter;
									}
									AlignedFree(frameBuffer);
								}

								if (viewSize)
								{
									GLDeleteRenderbuffers(1, &rboId);
									GLDeleteTextures(1, &tboId);
								}
							}
							GLDeleteFramebuffers(1, &fboId);
						}
						GLDeleteTextures(1, &textureId);
					}
					GLBindBuffer(GL_ARRAY_BUFFER, NULL);
				}
				GLDeleteBuffers(1, &bufferName);
			}
			GLBindVertexArray(NULL);
		}
		GLDeleteVertexArrays(1, &arrayName);

		if (stencil)
		{
			MemoryFree(stencil);
			GLDeleteBuffers(1, &stBufferName);
			GLDeleteVertexArrays(1, &stArrayName);
		}
	}
	GLUseProgram(NULL);

	ShaderProgram* shaderProgram = (ShaderProgram*)&shaders;
	DWORD count = sizeof(shaders) / sizeof(ShaderProgram);
	do
	{
		if (shaderProgram->id)
			GLDeleteProgram(shaderProgram->id);

	} while (--count);
}

VOID OpenDraw::RenderStart()
{
	if (!this->isFinish || !this->hWnd)
		return;

	this->isFinish = FALSE;
	GL::Load();

	RECT rect;
	GetClientRect(this->hWnd, &rect);

	if (!config.windowedMode)
	{
		this->hDraw = CreateWindowEx(
			WS_EX_CONTROLPARENT | WS_EX_TOPMOST,
			WC_DRAW,
			NULL,
			WS_VISIBLE | WS_POPUP,
			rect.left, rect.top,
			rect.right - rect.left, rect.bottom - rect.top,
			this->hWnd,
			NULL,
			hDllModule,
			NULL);
	}
	else
	{
		this->hDraw = CreateWindowEx(
			WS_EX_CONTROLPARENT,
			WC_DRAW,
			NULL,
			WS_VISIBLE | WS_CHILD,
			rect.left, rect.top,
			rect.right - rect.left, rect.bottom - rect.top,
			this->hWnd,
			NULL,
			hDllModule,
			NULL);
	}

	Window::SetCapturePanel(this->hDraw);

	SetClassLongPtr(this->hDraw, GCLP_HBRBACKGROUND, NULL);
	RedrawWindow(this->hDraw, NULL, NULL, RDW_INVALIDATE);
	SetClassLongPtr(this->hWnd, GCLP_HBRBACKGROUND, NULL);
	RedrawWindow(this->hWnd, NULL, NULL, RDW_INVALIDATE);

	this->viewport.width = rect.right - rect.left;
	this->viewport.height = rect.bottom - rect.top;
	this->viewport.refresh = TRUE;

	DWORD threadId;
	SECURITY_ATTRIBUTES sAttribs;
	MemoryZero(&sAttribs, sizeof(SECURITY_ATTRIBUTES));
	sAttribs.nLength = sizeof(SECURITY_ATTRIBUTES);
	this->hDrawThread = CreateThread(&sAttribs, NULL, RenderThread, this, HIGH_PRIORITY_CLASS, &threadId);
}

VOID OpenDraw::RenderStop()
{
	if (this->isFinish)
		return;

	this->isFinish = TRUE;
	SetEvent(this->hDrawEvent);
	WaitForSingleObject(this->hDrawThread, INFINITE);
	CloseHandle(this->hDrawThread);
	this->hDrawThread = NULL;

	BOOL wasFull = GetWindowLong(this->hDraw, GWL_STYLE) & WS_POPUP;
	if (DestroyWindow(this->hDraw))
		this->hDraw = NULL;

	if (wasFull)
		GL::ResetContext();

	ClipCursor(NULL);

	glVersion = NULL;
	Window::CheckMenu(this->hWnd);
}

BOOL OpenDraw::CheckView()
{
	if (this->viewport.refresh)
	{
		this->viewport.rectangle.x = this->viewport.rectangle.y = 0;
		this->viewport.rectangle.width = this->viewport.width;
		this->viewport.rectangle.height = this->viewport.height;

		this->viewport.clipFactor.x = this->viewport.viewFactor.x = (FLOAT)this->viewport.width / this->mode.width;
		this->viewport.clipFactor.y = this->viewport.viewFactor.y = (FLOAT)this->viewport.height / this->mode.height;

		if (config.image.aspect && this->viewport.viewFactor.x != this->viewport.viewFactor.y)
		{
			if (this->viewport.viewFactor.x > this->viewport.viewFactor.y)
			{
				FLOAT fw = this->viewport.viewFactor.y * this->mode.width;
				this->viewport.rectangle.width = (INT)MathRound(fw);
				this->viewport.rectangle.x = (INT)MathRound(((FLOAT)this->viewport.width - fw) / 2.0f);
				this->viewport.clipFactor.x = this->viewport.viewFactor.y;
			}
			else
			{
				FLOAT fh = this->viewport.viewFactor.x * this->mode.height;
				this->viewport.rectangle.height = (INT)MathRound(fh);
				this->viewport.rectangle.y = (INT)MathRound(((FLOAT)this->viewport.height - fh) / 2.0f);
				this->viewport.clipFactor.y = this->viewport.viewFactor.x;
			}
		}

		if (config.image.aspect && !config.windowedMode)
		{
			RECT clipRect;
			GetClientRect(this->hWnd, &clipRect);

			clipRect.left = this->viewport.rectangle.x;
			clipRect.right = clipRect.left + this->viewport.rectangle.width;
			clipRect.bottom = clipRect.bottom - this->viewport.rectangle.y;
			clipRect.top = clipRect.bottom - this->viewport.rectangle.height;

			ClientToScreen(this->hWnd, (POINT*)&clipRect.left);
			ClientToScreen(this->hWnd, (POINT*)&clipRect.right);

			ClipCursor(&clipRect);
		}
		else
			ClipCursor(NULL);

		this->clearStage = 0;
	}

	if (++this->clearStage <= 2)
		GLClear(GL_COLOR_BUFFER_BIT);

	if (this->viewport.refresh)
	{
		this->viewport.refresh = FALSE;
		return TRUE;
	}

	return FALSE;
}

VOID OpenDraw::ScaleMouse(LPPOINT p)
{
	p->x = (LONG)((FLOAT)((p->x - this->viewport.rectangle.x) * this->mode.width) / this->viewport.rectangle.width);
	p->y = (LONG)((FLOAT)((p->y - this->viewport.rectangle.y) * this->mode.height) / this->viewport.rectangle.height);
}

OpenDraw::OpenDraw(IDrawUnknown** list)
{
	this->refCount = 1;
	this->list = list;
	this->last = *list;
	*list = this;

	this->surfaceEntries = NULL;
	this->clipperEntries = NULL;
	this->paletteEntries = NULL;

	this->attachedSurface = NULL;

	this->hWnd = NULL;
	this->hDraw = NULL;
	this->hDc = NULL;

	this->mode = *config.mode;

	this->isTakeSnapshot = FALSE;
	this->isFinish = TRUE;

	MemoryZero(&this->windowPlacement, sizeof(WINDOWPLACEMENT));
	this->hDrawEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

OpenDraw::~OpenDraw()
{
	IDrawDestruct(this);
}

VOID OpenDraw::SetFullscreenMode()
{
	if (config.windowedMode)
		GetWindowPlacement(hWnd, &this->windowPlacement);

	SetMenu(hWnd, NULL);

	SetWindowLong(this->hWnd, GWL_STYLE, WS_FULLSCREEN);
	SetWindowPos(this->hWnd, NULL, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL);
}

VOID OpenDraw::SetWindowedMode()
{
	if (!this->windowPlacement.length)
	{
		this->windowPlacement.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(hWnd, &this->windowPlacement);

		INT monWidth = GetSystemMetrics(SM_CXSCREEN);
		INT monHeight = GetSystemMetrics(SM_CYSCREEN);

		RECT* rect = &this->windowPlacement.rcNormalPosition;
		rect->left = (monWidth - this->mode.width) >> 1;
		rect->top = (monHeight - this->mode.height) >> 1;
		rect->right = rect->left + this->mode.width;
		rect->bottom = rect->top + this->mode.height;
		AdjustWindowRect(rect, WS_WINDOWED, TRUE);

		if (rect->right - rect->left > monWidth)
		{
			rect->left = 0;
			rect->right = monWidth;
		}

		if (rect->bottom - rect->top > monHeight)
		{
			rect->top = 0;
			rect->bottom = monHeight;
		}

		this->windowPlacement.ptMinPosition.x = this->windowPlacement.ptMinPosition.y = -1;
		this->windowPlacement.ptMaxPosition.x = this->windowPlacement.ptMaxPosition.y = -1;

		this->windowPlacement.flags = NULL;
		this->windowPlacement.showCmd = SW_SHOWNORMAL;
	}

	SetMenu(hWnd, config.menu);

	SetWindowLong(this->hWnd, GWL_STYLE, WS_WINDOWED);
	SetWindowPlacement(this->hWnd, &this->windowPlacement);
}

HRESULT __stdcall OpenDraw::QueryInterface(REFIID id, LPVOID* lpObj)
{
	if (id == IID_IDirectDraw7)
		*lpObj = new OpenDraw((IDrawUnknown**)&drawList);
	else
		Main::ShowError("QueryInterface required", __FILE__, __LINE__);
	return DD_OK;
}

ULONG __stdcall OpenDraw::Release()
{
	ULONG count = --this->refCount;
	if (!count)
	{
		this->RenderStop();

		CloseHandle(this->hDrawEvent);
		ClipCursor(NULL);

		delete this;
	}

	return count;
}

HRESULT __stdcall OpenDraw::SetCooperativeLevel(HWND hWnd, DWORD dwFlags)
{
	this->hWnd = hWnd;

	if (dwFlags & DDSCL_NORMAL)
	{
		this->SetWindowedMode();
		this->RenderStart();
	}

	return DD_OK;
}

HRESULT __stdcall OpenDraw::SetDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP, DWORD dwRefreshRate, DWORD dwFlags)
{
	this->mode.width = dwWidth;
	this->mode.height = dwHeight;
	this->mode.bpp = dwBPP;

	this->SetFullscreenMode();
	this->RenderStart();

	return DD_OK;
}

HRESULT __stdcall OpenDraw::CreateSurface(LPDDSURFACEDESC2 lpDDSurfaceDesc, IDrawSurface7** lplpDDSurface, IUnknown* pUnkOuter)
{
	// 33 - DDSD_BACKBUFFERCOUNT | DDSD_CAPS // 536 - DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX
	// 4103 - DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS // 2112 - DDSCAPS_SYSTEMMEMORY | DDSCAPS_OFFSCREENPLAIN
	// 7 - DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS // 64 - DDSCAPS_OFFSCREENPLAIN

	OpenDrawSurface* surface;

	if (lpDDSurfaceDesc->ddsCaps.dwCaps == DDSCAPS_OFFSCREENPLAIN)
	{
		surface = new OpenDrawSurface((IDrawUnknown**)&this->surfaceEntries, this, 1, &lpDDSurfaceDesc->ddsCaps);
		surface->AddRef();
		surface->CreateBuffer(this->mode.width, this->mode.height, this->mode.bpp);
		this->attachedSurface->attachedSurface = surface;
	}
	else
	{
		BOOL isPrimary = lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE;
		surface = new OpenDrawSurface((IDrawUnknown**)&this->surfaceEntries, this, !isPrimary, &lpDDSurfaceDesc->ddsCaps);

		if (isPrimary)
			this->attachedSurface = surface;

		if (lpDDSurfaceDesc->dwFlags & (DDSD_WIDTH | DDSD_HEIGHT))
		{
			if (lpDDSurfaceDesc->dwFlags & DDSD_PIXELFORMAT)
				surface->CreateBuffer(lpDDSurfaceDesc->dwWidth, lpDDSurfaceDesc->dwHeight, lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount);
			else
				surface->CreateBuffer(lpDDSurfaceDesc->dwWidth, lpDDSurfaceDesc->dwHeight, this->mode.bpp);
		}
		else
			surface->CreateBuffer(this->mode.width, this->mode.height, this->mode.bpp);
	}

	*lplpDDSurface = surface;

	return DD_OK;
}

HRESULT __stdcall OpenDraw::CreateClipper(DWORD dwFlags, IDrawClipper** lplpDDClipper, IUnknown* pUnkOuter)
{
	OpenDrawClipper* clipper = new OpenDrawClipper((IDrawUnknown**)&this->clipperEntries, this);
	*lplpDDClipper = clipper;

	return DD_OK;
}

HRESULT __stdcall OpenDraw::EnumDisplayModes(DWORD dwFlags, LPDDSURFACEDESC2 lpDDSurfaceDesc, LPVOID lpContext, LPDDENUMMODESCALLBACK2 lpEnumModesCallback)
{
	DDSURFACEDESC2 ddSurfaceDesc;
	MemoryZero(&ddSurfaceDesc, sizeof(DDSURFACEDESC2));

	DisplayMode* mode = modesList;
	DWORD count = sizeof(modesList) / sizeof(DisplayMode);
	do
	{
		ddSurfaceDesc.dwWidth = mode->width;
		ddSurfaceDesc.dwHeight = mode->height;

		ddSurfaceDesc.ddpfPixelFormat.dwFlags = DDSD_ZBUFFERBITDEPTH;
		ddSurfaceDesc.ddpfPixelFormat.dwRGBBitCount = mode->bpp;

		if (!lpEnumModesCallback(&ddSurfaceDesc, lpContext))
			break;

		++mode;
	} while (--count);

	return DD_OK;
}

HRESULT __stdcall OpenDraw::GetDisplayMode(LPDDSURFACEDESC2 lpDDSurfaceDesc)
{
	lpDDSurfaceDesc->dwWidth = this->mode.width;
	lpDDSurfaceDesc->dwHeight = this->mode.height;
	lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount = this->mode.bpp;
	return DD_OK;
}

HRESULT __stdcall OpenDraw::CreatePalette(DWORD dwFlags, LPPALETTEENTRY lpDDColorArray, IDrawPalette** lplpDDPalette, IUnknown* pUnkOuter)
{
	OpenDrawPalette* palette = new OpenDrawPalette((IDrawUnknown**)&this->paletteEntries, this);
	*lplpDDPalette = palette;
	MemoryCopy(palette->entries, lpDDColorArray, 256 * sizeof(PALETTEENTRY));

	return DD_OK;
}