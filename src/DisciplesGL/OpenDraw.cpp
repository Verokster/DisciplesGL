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
		GLUniform1i(GLGetUniformLocation(program->id, "tex02"), 1);

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
			GLUniform2f(program->texSize.location, (FLOAT)LOWORD(texSize), (FLOAT)HIWORD(texSize));
		}
	}
}

VOID OpenDraw::TakeSnapshot(VOID* buffer, DWORD frameWidth, DWORD frameHeight)
{
	this->isTakeSnapshot = FALSE;

	if (OpenClipboard(NULL))
	{
		EmptyClipboard();

		BOOL isTrueColor = config.mode->bpp != 16 || config.bpp32Hooked;

		DWORD pitch = frameWidth * (isTrueColor ? 3 : 2);
		if (pitch & 3)
			pitch = (pitch & 0xFFFFFFFC) + 4;

		DWORD infoSize = sizeof(BITMAPINFOHEADER);
		if (!isTrueColor)
			infoSize += sizeof(DWORD) * 3;

		DWORD dataSize = pitch * frameHeight;
		HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, infoSize + dataSize);
		{
			VOID* data = GlobalLock(hMemory);
			{
				BITMAPV4HEADER* bmiHeader = (BITMAPV4HEADER*)data;
				bmiHeader->bV4Size = sizeof(BITMAPINFOHEADER);
				bmiHeader->bV4Width = frameWidth;
				bmiHeader->bV4Height = frameHeight;
				bmiHeader->bV4Planes = 1;
				bmiHeader->bV4SizeImage = dataSize;
				bmiHeader->bV4XPelsPerMeter = 1;
				bmiHeader->bV4YPelsPerMeter = 1;
				bmiHeader->bV4ClrUsed = 0;
				bmiHeader->bV4ClrImportant = 0;

				if (isTrueColor)
				{
					bmiHeader->bV4BitCount = 24;
					bmiHeader->bV4V4Compression = BI_RGB;

					DWORD* src = (DWORD*)buffer;
					BYTE* dstData = (BYTE*)data + infoSize + dataSize - pitch;

					DWORD height = frameHeight;
					do
					{
						BYTE* dst = dstData;

						DWORD width = frameWidth;
						do
						{
							DWORD color = _byteswap_ulong(*src++);
							BYTE* px = (BYTE*)&color;

							*dst++ = *++px;
							*dst++ = *++px;
							*dst++ = *++px;
						} while (--width);

						dstData -= pitch;
					} while (--height);
				}
				else
				{
					bmiHeader->bV4BitCount = 16;
					bmiHeader->bV4V4Compression = BI_BITFIELDS;
					bmiHeader->bV4RedMask = 0xF800;
					bmiHeader->bV4GreenMask = 0x07E0;
					bmiHeader->bV4BlueMask = 0x001F;

					WORD* src = (WORD*)buffer;
					BYTE* dstData = (BYTE*)data + infoSize + dataSize - pitch;

					DWORD height = frameHeight;
					do
					{
						BYTE* dst = dstData;

						DWORD width = frameWidth;
						do
						{
							*(WORD*)dst = *src++;
							dst += 2;
						} while (--width);

						dstData -= pitch;
					} while (--height);
				}
			}
			GlobalUnlock(hMemory);
			SetClipboardData(CF_DIB, hMemory);
		}
		GlobalFree(hMemory);

		CloseClipboard();
	}
}

DWORD __stdcall RenderThread(LPVOID lpParameter)
{
	OpenDraw* ddraw = (OpenDraw*)lpParameter;
	ddraw->hDc = ::GetDC(ddraw->hDraw);
	if (ddraw->hDc)
	{
		PIXELFORMATDESCRIPTOR pfd;
		INT glPixelFormat = GL::PreparePixelFormat(&pfd);
		if (!glPixelFormat)
		{
			glPixelFormat = ::ChoosePixelFormat(ddraw->hDc, &pfd);
			if (!glPixelFormat)
				Main::ShowError(IDS_ERROR_CHOOSE_PF, __FILE__, __LINE__);
			else if (pfd.dwFlags & PFD_NEED_PALETTE)
				Main::ShowError(IDS_ERROR_NEED_PALETTE, __FILE__, __LINE__);
		}

		if (!::SetPixelFormat(ddraw->hDc, glPixelFormat, &pfd))
			Main::ShowError(IDS_ERROR_SET_PF, __FILE__, __LINE__);

		GL::ResetPixelFormatDescription(&pfd);
		if (::DescribePixelFormat(ddraw->hDc, glPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd) == NULL)
			Main::ShowError(IDS_ERROR_DESCRIBE_PF, __FILE__, __LINE__);

		if ((pfd.iPixelType != PFD_TYPE_RGBA) || (pfd.cRedBits < 5) || (pfd.cGreenBits < 6) || (pfd.cBlueBits < 5))
			Main::ShowError(IDS_ERROR_BAD_PF, __FILE__, __LINE__);

		HGLRC hRc = wglCreateContext(ddraw->hDc);
		if (hRc)
		{
			if (wglMakeCurrent(ddraw->hDc, hRc))
			{
				GL::CreateContextAttribs(ddraw->hDc, &hRc);
				if (glVersion >= GL_VER_2_0)
				{
					DWORD maxSize = config.mode->width > config.mode->height ? config.mode->width : config.mode->height;

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

				wglMakeCurrent(ddraw->hDc, NULL);
			}

			wglDeleteContext(hRc);
		}

		::ReleaseDC(ddraw->hDraw, ddraw->hDc);
		ddraw->hDc = NULL;
	}

	return NULL;
}

VOID OpenDraw::RenderOld()
{
	BOOL isTrueColor = config.mode->bpp != 16 || config.bpp32Hooked;

	if (config.image.filter > FilterLinear)
		config.image.filter = FilterLinear;

	DWORD glMaxTexSize;
	GLGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&glMaxTexSize);
	if (glMaxTexSize < 256)
		glMaxTexSize = 256;

	DWORD size = config.mode->width > config.mode->height ? config.mode->width : config.mode->height;
	DWORD maxAllow = 1;
	while (maxAllow < size)
		maxAllow <<= 1;

	DWORD maxTexSize = maxAllow < glMaxTexSize ? maxAllow : glMaxTexSize;
	DWORD glFilter = config.image.filter == FilterNearest ? GL_NEAREST : GL_LINEAR;

	DWORD framePerWidth = config.mode->width / maxTexSize + (config.mode->width % maxTexSize ? 1 : 0);
	DWORD framePerHeight = config.mode->height / maxTexSize + (config.mode->height % maxTexSize ? 1 : 0);
	DWORD frameCount = framePerWidth * framePerHeight;

	FLOAT tw, th;
	DWORD zWidth, zHeight;

	if (frameCount == 1)
	{
		FLOAT k = (FLOAT)config.mode->width / config.mode->height;
		if (k >= 4.0f / 3.0f)
		{
			th = GAME_HEIGHT_FLOAT / maxTexSize;
			tw = th * k;

			zWidth = DWORD(GAME_HEIGHT_FLOAT * k);
			zHeight = GAME_HEIGHT;
		}
		else
		{
			k = (FLOAT)config.mode->height / config.mode->width;

			tw = GAME_WIDTH_FLOAT / maxTexSize;
			th = tw * config.mode->height / config.mode->width;

			zWidth = GAME_WIDTH;
			zHeight = DWORD(GAME_WIDTH_FLOAT * config.mode->height / config.mode->width);
		}
	}
	else if (config.zoomable)
	{
		config.zoomable = FALSE;
		config.menuZoomImage = FALSE;
		config.zoomImage = FALSE;
		Window::CheckMenu(this->hWnd);
	}

	Frame* frames = (Frame*)MemoryAlloc(frameCount * sizeof(Frame));
	{
		Frame* frame = frames;
		for (DWORD y = 0; y < config.mode->height; y += maxTexSize)
		{
			DWORD height = config.mode->height - y;
			if (height > maxTexSize)
				height = maxTexSize;

			for (DWORD x = 0; x < config.mode->width; x += maxTexSize, ++frame)
			{
				DWORD width = config.mode->width - x;
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

				if (!isTrueColor && glVersion > GL_VER_1_1)
					GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
				else
					GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			}
		}

		GLMatrixMode(GL_PROJECTION);
		GLLoadIdentity();
		GLOrtho(0.0, (GLdouble)config.mode->width, (GLdouble)config.mode->height, 0.0, 0.0, 1.0);
		GLMatrixMode(GL_MODELVIEW);
		GLLoadIdentity();

		GLEnable(GL_TEXTURE_2D);
		GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		this->viewport.refresh = TRUE;

		VOID* frameBuffer = AlignedAlloc(maxTexSize * maxTexSize * (!isTrueColor && glVersion > GL_VER_1_1 ? sizeof(WORD) : sizeof(DWORD)));
		{
			FpsCounter* fpsCounter = new FpsCounter(FPS_ACCURACY);
			{
				BOOL isVSync = config.image.vSync;
				if (WGLSwapInterval)
					WGLSwapInterval(isVSync);

				while (!this->isFinish)
				{
					OpenDrawSurface* surface = this->attachedSurface;
					if (surface)
					{
						BOOL isZoomed = surface->isZoomed[surface->bufferIndex];
						VOID* currentBuffer;
						if (config.version)
							currentBuffer = surface->indexBuffer;
						else
						{
							surface->bufferIndex = !surface->bufferIndex;
							currentBuffer = surface->bufferIndex ? surface->indexBuffer : surface->secondaryBuffer;

							if (surface->drawIndex)
								--surface->drawIndex;
						}

						if (isVSync != config.image.vSync)
						{
							isVSync = config.image.vSync;
							if (WGLSwapInterval)
								WGLSwapInterval(isVSync);
						}

						if (fpsState)
							fpsCounter->Calculate();

						if (this->CheckView(TRUE))
							GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y + this->viewport.offset, this->viewport.rectangle.width, this->viewport.rectangle.height);

						glFilter = 0;
						if (this->isStateChanged)
						{
							this->isStateChanged = FALSE;
							glFilter = config.image.filter == FilterNearest ? GL_NEAREST : GL_LINEAR;
						}

						DWORD frameWidth, frameHeight;
						if (isZoomed)
						{
							frameWidth = zWidth;
							frameHeight = zHeight;
						}
						else
						{
							frameWidth = config.mode->width;
							frameHeight = config.mode->height;
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

								if (isTrueColor)
									GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameWidth, frameHeight, GL_RGBA, GL_UNSIGNED_BYTE, currentBuffer);
								else
								{
									if (glVersion > GL_VER_1_1)
										GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameWidth, frameHeight, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, currentBuffer);
									else
									{
										WORD* source = (WORD*)currentBuffer;
										DWORD* dest = (DWORD*)frameBuffer;
										DWORD copyWidth = frameWidth;
										DWORD copyHeight = frameHeight;
										do
										{
											WORD* src = source;
											source += frameWidth;

											DWORD count = copyWidth;
											do
											{
												WORD px = *src++;
												*dest++ = ((px & 0xF800) >> 8) | ((px & 0x07E0) << 5) | ((px & 0x001F) << 19);
											} while (--count);
										} while (--copyHeight);

										GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameWidth, frameHeight, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
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

								if (isTrueColor)
								{
									DWORD* source = (DWORD*)currentBuffer + frame->rect.y * frameWidth + frame->rect.x;
									DWORD* dest = (DWORD*)frameBuffer;
									DWORD copyHeight = frame->rect.height;
									do
									{
										MemoryCopy(dest, source, frame->rect.width << 2);
										source += frameWidth;
										dest += frame->rect.width;
									} while (--copyHeight);

									GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->rect.width, frame->rect.height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
								}
								else
								{
									if (glVersion > GL_VER_1_1)
									{
										WORD* source = (WORD*)currentBuffer + frame->rect.y * frameWidth + frame->rect.x;
										WORD* dest = (WORD*)frameBuffer;
										DWORD copyHeight = frame->rect.height;
										do
										{
											MemoryCopy(dest, source, frame->rect.width << 1);
											source += frameWidth;
											dest += frame->rect.width;
										} while (--copyHeight);

										GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->rect.width, frame->rect.height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, frameBuffer);
									}
									else
									{
										WORD* source = (WORD*)currentBuffer + frame->rect.y * frameWidth + frame->rect.x;
										DWORD* dest = (DWORD*)frameBuffer;
										DWORD copyHeight = frame->rect.height;
										do
										{
											WORD* src = source;
											source += frameWidth;

											DWORD count = frame->rect.width;
											do
											{
												WORD px = *src++;
												*dest++ = ((px & 0xF800) >> 8) | ((px & 0x07E0) << 5) | ((px & 0x001F) << 19);
											} while (--count);
										} while (--copyHeight);

										GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->rect.width, frame->rect.height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
									}
								}
							}

							if (fpsState && !this->isTakeSnapshot && frame == frames)
								fpsCounter->Draw(currentBuffer, frameBuffer, frameWidth, frameHeight);

							GLBegin(GL_TRIANGLE_FAN);
							{
								if (!isZoomed)
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

						if (this->isTakeSnapshot)
							this->TakeSnapshot(currentBuffer, frameWidth, frameHeight);

						SwapBuffers(this->hDc);
						GLFinish();

						if (fpsState != FpsBenchmark || !config.version)
							WaitForSingleObject(this->hDrawEvent, INFINITE);
						else
							Sleep(0);
					}
				}
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
	BOOL isTrueColor = config.mode->bpp != 16 || config.bpp32Hooked;

	if (config.image.filter > FilterCubic)
		config.image.filter = FilterLinear;

	DWORD maxSize = config.mode->width > config.mode->height ? config.mode->width : config.mode->height;
	DWORD maxTexSize = 1;
	while (maxTexSize < maxSize)
		maxTexSize <<= 1;
	FLOAT texWidth = config.mode->width == maxTexSize ? 1.0f : (FLOAT)config.mode->width / maxTexSize;
	FLOAT texHeight = config.mode->height == maxTexSize ? 1.0f : (FLOAT)config.mode->height / maxTexSize;

	DWORD texSize = (maxTexSize & 0xFFFF) | (maxTexSize << 16);

	FLOAT tw, th;
	DWORD zWidth, zHeight;

	FLOAT k = (FLOAT)config.mode->width / config.mode->height;
	if (k >= 4.0f / 3.0f)
	{
		th = GAME_HEIGHT_FLOAT / maxTexSize;
		tw = th * k;

		zWidth = DWORD(GAME_HEIGHT_FLOAT * k);
		zHeight = GAME_HEIGHT;
	}
	else
	{
		tw = GAME_WIDTH_FLOAT / maxTexSize;
		th = tw * config.mode->height / config.mode->width;

		zWidth = GAME_WIDTH;
		zHeight = DWORD(GAME_WIDTH_FLOAT * config.mode->height / config.mode->width);
	}

	FLOAT buffer[8][4] = {
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ (FLOAT)config.mode->width, 0.0f, texWidth, 0.0f },
		{ (FLOAT)config.mode->width, (FLOAT)config.mode->height, texWidth, texHeight },
		{ 0.0f, (FLOAT)config.mode->height, 0.0f, texHeight },

		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ (FLOAT)config.mode->width, 0.0f, tw, 0.0f },
		{ (FLOAT)config.mode->width, (FLOAT)config.mode->height, tw, th },
		{ 0.0f, (FLOAT)config.mode->height, 0.0f, th },
	};

	FLOAT mvpMatrix[4][4] = {
		{ FLOAT(2.0f / config.mode->width), 0.0f, 0.0f, 0.0f },
		{ 0.0f, FLOAT(-2.0f / config.mode->height), 0.0f, 0.0f },
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
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

					if (isTrueColor)
						GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
					else
						GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);

					GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);
					this->viewport.refresh = TRUE;

					VOID* frameBuffer = AlignedAlloc(config.mode->width * config.mode->height * (isTrueColor ? sizeof(DWORD) : sizeof(WORD)));
					{
						FpsCounter* fpsCounter = new FpsCounter(FPS_ACCURACY);
						{
							this->isStateChanged = TRUE;

							BOOL isVSync = config.image.vSync;
							if (WGLSwapInterval)
								WGLSwapInterval(isVSync);

							while (!this->isFinish)
							{
								OpenDrawSurface* surface = this->attachedSurface;
								if (surface)
								{
									BOOL isZoomed = surface->isZoomed[surface->bufferIndex];
									VOID* currentBuffer;
									if (config.version)
										currentBuffer = surface->indexBuffer;
									else
									{
										surface->bufferIndex = !surface->bufferIndex;
										currentBuffer = surface->bufferIndex ? surface->indexBuffer : surface->secondaryBuffer;

										if (surface->drawIndex)
											--surface->drawIndex;
									}

									if (isVSync != config.image.vSync)
									{
										isVSync = config.image.vSync;
										if (WGLSwapInterval)
											WGLSwapInterval(isVSync);
									}

									if (fpsState)
										fpsCounter->Calculate();

									if (this->CheckView(TRUE))
										GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y + this->viewport.offset, this->viewport.rectangle.width, this->viewport.rectangle.height);

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

									DWORD frameWidth, frameHeight;
									if (isZoomed)
									{
										frameWidth = zWidth;
										frameHeight = zHeight;
									}
									else
									{
										frameWidth = config.mode->width;
										frameHeight = config.mode->height;
									}

									// NEXT UNCHANGED
									{
										if (isTrueColor)
											GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameWidth, frameHeight, GL_RGBA, GL_UNSIGNED_BYTE, currentBuffer);
										else
											GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameWidth, frameHeight, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, currentBuffer);

										// Update FPS
										if (fpsState && !this->isTakeSnapshot)
											fpsCounter->Draw(currentBuffer, frameBuffer, frameWidth, frameHeight);

										GLDrawArrays(GL_TRIANGLE_FAN, isZoomed ? 4 : 0, 4);
									}

									if (this->isTakeSnapshot)
										this->TakeSnapshot(currentBuffer, frameWidth, frameHeight);

									SwapBuffers(this->hDc);
									GLFinish();

									if (fpsState != FpsBenchmark || !config.version)
										WaitForSingleObject(this->hDrawEvent, INFINITE);
									else
										Sleep(0);
								}
							}
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
	BOOL isTrueColor = config.mode->bpp != 16 || config.bpp32Hooked;

	DWORD maxSize = config.mode->width > config.mode->height ? config.mode->width : config.mode->height;
	DWORD maxTexSize = 1;
	while (maxTexSize < maxSize)
		maxTexSize <<= 1;
	FLOAT texWidth = config.mode->width == maxTexSize ? 1.0f : (FLOAT)config.mode->width / maxTexSize;
	FLOAT texHeight = config.mode->height == maxTexSize ? 1.0f : (FLOAT)config.mode->height / maxTexSize;

	DWORD texSize = (maxTexSize & 0xFFFF) | (maxTexSize << 16);

	FLOAT tw, th, tk;
	DWORD zWidth, zHeight;

	FLOAT k = (FLOAT)config.mode->width / config.mode->height;
	if (k >= 4.0f / 3.0f)
	{
		tk = GAME_HEIGHT_FLOAT / config.mode->height;
		th = GAME_HEIGHT_FLOAT / maxTexSize;
		tw = th * k;

		zWidth = DWORD(GAME_HEIGHT_FLOAT * k);
		zHeight = GAME_HEIGHT;
	}
	else
	{
		tk = GAME_WIDTH_FLOAT / config.mode->width;
		tw = GAME_WIDTH_FLOAT / maxTexSize;
		th = tw * config.mode->height / config.mode->width;

		zWidth = GAME_WIDTH;
		zHeight = DWORD(GAME_WIDTH_FLOAT * config.mode->height / config.mode->width);
	}

	FLOAT buffer[16][4] = {
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ (FLOAT)config.mode->width, 0.0f, texWidth, 0.0f },
		{ (FLOAT)config.mode->width, (FLOAT)config.mode->height, texWidth, texHeight },
		{ 0.0f, (FLOAT)config.mode->height, 0.0f, texHeight },

		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ (FLOAT)config.mode->width, 0.0f, tw, 0.0f },
		{ (FLOAT)config.mode->width, (FLOAT)config.mode->height, tw, th },
		{ 0.0f, (FLOAT)config.mode->height, 0.0f, th },

		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ (FLOAT)config.mode->width, 0.0f, 1.0f, 1.0f },
		{ (FLOAT)config.mode->width, (FLOAT)config.mode->height, 1.0f, 0.0f },
		{ 0.0f, (FLOAT)config.mode->height, 0.0f, 0.0f },

		{ 0.0f, 0.0f, 0.0f, tk },
		{ (FLOAT)config.mode->width, 0.0f, tk, tk },
		{ (FLOAT)config.mode->width, (FLOAT)config.mode->height, tk, 0.0f },
		{ 0.0f, (FLOAT)config.mode->height, 0.0f, 0.0f }
	};

	FLOAT mvpMatrix[4][4] = {
		{ FLOAT(2.0f / config.mode->width), 0.0f, 0.0f, 0.0f },
		{ 0.0f, FLOAT(-2.0f / config.mode->height), 0.0f, 0.0f },
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

						GLuint textureId[2];
						GLGenTextures(2, textureId);
						{
							DWORD idx = 2;
							while (idx--)
							{
								GLActiveTexture(GL_TEXTURE0 + idx);
								GLBindTexture(GL_TEXTURE_2D, textureId[idx]);

								GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
								GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
								GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
								GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
								GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
								GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

								if (isTrueColor)
									GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
								else
									GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
							}

							GLuint fboId;
							GLGenFramebuffers(1, &fboId);
							{
								DWORD viewSize = 0;
								GLuint rboId = 0, tboId = 0;
								{
									GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);
									this->viewport.refresh = TRUE;

									VOID* frameBuffer = AlignedAlloc(config.mode->width * config.mode->height * (isTrueColor ? sizeof(DWORD) : sizeof(WORD)));
									{
										FpsCounter* fpsCounter = new FpsCounter(FPS_ACCURACY);
										{
											BOOL activeIndex = TRUE;
											BOOL oldZoomed = FALSE;
											this->isStateChanged = TRUE;

											BOOL isVSync = config.image.vSync;
											if (WGLSwapInterval)
												WGLSwapInterval(isVSync);

											while (!this->isFinish)
											{
												OpenDrawSurface* surface = this->attachedSurface;
												if (surface)
												{
													BOOL isZoomed = surface->isZoomed[surface->bufferIndex];
													VOID* currentBuffer;
													if (config.version)
														currentBuffer = surface->indexBuffer;
													else
													{
														surface->bufferIndex = !surface->bufferIndex;
														currentBuffer = surface->bufferIndex ? surface->indexBuffer : surface->secondaryBuffer;

														if (surface->drawIndex)
															--surface->drawIndex;
													}

													if (isVSync != config.image.vSync)
													{
														isVSync = config.image.vSync;
														if (WGLSwapInterval)
															WGLSwapInterval(isVSync);
													}

													if (this->isStateChanged)
														this->viewport.refresh = TRUE;

													if (oldZoomed != isZoomed)
													{
														oldZoomed = isZoomed;
														this->viewport.refresh = TRUE;
													}

													if (fpsState)
														fpsCounter->Calculate();

													DWORD frameWidth, frameHeight;
													if (isZoomed)
													{
														frameWidth = zWidth;
														frameHeight = zHeight;
													}
													else
													{
														frameWidth = config.mode->width;
														frameHeight = config.mode->height;
													}

													ImageFilter frameFilter = config.image.filter;
													if (frameFilter == FilterXRBZ || frameFilter == FilterScaleHQ || frameFilter == FilterXSal || frameFilter == FilterEagle || frameFilter == FilterScaleNx)
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

															DWORD newSize = MAKELONG(config.mode->width * scaleValue, config.mode->height * scaleValue);
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
																GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
																GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
																GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
																GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
																GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
																GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
																GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, LOWORD(viewSize), HIWORD(viewSize), GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

																// Get storage
																GLBindRenderbuffer(GL_RENDERBUFFER, rboId);
																GLRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, LOWORD(viewSize), HIWORD(viewSize));
																GLBindRenderbuffer(GL_RENDERBUFFER, NULL);

																GLFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tboId, 0);
																GLFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboId);
															}
														}

														if (isZoomed)
															GLViewport(0, 0, DWORD(tk * LOWORD(viewSize)), DWORD(tk * HIWORD(viewSize)));
														else
															GLViewport(0, 0, LOWORD(viewSize), HIWORD(viewSize));

														GLActiveTexture(GL_TEXTURE1);
														GLBindTexture(GL_TEXTURE_2D, textureId[activeIndex]);
														GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
														GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

														if (this->CheckView(FALSE))
														{
															MemoryZero(frameBuffer, frameWidth * frameHeight * (isTrueColor ? sizeof(DWORD) : sizeof(WORD)));
															if (isTrueColor)
																GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameWidth, frameHeight, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
															else
																GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameWidth, frameHeight, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, frameBuffer);
														}

														activeIndex = !activeIndex;
														GLActiveTexture(GL_TEXTURE0);
														GLBindTexture(GL_TEXTURE_2D, textureId[activeIndex]);
														GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
														GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
													}
													else
													{
														if (this->CheckView(TRUE))
															GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y + this->viewport.offset, this->viewport.rectangle.width, this->viewport.rectangle.height);

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

															GLBindTexture(GL_TEXTURE_2D, *textureId);
															DWORD filter = frameFilter == FilterLinear ? GL_LINEAR : GL_NEAREST;
															GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
															GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
														}
													}

													// NEXT UNCHANGED
													{
														if (isTrueColor)
															GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameWidth, frameHeight, GL_RGBA, GL_UNSIGNED_BYTE, currentBuffer);
														else
															GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameWidth, frameHeight, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, currentBuffer);

														// Update FPS
														if (fpsState && !this->isTakeSnapshot)
															fpsCounter->Draw(currentBuffer, frameBuffer, frameWidth, frameHeight);

														// Draw into FBO texture
														GLDrawArrays(GL_TRIANGLE_FAN, isZoomed ? 4 : 0, 4);
													}

													// Draw from FBO
													if (frameFilter == FilterXRBZ || frameFilter == FilterScaleHQ || frameFilter == FilterXSal || frameFilter == FilterEagle || frameFilter == FilterScaleNx)
													{
														GLFinish();
														GLBindFramebuffer(GL_DRAW_FRAMEBUFFER, NULL);

														UseShaderProgram(filterProgram2, viewSize);
														{
															GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y + this->viewport.offset, this->viewport.rectangle.width, this->viewport.rectangle.height);

															GLClear(GL_COLOR_BUFFER_BIT);
															GLBindTexture(GL_TEXTURE_2D, tboId);

															DWORD filter = filterProgram2 == &shaders.linear ? GL_LINEAR : GL_NEAREST;
															GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
															GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

															GLDrawArrays(GL_TRIANGLE_FAN, isZoomed ? 12 : 8, 4);

															if (this->isTakeSnapshot)
															{
																this->isTakeSnapshot = FALSE;

																GLFinish();

																if (OpenClipboard(NULL))
																{
																	EmptyClipboard();

																	DWORD viewWidth = LOWORD(viewSize);
																	DWORD viewHeight = HIWORD(viewSize);
																	if (isZoomed)
																	{
																		viewWidth = DWORD(tk * viewWidth);
																		viewHeight = DWORD(tk * viewHeight);
																	}

																	DWORD pitch = viewWidth * 3;
																	if (pitch & 3)
																		pitch = (pitch & 0xFFFFFFFC) + 4;

																	DWORD dataSize = pitch * viewHeight;
																	HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + dataSize);
																	{
																		VOID* data = GlobalLock(hMemory);
																		{
																			BITMAPINFOHEADER* bmiHeader = (BITMAPINFOHEADER*)data;
																			bmiHeader->biSize = sizeof(BITMAPINFOHEADER);
																			bmiHeader->biWidth = viewWidth;
																			bmiHeader->biHeight = viewHeight;
																			bmiHeader->biPlanes = 1;
																			bmiHeader->biBitCount = 24;
																			bmiHeader->biCompression = BI_RGB;
																			bmiHeader->biSizeImage = dataSize;
																			bmiHeader->biXPelsPerMeter = 1;
																			bmiHeader->biYPelsPerMeter = 1;
																			bmiHeader->biClrUsed = 0;
																			bmiHeader->biClrImportant = 0;

																			VOID* pixels = (BYTE*)data + sizeof(BITMAPINFOHEADER);

																			if (!isZoomed)
																				GLGetTexImage(GL_TEXTURE_2D, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
																			else
																			{
																				DWORD bufferPitch = LOWORD(viewSize) * 3;
																				if (bufferPitch & 3)
																					bufferPitch = (bufferPitch & 0xFFFFFFFC) + 4;

																				DWORD bufferSize = bufferPitch * HIWORD(viewSize);
																				VOID* buffer = MemoryAlloc(bufferSize);
																				{
																					GLGetTexImage(GL_TEXTURE_2D, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, buffer);

																					BYTE* srcData = (BYTE*)buffer;
																					BYTE* dstData = (BYTE*)pixels;

																					do
																					{
																						MemoryCopy(dstData, srcData, pitch);

																						srcData += bufferPitch;
																						dstData += pitch;
																					} while (--viewHeight);
																				}
																				MemoryFree(buffer);
																			}
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
													else if (this->isTakeSnapshot)
														this->TakeSnapshot(currentBuffer, frameWidth, frameHeight);

													SwapBuffers(this->hDc);
													GLFinish();

													if (fpsState != FpsBenchmark || !config.version)
														WaitForSingleObject(this->hDrawEvent, INFINITE);
													else
														Sleep(0);
												}
											}
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
						GLDeleteTextures(2, textureId);
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

	RECT rect;
	GetClientRect(this->hWnd, &rect);

	if (config.singleWindow)
		this->hDraw = this->hWnd;
	else
	{
		if (!config.windowedMode && !config.borderlessMode)
		{
			this->hDraw = CreateWindowEx(
				WS_EX_CONTROLPARENT | WS_EX_TOPMOST,
				WC_DRAW,
				NULL,
				WS_VISIBLE | WS_POPUP,
				0, 0,
				rect.right, rect.bottom,
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
				0, 0,
				rect.right, rect.bottom,
				this->hWnd,
				NULL,
				hDllModule,
				NULL);
		}

		Window::SetCapturePanel(this->hDraw);

		SetClassLongPtr(this->hDraw, GCLP_HBRBACKGROUND, NULL);
		RedrawWindow(this->hDraw, NULL, NULL, RDW_INVALIDATE);
	}

	SetClassLongPtr(this->hWnd, GCLP_HBRBACKGROUND, NULL);
	RedrawWindow(this->hWnd, NULL, NULL, RDW_INVALIDATE);

	this->viewport.width = rect.right;
	this->viewport.height = rect.bottom - this->viewport.offset;
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
	if (!config.singleWindow)
		DestroyWindow(this->hDraw);

	this->hDraw = NULL;
	if (wasFull)
		GL::ResetPixelFormat();

	ClipCursor(NULL);

	glVersion = NULL;
	Window::CheckMenu(this->hWnd);
}

BOOL OpenDraw::CheckView(BOOL isDouble)
{
	if (this->viewport.refresh)
	{
		this->viewport.rectangle.x = this->viewport.rectangle.y = 0;
		this->viewport.rectangle.width = this->viewport.width;
		this->viewport.rectangle.height = this->viewport.height;

		this->viewport.clipFactor.x = this->viewport.viewFactor.x = (FLOAT)this->viewport.width / config.mode->width;
		this->viewport.clipFactor.y = this->viewport.viewFactor.y = (FLOAT)this->viewport.height / config.mode->height;

		if (config.image.aspect && this->viewport.viewFactor.x != this->viewport.viewFactor.y)
		{
			if (this->viewport.viewFactor.x > this->viewport.viewFactor.y)
			{
				FLOAT fw = this->viewport.viewFactor.y * config.mode->width;
				this->viewport.rectangle.width = (INT)MathRound(fw);
				this->viewport.rectangle.x = (INT)MathRound(((FLOAT)this->viewport.width - fw) / 2.0f);
				this->viewport.clipFactor.x = this->viewport.viewFactor.y;
			}
			else
			{
				FLOAT fh = this->viewport.viewFactor.x * config.mode->height;
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
			clipRect.bottom -= this->viewport.rectangle.y + this->viewport.offset;
			clipRect.top = clipRect.bottom - this->viewport.rectangle.height;

			ClientToScreen(this->hWnd, (POINT*)&clipRect.left);
			ClientToScreen(this->hWnd, (POINT*)&clipRect.right);

			ClipCursor(&clipRect);
		}
		else
			ClipCursor(NULL);

		this->clearStage = 0;
	}

	if (this->clearStage++ <= *(DWORD*)&isDouble)
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
	if (!config.zoomImage || !config.isBorder)
	{
		p->x = LONG((FLOAT)((p->x - this->viewport.rectangle.x) * config.mode->width) / this->viewport.rectangle.width);
		p->y = LONG((FLOAT)((p->y - this->viewport.rectangle.y) * config.mode->height) / this->viewport.rectangle.height);
	}
	else
	{
		LONG x, y;
		FLOAT k = (FLOAT)config.mode->width / config.mode->height;
		if (k >= 4.0f / 3.0f)
		{
			y = GAME_HEIGHT;
			x = LONG(k * y);

			k = GAME_HEIGHT_FLOAT / config.mode->height;
		}
		else
		{
			k = (FLOAT)config.mode->height / config.mode->width;

			x = GAME_WIDTH;
			y = LONG(k * x);

			k = GAME_WIDTH_FLOAT / config.mode->width;
		}

		p->x = LONG((FLOAT)((p->x - this->viewport.rectangle.x) * config.mode->width) / this->viewport.rectangle.width * k) + ((config.mode->width - x) >> 1);
		p->y = LONG((FLOAT)((p->y - this->viewport.rectangle.y) * config.mode->height) / this->viewport.rectangle.height * k) + ((config.mode->height - y) >> 1);
	}
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

	this->isTakeSnapshot = FALSE;
	this->isFinish = TRUE;

	MemoryZero(&this->windowPlacement, sizeof(WINDOWPLACEMENT));
	this->hDrawEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

OpenDraw::~OpenDraw()
{
	this->RenderStop();
	CloseHandle(this->hDrawEvent);
}

VOID OpenDraw::SetFullscreenMode()
{
	if (config.windowedMode)
		GetWindowPlacement(hWnd, &this->windowPlacement);

	this->viewport.offset = config.borderlessMode ? BORDERLESS_OFFSET : 0;
	SetMenu(hWnd, NULL);

	SetWindowLong(this->hWnd, GWL_STYLE, WS_FULLSCREEN);
	SetWindowPos(this->hWnd, NULL, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) + this->viewport.offset, NULL);
}

VOID OpenDraw::SetWindowedMode()
{
	if (!this->windowPlacement.length)
	{
		this->windowPlacement.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(hWnd, &this->windowPlacement);

		RECT rect = { 0, 0, (LONG)config.mode->width, (LONG)config.mode->height };
		AdjustWindowRect(&rect, WS_WINDOWED, TRUE);

		LONG nWidth = rect.right - rect.left;
		LONG nHeight = rect.bottom - rect.top;

		RECT* rc = &this->windowPlacement.rcNormalPosition;
		rc->left = (GetSystemMetrics(SM_CXSCREEN) - nWidth) >> 1;
		if (rc->left < 0)
			rc->left = 0;

		rc->top = (GetSystemMetrics(SM_CYSCREEN) - nHeight) >> 1;
		if (rc->top < 0)
			rc->top = 0;

		rc->right = rc->left + nWidth;
		rc->bottom = rc->top + nHeight;

		this->windowPlacement.ptMinPosition.x = this->windowPlacement.ptMinPosition.y = -1;
		this->windowPlacement.ptMaxPosition.x = this->windowPlacement.ptMaxPosition.y = -1;

		this->windowPlacement.flags = NULL;
		this->windowPlacement.showCmd = SW_SHOWNORMAL;
	}

	this->viewport.offset = 0;
	SetMenu(hWnd, config.menu);

	SetWindowLong(this->hWnd, GWL_STYLE, WS_WINDOWED);
	SetWindowPlacement(this->hWnd, &this->windowPlacement);
}

HRESULT __stdcall OpenDraw::QueryInterface(REFIID id, LPVOID* lpObj)
{
	*lpObj = new OpenDraw((IDrawUnknown**)&drawList);
	return DD_OK;
}

ULONG __stdcall OpenDraw::Release()
{
	if (--this->refCount)
		return this->refCount;

	delete this;
	return 0;
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
	config.mode->width = dwWidth;
	config.mode->height = dwHeight;
	config.mode->bpp = dwBPP;

	this->SetFullscreenMode();
	this->RenderStart();

	return DD_OK;
}

HRESULT __stdcall OpenDraw::CreateSurface(LPDDSURFACEDESC2 lpDDSurfaceDesc, IDrawSurface7** lplpDDSurface, IUnknown* pUnkOuter)
{
	// 1 - DDSD_CAPS // 512 - DDSCAPS_PRIMARYSURFACE
	// 33 - DDSD_BACKBUFFERCOUNT | DDSD_CAPS // 536 - DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX
	// 4103 - DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS // 2112 - DDSCAPS_SYSTEMMEMORY | DDSCAPS_OFFSCREENPLAIN
	// 7 - DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS // 64 - DDSCAPS_OFFSCREENPLAIN
	// 7 - DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS // 2112 - DDSCAPS_SYSTEMMEMORY | DDSCAPS_OFFSCREENPLAIN
	// 4103 - DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS // 64 - // 268451904 - DDSCAPS_LOCALVIDMEM | DDSCAPS_VIDEOMEMORY | DDSCAPS_OFFSCREENPLAIN
	// 4103 - DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS // 64 - // 0
	// 6159 - DDSD_PIXELFORMAT | DDSD_LPSURFACE | DDSD_PITCH | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS // 2112 - DDSCAPS_SYSTEMMEMORY | DDSCAPS_OFFSCREENPLAIN

	VOID* buffer = NULL;
	if (lpDDSurfaceDesc->dwFlags & DDSD_LPSURFACE)
		buffer = lpDDSurfaceDesc->lpSurface;

	OpenDrawSurface* surface;

	if (lpDDSurfaceDesc->dwFlags == (DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS))
	{
		surface = new OpenDrawSurface((IDrawUnknown**)&this->surfaceEntries, this, SurfaceSecondary);
		surface->AddRef();
		surface->CreateBuffer(lpDDSurfaceDesc->dwWidth, lpDDSurfaceDesc->dwHeight, config.mode->bpp, buffer);
		this->attachedSurface->attachedSurface = surface;
	}
	else
	{
		surface = new OpenDrawSurface((IDrawUnknown**)&this->surfaceEntries, this, (lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) ? SurfacePrimary : SurfaceOther);

		if (surface->type == SurfacePrimary)
			this->attachedSurface = surface;

		if (lpDDSurfaceDesc->dwFlags & (DDSD_WIDTH | DDSD_HEIGHT))
			surface->CreateBuffer(lpDDSurfaceDesc->dwWidth, lpDDSurfaceDesc->dwHeight, (lpDDSurfaceDesc->dwFlags & DDSD_PIXELFORMAT) ? lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount : config.mode->bpp, buffer);
		else
			surface->CreateBuffer(config.mode->width, config.mode->height, config.mode->bpp, buffer);
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
		ddSurfaceDesc.ddpfPixelFormat.dwRGBBitCount = mode->bpp;
		ddSurfaceDesc.ddpfPixelFormat.dwFlags = DDSD_ZBUFFERBITDEPTH;
		if (!lpEnumModesCallback(&ddSurfaceDesc, lpContext))
			break;

		++mode;
	} while (--count);

	return DD_OK;
}

HRESULT __stdcall OpenDraw::GetDisplayMode(LPDDSURFACEDESC2 lpDDSurfaceDesc)
{
	lpDDSurfaceDesc->dwWidth = config.mode->width;
	lpDDSurfaceDesc->dwHeight = config.mode->height;
	lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount = config.mode->bpp;
	lpDDSurfaceDesc->ddpfPixelFormat.dwFlags = DDPF_PALETTEINDEXED8;
	return DD_OK;
}

HRESULT __stdcall OpenDraw::CreatePalette(DWORD dwFlags, LPPALETTEENTRY lpDDColorArray, IDrawPalette** lplpDDPalette, IUnknown* pUnkOuter)
{
	OpenDrawPalette* palette = new OpenDrawPalette((IDrawUnknown**)&this->paletteEntries, this);
	*lplpDDPalette = palette;
	MemoryCopy(palette->entries, lpDDColorArray, 256 * sizeof(PALETTEENTRY));

	return DD_OK;
}