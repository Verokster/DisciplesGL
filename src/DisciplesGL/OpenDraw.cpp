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
#include "PngLib.h"

VOID __fastcall UseShaderProgram(ShaderProgram* program, DWORD texSize)
{
	if (!program->id)
	{
		program->id = GLCreateProgram();

		GLBindAttribLocation(program->id, 0, "vCoord");
		GLBindAttribLocation(program->id, 1, "vTex01");
		GLBindAttribLocation(program->id, 2, "vTex02");

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
		GLint loc = GLGetUniformLocation(program->id, "tex02");
		if (loc >= 0)
			GLUniform1i(loc, 1);

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

VOID OpenDraw::TakeSnapshot()
{
	if (this->isTakeSnapshot)
	{

		this->isTakeSnapshot = FALSE;
		GLFinish();

		if (OpenClipboard(NULL))
		{
			EmptyClipboard();

			BOOL isTrueColor = config.mode->bpp != 16 || config.bpp32Hooked || glVersion < GL_VER_1_1;

			DWORD pitch = this->viewport.rectangle.width * (isTrueColor ? 3 : 2);
			if (pitch & 3)
				pitch = (pitch & 0xFFFFFFFC) + 4;

			DWORD dataSize = pitch * this->viewport.rectangle.height;
			{
				DWORD infoSize = sizeof(BITMAPINFOHEADER);
				if (!isTrueColor)
					infoSize += sizeof(DWORD) * 3;

				HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, infoSize + dataSize);
				{
					VOID* data = GlobalLock(hMemory);
					{
						BITMAPV4HEADER* bmiHeader = (BITMAPV4HEADER*)data;
						bmiHeader->bV4Size = sizeof(BITMAPINFOHEADER);
						bmiHeader->bV4Width = this->viewport.rectangle.width;
						bmiHeader->bV4Height = this->viewport.rectangle.height;
						bmiHeader->bV4Planes = 1;
						bmiHeader->bV4SizeImage = dataSize;
						bmiHeader->bV4XPelsPerMeter = 1;
						bmiHeader->bV4YPelsPerMeter = 1;
						bmiHeader->bV4ClrUsed = 0;
						bmiHeader->bV4ClrImportant = 0;

						BYTE* buffer = (BYTE*)data + infoSize;

						if (isTrueColor)
						{
							bmiHeader->bV4BitCount = 24;
							bmiHeader->bV4V4Compression = BI_RGB;

							if (glVersion > GL_VER_1_1)
								GLReadPixels(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height, GL_BGR_EXT, GL_UNSIGNED_BYTE, buffer);
							else
							{
								GLReadPixels(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height, GL_RGB, GL_UNSIGNED_BYTE, buffer);

								DWORD height = this->viewport.rectangle.height;
								do
								{
									BYTE* src = buffer;

									DWORD width = this->viewport.rectangle.width;
									do
									{
										BYTE dt = src[0];
										src[0] = src[2];
										src[2] = dt;

										src += 3;
									} while (--width);

									buffer += pitch;
								} while (--height);
							}
						}
						else
						{
							bmiHeader->bV4BitCount = 16;
							bmiHeader->bV4V4Compression = BI_BITFIELDS;
							bmiHeader->bV4RedMask = 0xF800;
							bmiHeader->bV4GreenMask = 0x07E0;
							bmiHeader->bV4BlueMask = 0x001F;

							GLReadPixels(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, buffer);
						}
					}
					GlobalUnlock(hMemory);
					SetClipboardData(CF_DIB, hMemory);
				}
				GlobalFree(hMemory);
			}

			CloseClipboard();
		}
	}
}

DWORD __stdcall RenderThread(LPVOID lpParameter)
{
	OpenDraw* ddraw = (OpenDraw*)lpParameter;
	ddraw->hDc = ::GetDC(ddraw->hWnd);
	if (ddraw->hDc)
	{
		if (!::GetPixelFormat(ddraw->hDc))
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
		}

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

		::ReleaseDC(ddraw->hWnd, ddraw->hDc);
		ddraw->hDc = NULL;
	}

	return NULL;
}

VOID __fastcall LoadBack(VOID* buffer, DWORD width, DWORD height)
{
	if (!config.resHooked)
		return;

	HRSRC hResource = FindResource(hDllModule, MAKEINTRESOURCE(IDR_BORDER), RT_RCDATA);
	if (hResource)
	{
		{
			HRSRC hResource = FindResource(hDllModule, MAKEINTRESOURCE(IDR_BACK), RT_RCDATA);
			if (hResource)
			{
				HGLOBAL hResourceData = LoadResource(hDllModule, hResource);
				if (hResourceData)
				{
					ResourceStream stream = { LockResource(hResourceData), 0 };
					if (stream.data)
					{
						png_structp png_ptr = pnglib_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
						png_infop info_ptr = pnglib_create_info_struct(png_ptr);

						if (info_ptr)
						{
							pnglib_set_read_fn(png_ptr, &stream, PngLib::ReadDataFromInputStream);
							pnglib_read_info(png_ptr, info_ptr);

							if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
							{
								BYTE* data = (BYTE*)MemoryAlloc(info_ptr->height * info_ptr->rowbytes);
								if (data)
								{
									BYTE** list = (BYTE**)MemoryAlloc(info_ptr->height * sizeof(BYTE*));
									if (list)
									{
										BYTE** item = list;
										BYTE* row = data;
										DWORD count = info_ptr->height;
										while (count--)
										{
											*item++ = (BYTE*)row;
											row += info_ptr->rowbytes;
										}

										pnglib_read_image(png_ptr, list);
										MemoryFree(list);

										DWORD palette[256];
										{
											BYTE* src = (BYTE*)info_ptr->palette;
											BYTE* dst = (BYTE*)palette;
											DWORD count = (DWORD)info_ptr->num_palette;
											if (count > 256)
												count = 256;
											while (count--)
											{
												*dst++ = *src++;
												*dst++ = *src++;
												*dst++ = *src++;
												*dst++ = 0xFF;
											}
										}

										DWORD startX = DWORD(config.randPos.x % info_ptr->width);
										DWORD startY = DWORD(config.randPos.y % info_ptr->height);

										DWORD divX = width / (DWORD)info_ptr->width;
										DWORD modX = width % (DWORD)info_ptr->width;
										if (modX)
											++divX;

										DWORD divY = height / (DWORD)info_ptr->height;
										DWORD modY = height % (DWORD)info_ptr->height;
										if (modY)
											++divY;

										DWORD* dstH = (DWORD*)buffer;

										DWORD divH = divY;
										while (divH--)
										{
											DWORD cheight = !divH && modY ? modY : (DWORD)info_ptr->height;

											DWORD* dstW = dstH;

											DWORD divW = divX;
											while (divW--)
											{
												DWORD cwidth = !divW && modX ? modX : (DWORD)info_ptr->width;

												BYTE* srcData = data + info_ptr->rowbytes * startY;
												DWORD* dstData = dstW;

												DWORD copyY = startY;
												DWORD copyHeight = cheight;
												while (copyHeight--)
												{
													if (copyY == (DWORD)info_ptr->height)
													{
														copyY = 0;
														srcData = data;
													}
													++copyY;

													BYTE* src = srcData + (info_ptr->pixel_depth == 4 ? (startX >> 1) : startX);
													DWORD* dst = dstData;

													DWORD copyX = startX;
													DWORD copyWidth = cwidth;
													if (info_ptr->pixel_depth == 4)
													{
														BOOL tick = startX & 1;
														while (copyWidth--)
														{
															if (copyX == (DWORD)info_ptr->width)
															{
																copyX = 0;
																src = srcData;
																tick = FALSE;
															}
															++copyX;

															*dst++ = palette[!tick ? (*src >> 4) : (*src++ & 0xF)];
															tick = !tick;
														}
													}
													else
													{
														while (copyWidth--)
														{
															if (copyX == (DWORD)info_ptr->width)
															{
																copyX = 0;
																src = srcData;
															}
															++copyX;

															*dst++ = palette[*src++];
														}
													}

													srcData += info_ptr->rowbytes;
													dstData += width;
												}

												dstW += cwidth;
											}

											dstH += cheight * width;
										}
									}
									MemoryFree(data);
								}
							}
						}

						pnglib_destroy_read_struct(&png_ptr, NULL, NULL);
					}
				}
			}
		}
	}
}

VOID OpenDraw::RenderOld()
{
	BOOL isTrueColor = config.mode->bpp != 16 || config.bpp32Hooked;

	if (config.image.interpolation > InterpolateLinear)
		config.image.interpolation = InterpolateLinear;

	DWORD glMaxTexSize;
	GLGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&glMaxTexSize);
	if (glMaxTexSize < 256)
		glMaxTexSize = 256;

	DWORD size = config.mode->width > config.mode->height ? config.mode->width : config.mode->height;
	DWORD maxAllow = 1;
	while (maxAllow < size)
		maxAllow <<= 1;

	DWORD maxTexSize = maxAllow < glMaxTexSize ? maxAllow : glMaxTexSize;
	DWORD glFilter = config.image.interpolation == InterpolateNearest ? GL_NEAREST : GL_LINEAR;

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
		VOID* frameBuffer = AlignedAlloc(maxTexSize * maxTexSize * (!isTrueColor && glVersion > GL_VER_1_1 ? sizeof(WORD) : sizeof(DWORD)));
		{
			DWORD size = config.mode->width * config.mode->height * 4;
			VOID* back = AlignedAlloc(size);
			{
				MemoryZero(back, size);
				LoadBack(back, config.mode->width, config.mode->height);

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

						GLGenTextures(2, frame->id);

						if (config.resHooked)
						{
							GLBindTexture(GL_TEXTURE_2D, frame->id[1]);

							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glCapsClampToEdge);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glCapsClampToEdge);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glFilter);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glFilter);

							GLTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

							GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

							{
								DWORD* source = (DWORD*)back + frame->rect.y * config.mode->width + frame->rect.x;
								DWORD* dest = (DWORD*)frameBuffer;
								DWORD copyHeight = frame->rect.height;
								do
								{
									MemoryCopy(dest, source, frame->rect.width << 2);
									source += config.mode->width;
									dest += frame->rect.width;
								} while (--copyHeight);

								GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->rect.width, frame->rect.height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
							}
						}

						GLBindTexture(GL_TEXTURE_2D, frame->id[0]);

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
							GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
					}
				}
			}
			AlignedFree(back);

			GLMatrixMode(GL_PROJECTION);
			GLLoadIdentity();
			GLOrtho(0.0, (GLdouble)config.mode->width, (GLdouble)config.mode->height, 0.0, 0.0, 1.0);
			GLMatrixMode(GL_MODELVIEW);
			GLLoadIdentity();

			GLBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			GLEnable(GL_TEXTURE_2D);
			GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			this->viewport.refresh = TRUE;

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
						DWORD state = surface->state[surface->bufferIndex];

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
							glFilter = config.image.interpolation == InterpolateNearest ? GL_NEAREST : GL_LINEAR;
						}

						DWORD frameWidth, frameHeight;
						if (state & STATE_ZOOMED)
						{
							frameWidth = zWidth;
							frameHeight = zHeight;
						}
						else
						{
							frameWidth = config.mode->width;
							frameHeight = config.mode->height;
						}

						GLDisable(GL_BLEND);
						if (state & STATE_BORDER)
						{
							DWORD count = frameCount;
							Frame* frame = frames;
							while (count--)
							{
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
							DWORD count = frameCount;
							Frame* frame = frames;
							while (count--)
							{
								GLBindTexture(GL_TEXTURE_2D, frame->id[0]);

								if (glFilter)
								{
									GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glFilter);
									GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glFilter);
								}

								if (frameCount == 1)
								{
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
									if (!(state & STATE_ZOOMED))
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
						}

						this->TakeSnapshot();

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

		Frame* frame = frames;
		DWORD count = frameCount;
		while (count--)
		{
			GLDeleteTextures(2, frame->id);
			++frame;
		}
	}
	MemoryFree(frames);
}

VOID OpenDraw::RenderMid()
{
	BOOL isTrueColor = config.mode->bpp != 16 || config.bpp32Hooked;

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

	FLOAT mvpMatrix[4][4] = {
		{ FLOAT(2.0f / config.mode->width), 0.0f, 0.0f, 0.0f },
		{ 0.0f, FLOAT(-2.0f / config.mode->height), 0.0f, 0.0f },
		{ 0.0f, 0.0f, 2.0f, 0.0f },
		{ -1.0f, 1.0f, -1.0f, 1.0f }
	};

	struct {
		ShaderProgram linear;
		ShaderProgram linear_double;
		ShaderProgram hermite;
		ShaderProgram hermite_double;
		ShaderProgram cubic;
		ShaderProgram cubic_double;
	} shaders = {
		{ 0, GLSL_VER_1_10, IDR_LINEAR_VERTEX, IDR_LINEAR_FRAGMENT, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_10, IDR_LINEAR_VERTEX_DOUBLE, IDR_LINEAR_FRAGMENT_DOUBLE, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_10, IDR_HERMITE_VERTEX, IDR_HERMITE_FRAGMENT, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_10, IDR_HERMITE_VERTEX_DOUBLE, IDR_HERMITE_FRAGMENT_DOUBLE, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_10, IDR_CUBIC_VERTEX, IDR_CUBIC_FRAGMENT, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_10, IDR_CUBIC_VERTEX_DOUBLE, IDR_CUBIC_FRAGMENT_DOUBLE, (GLfloat*)mvpMatrix }
	};

	{
		GLuint bufferName;
		GLGenBuffers(1, &bufferName);
		{
			GLBindBuffer(GL_ARRAY_BUFFER, bufferName);
			{
				{
					FLOAT buffer[8][8] = {
						{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
						{ (FLOAT)config.mode->width, 0.0f, texWidth, 0.0f, texWidth, 0.0f },
						{ (FLOAT)config.mode->width, (FLOAT)config.mode->height, texWidth, texHeight, texWidth, texHeight },
						{ 0.0f, (FLOAT)config.mode->height, 0.0f, texHeight, 0.0f, texHeight },

						{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
						{ (FLOAT)config.mode->width, 0.0f, tw, 0.0f, texWidth, 0.0f },
						{ (FLOAT)config.mode->width, (FLOAT)config.mode->height, tw, th, texWidth, texHeight },
						{ 0.0f, (FLOAT)config.mode->height, 0.0f, th, 0.0f, texHeight }
					};

					GLBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_STATIC_DRAW);
				}

				{
					GLEnableVertexAttribArray(0);
					GLVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 32, (GLvoid*)0);

					GLEnableVertexAttribArray(1);
					GLVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 32, (GLvoid*)8);

					GLEnableVertexAttribArray(2);
					GLVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, (GLvoid*)16);
				}

				struct {
					GLuint back;
					GLuint primary;
				} textureId;

				if (config.resHooked)
					GLGenTextures(2, &textureId.back);
				else
					GLGenTextures(1, &textureId.primary);
				{
					if (config.resHooked)
					{
						GLActiveTexture(GL_TEXTURE1);
						GLBindTexture(GL_TEXTURE_2D, textureId.back);

						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

						GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

						DWORD size = config.mode->width * config.mode->height * 4;
						VOID* back = AlignedAlloc(size);
						{
							MemoryZero(back, size);
							LoadBack(back, config.mode->width, config.mode->height);
							GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, config.mode->width, config.mode->height, GL_RGBA, GL_UNSIGNED_BYTE, back);
						}
						AlignedFree(back);
					}

					{
						GLActiveTexture(GL_TEXTURE0);
						GLBindTexture(GL_TEXTURE_2D, textureId.primary);

						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

						if (isTrueColor)
							GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
						else
							GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
					}

					VOID* frameBuffer = AlignedAlloc(config.mode->width * config.mode->height * (isTrueColor ? sizeof(DWORD) : sizeof(WORD)));
					{
						FpsCounter* fpsCounter = new FpsCounter(FPS_ACCURACY);
						{
							GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);

							this->viewport.refresh = TRUE;
							this->isStateChanged = TRUE;

							DWORD borderStatus = STATE_NONE;
							BOOL isVSync = config.image.vSync;

							if (WGLSwapInterval)
								WGLSwapInterval(isVSync);

							while (!this->isFinish)
							{
								OpenDrawSurface* surface = this->attachedSurface;
								if (surface)
								{
									DWORD state = surface->state[surface->bufferIndex];
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

									DWORD frameWidth, frameHeight;
									if (state & STATE_ZOOMED)
									{
										frameWidth = zWidth;
										frameHeight = zHeight;
									}
									else
									{
										frameWidth = config.mode->width;
										frameHeight = config.mode->height;
									}

									if (state & STATE_BORDER && frameWidth == GAME_WIDTH && frameHeight == GAME_HEIGHT)
										state ^= STATE_BORDER;

									if (this->CheckView(TRUE))
										GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y + this->viewport.offset, this->viewport.rectangle.width, this->viewport.rectangle.height);

									if (this->isStateChanged || borderStatus != (state & STATE_BORDER))
									{
										borderStatus = state & STATE_BORDER;

										switch (config.image.interpolation)
										{
										case InterpolateHermite:
											UseShaderProgram(state & STATE_BORDER ? &shaders.hermite_double : &shaders.hermite, texSize);
											break;
										case InterpolateCubic:
											UseShaderProgram(state & STATE_BORDER ? &shaders.cubic_double : &shaders.cubic, texSize);
											break;
										default:
											UseShaderProgram(state & STATE_BORDER ? &shaders.linear_double : &shaders.linear, texSize);
											break;
										}

										if (this->isStateChanged)
										{
											this->isStateChanged = FALSE;

											DWORD filter = config.image.interpolation == InterpolateLinear || config.image.interpolation == InterpolateHermite ? GL_LINEAR : GL_NEAREST;

											GLActiveTexture(GL_TEXTURE1);
											GLBindTexture(GL_TEXTURE_2D, textureId.back);
											GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
											GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

											GLActiveTexture(GL_TEXTURE0);
											GLBindTexture(GL_TEXTURE_2D, textureId.primary);
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

										GLDrawArrays(GL_TRIANGLE_FAN, state & STATE_ZOOMED ? 4 : 0, 4);
									}

									this->TakeSnapshot();

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
				if (config.resHooked)
					GLDeleteTextures(2, &textureId.back);
				else
					GLDeleteTextures(1, &textureId.primary);
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

	FLOAT mvpMatrix[4][4] = {
		{ FLOAT(2.0f / config.mode->width), 0.0f, 0.0f, 0.0f },
		{ 0.0f, FLOAT(-2.0f / config.mode->height), 0.0f, 0.0f },
		{ 0.0f, 0.0f, 2.0f, 0.0f },
		{ -1.0f, 1.0f, -1.0f, 1.0f }
	};

	struct {
		ShaderProgram linear;
		ShaderProgram linear_double;
		ShaderProgram hermite;
		ShaderProgram hermite_double;
		ShaderProgram cubic;
		ShaderProgram cubic_double;
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
		{ 0, GLSL_VER_1_30, IDR_LINEAR_VERTEX_DOUBLE, IDR_LINEAR_FRAGMENT_DOUBLE, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_HERMITE_VERTEX, IDR_HERMITE_FRAGMENT, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_HERMITE_VERTEX_DOUBLE, IDR_HERMITE_FRAGMENT_DOUBLE, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_CUBIC_VERTEX, IDR_CUBIC_FRAGMENT, (GLfloat*)mvpMatrix },
		{ 0, GLSL_VER_1_30, IDR_CUBIC_VERTEX_DOUBLE, IDR_CUBIC_FRAGMENT_DOUBLE, (GLfloat*)mvpMatrix },
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

	{
		GLuint arrayName;
		GLGenVertexArrays(1, &arrayName);
		{
			GLBindVertexArray(arrayName);
			{
				GLuint bufferName;
				GLGenBuffers(1, &bufferName);
				{
					GLBindBuffer(GL_ARRAY_BUFFER, bufferName);
					{
						{
							FLOAT buffer[16][8] = {
								{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
								{ (FLOAT)config.mode->width, 0.0f, texWidth, 0.0f, texWidth, 0.0f },
								{ (FLOAT)config.mode->width, (FLOAT)config.mode->height, texWidth, texHeight, texWidth, texHeight },
								{ 0.0f, (FLOAT)config.mode->height, 0.0f, texHeight, 0.0f, texHeight },

								{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
								{ (FLOAT)config.mode->width, 0.0f, tw, 0.0f, texWidth, 0.0f },
								{ (FLOAT)config.mode->width, (FLOAT)config.mode->height, tw, th, texWidth, texHeight },
								{ 0.0f, (FLOAT)config.mode->height, 0.0f, th, 0.0f, texHeight },

								{ 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f },
								{ (FLOAT)config.mode->width, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
								{ (FLOAT)config.mode->width, (FLOAT)config.mode->height, 1.0f, 0.0f, 1.0f, 0.0f },
								{ 0.0f, (FLOAT)config.mode->height, 0.0f, 0.0f, 0.0f, 0.0f },

								{ 0.0f, 0.0f, 0.0f, tk, 0.0f, 1.0f },
								{ (FLOAT)config.mode->width, 0.0f, tk, tk, 1.0f, 1.0f },
								{ (FLOAT)config.mode->width, (FLOAT)config.mode->height, tk, 0.0f, 1.0f, 0.0f },
								{ 0.0f, (FLOAT)config.mode->height, 0.0f, 0.0f, 0.0f, 0.0f }
							};

							GLBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_STATIC_DRAW);
						}

						{
							GLEnableVertexAttribArray(0);
							GLVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 32, (GLvoid*)0);

							GLEnableVertexAttribArray(1);
							GLVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 32, (GLvoid*)8);

							GLEnableVertexAttribArray(2);
							GLVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, (GLvoid*)16);
						}

						struct {
							GLuint back;
							GLuint primary;
							GLuint secondary;
							GLuint primaryBO;
							GLuint backBO;
						} textureId;

						if (config.resHooked)
							GLGenTextures(2, &textureId.back);
						else
							GLGenTextures(1, &textureId.primary);
						{
							GLActiveTexture(GL_TEXTURE0);

							if (config.resHooked)
							{
								GLBindTexture(GL_TEXTURE_2D, textureId.back);

								GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
								GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
								GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
								GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
								GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
								GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

								GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

								DWORD size = config.mode->width * config.mode->height * 4;
								VOID* back = AlignedAlloc(size);
								{
									MemoryZero(back, size);
									LoadBack(back, config.mode->width, config.mode->height);
									GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, config.mode->width, config.mode->height, GL_RGBA, GL_UNSIGNED_BYTE, back);
								}
								AlignedFree(back);
							}

							{
								GLBindTexture(GL_TEXTURE_2D, textureId.primary);

								GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
								GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
								GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
								GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
								GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
								GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

								if (isTrueColor)
									GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
								else
									GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
							}

							GLuint fboId;
							GLGenFramebuffers(1, &fboId);
							{
								DWORD viewSize = 0;
								GLuint rboId = 0;

								VOID* frameBuffer = AlignedAlloc(config.mode->width * config.mode->height * (isTrueColor ? sizeof(DWORD) : sizeof(WORD)));
								{
									FpsCounter* fpsCounter = new FpsCounter(FPS_ACCURACY);
									{
										GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);

										this->viewport.refresh = TRUE;
										this->isStateChanged = TRUE;

										BOOL activeIndex = TRUE;
										DWORD zoomStatus = STATE_NONE;
										DWORD borderStatus = STATE_NONE;
										ShaderProgram* upscaleProgram;
										BOOL isVSync = config.image.vSync;

										if (WGLSwapInterval)
											WGLSwapInterval(isVSync);

										while (!this->isFinish)
										{
											OpenDrawSurface* surface = this->attachedSurface;
											if (surface)
											{
												DWORD state = surface->state[surface->bufferIndex];
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

												if (this->isStateChanged)
													this->viewport.refresh = TRUE;

												DWORD frameWidth, frameHeight;
												if (state & STATE_ZOOMED)
												{
													frameWidth = zWidth;
													frameHeight = zHeight;
												}
												else
												{
													frameWidth = config.mode->width;
													frameHeight = config.mode->height;
												}

												if (state & STATE_BORDER && frameWidth == GAME_WIDTH && frameHeight == GAME_HEIGHT)
													state ^= STATE_BORDER;

												if (config.image.upscaling != UpscaleNone)
												{
													GLBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboId);

													if (this->isStateChanged)
													{
														this->isStateChanged = FALSE;

														INT scale;
														switch (config.image.upscaling)
														{
														case UpscaleScaleNx:
															scale = config.image.scaleNx;
															switch (scale)
															{
															case 3:
																upscaleProgram = &shaders.scaleNx_3x;
																break;
															default:
																upscaleProgram = &shaders.scaleNx_2x;
																break;
															}

															break;

														case UpscaleScaleHQ:
															scale = config.image.scaleHQ;
															switch (scale)
															{
															case 4:
																upscaleProgram = &shaders.scaleHQ_4x;
																break;
															default:
																upscaleProgram = &shaders.scaleHQ_2x;
																break;
															}

															break;

														case UpscaleXRBZ:
															scale = config.image.xBRz;
															switch (scale)
															{
															case 6:
																upscaleProgram = &shaders.xBRz_6x;
																break;
															case 5:
																upscaleProgram = &shaders.xBRz_5x;
																break;
															case 4:
																upscaleProgram = &shaders.xBRz_4x;
																break;
															case 3:
																upscaleProgram = &shaders.xBRz_3x;
																break;
															default:
																upscaleProgram = &shaders.xBRz_2x;
																break;
															}

															break;

														case UpscaleXSal:
															scale = config.image.xSal;
															upscaleProgram = &shaders.xSal_2x;

															break;

														default:
															scale = config.image.eagle;
															upscaleProgram = &shaders.eagle_2x;

															break;
														}

														UseShaderProgram(upscaleProgram, texSize);

														DWORD newSize = MAKELONG(config.mode->width * scale, config.mode->height * scale);
														if (newSize != viewSize)
														{
															if (!viewSize)
															{
																GLGenRenderbuffers(1, &rboId);
																{
																	GLBindRenderbuffer(GL_RENDERBUFFER, rboId);
																	GLRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, LOWORD(newSize), HIWORD(newSize));
																	GLBindRenderbuffer(GL_RENDERBUFFER, NULL);
																	GLFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboId);
																}

																GLGenTextures(config.resHooked ? 3 : 2, (GLuint*)&textureId.secondary);
																{
																	GLBindTexture(GL_TEXTURE_2D, textureId.secondary);

																	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
																	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
																	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
																	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
																	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
																	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

																	if (isTrueColor)
																		GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
																	else
																		GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
																}
															}

															viewSize = newSize;

															// Gen texture
															DWORD idx = config.resHooked ? 2 : 1;
															while (idx--)
															{
																GLBindTexture(GL_TEXTURE_2D, ((GLuint*)&textureId.primaryBO)[idx]);
																GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
																GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
																GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
																GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
																GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
																GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
																GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, LOWORD(viewSize), HIWORD(viewSize), GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
															}
														}

														if (config.resHooked)
														{
															GLViewport(0, 0, LOWORD(viewSize), HIWORD(viewSize));

															GLFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId.backBO, 0);

															GLActiveTexture(GL_TEXTURE1);
															GLBindTexture(GL_TEXTURE_2D, ((GLuint*)&textureId.primary)[activeIndex]);
															GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
															GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

															DWORD* ptr = (DWORD*)frameBuffer;
															DWORD count = config.mode->width * config.mode->height;
															do
																*ptr++ = 0xFF000000;
															while (--count);
															GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, config.mode->width, config.mode->height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);

															GLActiveTexture(GL_TEXTURE0);
															GLBindTexture(GL_TEXTURE_2D, textureId.back);
															GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
															GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

															GLDrawArrays(GL_TRIANGLE_FAN, 0, 4);
															GLFinish();
														}

														GLFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId.primaryBO, 0);
													}

													if (state & STATE_ZOOMED)
														GLViewport(0, 0, DWORD(tk * LOWORD(viewSize)), DWORD(tk * HIWORD(viewSize)));
													else
														GLViewport(0, 0, LOWORD(viewSize), HIWORD(viewSize));

													GLActiveTexture(GL_TEXTURE1);
													GLBindTexture(GL_TEXTURE_2D, ((GLuint*)&textureId.primary)[activeIndex]);
													GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
													GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

													if (this->CheckView(FALSE) || zoomStatus != (state & STATE_ZOOMED))
													{
														zoomStatus = state & STATE_ZOOMED;

														if (isTrueColor)
														{
															DWORD* ptr = (DWORD*)frameBuffer;
															DWORD count = frameWidth * frameHeight;
															do
																*ptr++ = 0xFF000000;
															while (--count);
															GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameWidth, frameHeight, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
														}
														else
														{
															MemoryZero(frameBuffer, frameWidth * frameHeight * (isTrueColor ? sizeof(DWORD) : sizeof(WORD)));
															GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameWidth, frameHeight, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, frameBuffer);
														}
													}

													activeIndex = !activeIndex;
													GLActiveTexture(GL_TEXTURE0);
													GLBindTexture(GL_TEXTURE_2D, ((GLuint*)&textureId.primary)[activeIndex]);
													GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
													GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
												}
												else
												{
													if (this->CheckView(TRUE))
														GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y + this->viewport.offset, this->viewport.rectangle.width, this->viewport.rectangle.height);

													if (this->isStateChanged || borderStatus != (state & STATE_BORDER))
													{
														borderStatus = state & STATE_BORDER;

														switch (config.image.interpolation)
														{
														case InterpolateHermite:
															UseShaderProgram(state & STATE_BORDER ? &shaders.hermite_double : &shaders.hermite, texSize);
															break;
														case InterpolateCubic:
															UseShaderProgram(state & STATE_BORDER ? &shaders.cubic_double : &shaders.cubic, texSize);
															break;
														default:
															UseShaderProgram(state & STATE_BORDER ? &shaders.linear_double : &shaders.linear, texSize);
															break;
														}

														if (this->isStateChanged)
														{
															this->isStateChanged = FALSE;

															if (viewSize)
															{
																GLDeleteTextures(config.resHooked ? 3 : 2, (GLuint*)&textureId.secondary);
																GLDeleteRenderbuffers(1, &rboId);
																viewSize = 0;
															}

															DWORD filter = config.image.interpolation == InterpolateLinear || config.image.interpolation == InterpolateHermite ? GL_LINEAR : GL_NEAREST;

															GLActiveTexture(GL_TEXTURE1);
															GLBindTexture(GL_TEXTURE_2D, textureId.back);
															GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
															GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

															GLActiveTexture(GL_TEXTURE0);
															GLBindTexture(GL_TEXTURE_2D, textureId.primary);
															GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
															GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
														}
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
													GLDrawArrays(GL_TRIANGLE_FAN, state & STATE_ZOOMED ? 4 : 0, 4);
												}

												// Draw from FBO
												if (config.image.upscaling != UpscaleNone)
												{
													GLFinish();
													GLBindFramebuffer(GL_DRAW_FRAMEBUFFER, NULL);

													switch (config.image.interpolation)
													{
													case InterpolateHermite:
														UseShaderProgram(state & STATE_BORDER ? &shaders.hermite_double : &shaders.hermite, texSize);
														break;
													case InterpolateCubic:
														UseShaderProgram(state & STATE_BORDER ? &shaders.cubic_double : &shaders.cubic, texSize);
														break;
													default:
														UseShaderProgram(state & STATE_BORDER ? &shaders.linear_double : &shaders.linear, texSize);
														break;
													}

													{
														GLClear(GL_COLOR_BUFFER_BIT);
														GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y + this->viewport.offset, this->viewport.rectangle.width, this->viewport.rectangle.height);

														DWORD filter = config.image.interpolation == InterpolateLinear || config.image.interpolation == InterpolateHermite ? GL_LINEAR : GL_NEAREST;

														GLActiveTexture(GL_TEXTURE1);
														GLBindTexture(GL_TEXTURE_2D, textureId.backBO);
														GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
														GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

														GLActiveTexture(GL_TEXTURE0);
														GLBindTexture(GL_TEXTURE_2D, textureId.primaryBO);
														GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
														GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

														GLDrawArrays(GL_TRIANGLE_FAN, state & STATE_ZOOMED ? 12 : 8, 4);
													}
													UseShaderProgram(upscaleProgram, texSize);
												}

												this->TakeSnapshot();

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

								if (viewSize)
								{
									GLDeleteTextures(config.resHooked ? 3 : 2, (GLuint*)&textureId.secondary);
									GLDeleteRenderbuffers(1, &rboId);
								}
							}
							GLDeleteFramebuffers(1, &fboId);
						}
						if (config.resHooked)
							GLDeleteTextures(2, &textureId.back);
						else
							GLDeleteTextures(1, &textureId.primary);
					}
					GLBindBuffer(GL_ARRAY_BUFFER, NULL);
				}
				GLDeleteBuffers(1, &bufferName);
			}
			GLBindVertexArray(NULL);
		}
		GLDeleteVertexArrays(1, &arrayName);
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

	SetClassLongPtr(this->hWnd, GCLP_HBRBACKGROUND, NULL);
	RedrawWindow(this->hWnd, NULL, NULL, RDW_INVALIDATE);

	RECT rect;
	GetClientRect(this->hWnd, &rect);
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

	if (GetWindowLong(this->hWnd, GWL_STYLE) & WS_POPUP)
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