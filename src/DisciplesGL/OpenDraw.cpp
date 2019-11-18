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
#include "PixelBuffer.h"
#include "FpsCounter.h"
#include "Config.h"
#include "Window.h"
#include "Hooks.h"
#include "PngLib.h"

DWORD __fastcall GetPow2(DWORD value)
{
	DWORD res = 1;
	while (res < value)
		res <<= 1;
	return res;
}

VOID __fastcall GLBindTexFilter(GLuint textureId, GLint filter)
{
	GLBindTexture(GL_TEXTURE_2D, textureId);
	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
}

VOID __fastcall GLBindTexParameter(GLuint textureId, GLint filter)
{
	GLBindTexture(GL_TEXTURE_2D, textureId);
	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
}

VOID __fastcall UseShaderProgram(ShaderProgram* program, DWORD texSize)
{
	if (!program->id)
	{
		program->id = GLCreateProgram();

		GLBindAttribLocation(program->id, 0, "vCoord");
		GLBindAttribLocation(program->id, 1, "vTex");

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

VOID __fastcall LoadBack(VOID* buffer, DWORD width, DWORD height)
{
	if (config.version || !config.resHooked)
		return;

	Stream stream;
	MemoryZero(&stream, sizeof(Stream));
	if (Main::LoadResource(MAKEINTRESOURCE(IDR_BACK), &stream))
	{
		png_structp png_ptr = pnglib_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		png_infop info_ptr = pnglib_create_info_struct(png_ptr);

		if (info_ptr)
		{
			pnglib_set_read_fn(png_ptr, &stream, PngLib::ReadDataFromInputStream);
			pnglib_read_info(png_ptr, info_ptr);

			if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
			{
				DWORD infoWidth = info_ptr->width << 1;
				DWORD infoHeight = info_ptr->height;
				DWORD bytesInRow = (info_ptr->width << (info_ptr->pixel_depth == 4 ? 0 : 1));

				BYTE* data = (BYTE*)MemoryAlloc(infoHeight * bytesInRow);
				if (data)
				{
					BYTE** list = (BYTE**)MemoryAlloc(infoHeight * sizeof(BYTE*));
					if (list)
					{
						BYTE** item = list;
						BYTE* row = data;
						DWORD count = infoHeight;
						while (count--)
						{
							*item++ = (BYTE*)row;
							row += bytesInRow;
						}

						pnglib_read_image(png_ptr, list);
						MemoryFree(list);

						// tile images
						{
							BYTE* dstData = data + (bytesInRow >> 1);

							DWORD hHeight = info_ptr->height >> 1;
							for (DWORD i = 0; i < 2; ++i)
							{
								BYTE* srcData = data + !i * hHeight * bytesInRow;

								DWORD count = hHeight;
								do
								{
									MemoryCopy(dstData, srcData, bytesInRow >> 1);

									srcData += bytesInRow;
									dstData += bytesInRow;
								} while (--count);
							}
						}

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

						DWORD startX = DWORD(config.randPos.x % infoWidth);
						DWORD startY = DWORD(config.randPos.y % infoHeight);

						DWORD divX = width / (DWORD)infoWidth;
						DWORD modX = width % (DWORD)infoWidth;
						if (modX)
							++divX;

						DWORD divY = height / (DWORD)infoHeight;
						DWORD modY = height % (DWORD)infoHeight;
						if (modY)
							++divY;

						DWORD* dstH = (DWORD*)buffer;

						DWORD divH = divY;
						while (divH--)
						{
							DWORD cheight = !divH && modY ? modY : (DWORD)infoHeight;

							DWORD* dstW = dstH;

							DWORD divW = divX;
							while (divW--)
							{
								DWORD cwidth = !divW && modX ? modX : (DWORD)infoWidth;

								BYTE* srcData = data + bytesInRow * startY;
								DWORD* dstData = dstW;

								DWORD copyY = startY;
								DWORD copyHeight = cheight;
								while (copyHeight--)
								{
									if (copyY == (DWORD)infoHeight)
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
											if (copyX == (DWORD)infoWidth)
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
											if (copyX == (DWORD)infoWidth)
											{
												copyX = 0;
												src = srcData;
											}
											++copyX;

											*dst++ = palette[*src++];
										}
									}

									srcData += bytesInRow;
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

		pnglib_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	}
}

VOID OpenDraw::ReadFrameBufer(BYTE* dstData, DWORD pitch, BOOL isBGR, BOOL isReverse)
{
	GLReadPixels(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height, isBGR ? GL_BGR_EXT : GL_RGB, GL_UNSIGNED_BYTE, dstData);

	if (isReverse)
	{
		DWORD height = *(DWORD*)&this->viewport.rectangle.height;
		do
		{
			BYTE* src = dstData;

			DWORD width = *(DWORD*)&this->viewport.rectangle.width;
			do
			{
				BYTE dt = src[0];
				src[0] = src[2];
				src[2] = dt;

				src += 3;
			} while (--width);

			dstData += pitch;
		} while (--height);
	}
}

VOID OpenDraw::ReadDataBuffer(BYTE* dstData, VOID* srcData, Size* size, DWORD pitch, BOOL isTrueColor, BOOL isReverse)
{
	if (!isTrueColor)
	{
		WORD* srcPtr = (WORD*)srcData + (size->height - 1) * size->width;

		DWORD height = size->height;
		do
		{
			WORD* src = srcPtr;
			BYTE* dst = dstData;
			DWORD width = size->width;
			do
			{
				WORD p = *src++;
				DWORD px = ((p & 0xF800) >> 8) | ((p & 0x07E0) << 5) | ((p & 0x001F) << 19);
				BYTE* c = (BYTE*)&px;

				if (!isReverse)
				{
					*dst++ = c[0];
					*dst++ = c[1];
					*dst++ = c[2];
				}
				else
				{
					*dst++ = c[2];
					*dst++ = c[1];
					*dst++ = c[0];
				}
			} while (--width);

			srcPtr -= size->width;
			dstData += pitch;
		} while (--height);
	}
	else
	{
		DWORD* srcPtr = (DWORD*)srcData + (size->height - 1) * size->width;
		DWORD height = size->height;
		do
		{
			DWORD* src = srcPtr;
			BYTE* dst = dstData;
			DWORD width = size->width;
			do
			{
				DWORD px = *src++;
				BYTE* c = (BYTE*)&px;

				if (!isReverse)
				{
					*dst++ = c[0];
					*dst++ = c[1];
					*dst++ = c[2];
				}
				else
				{
					*dst++ = c[2];
					*dst++ = c[1];
					*dst++ = c[0];
				}
			} while (--width);

			srcPtr -= size->width;
			dstData += pitch;
		} while (--height);
	}
}

VOID OpenDraw::TakeSnapshot(Size* size, VOID* stateData, BOOL isTrueColor)
{
	BOOL res = FALSE;
	BOOL isSurface = this->isTakeSnapshot == SnapshotSurfaceClipboard || this->isTakeSnapshot == SnapshotSurfaceFile;
	Size snapSize = isSurface ? *(Size*)size : *(Size*)&this->viewport.rectangle.width;

	switch (this->isTakeSnapshot)
	{
	case SnapshotClipboard:
	case SnapshotSurfaceClipboard:
	{
		if (OpenClipboard(NULL))
		{
			EmptyClipboard();

			DWORD pitch = snapSize.width * 3;
			if (pitch & 3)
				pitch = (pitch & 0xFFFFFFFC) + 4;

			DWORD dataSize = pitch * snapSize.height;
			{
				DWORD infoSize = sizeof(BITMAPINFOHEADER);
				HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, infoSize + dataSize);
				{
					VOID* data = GlobalLock(hMemory);
					{
						BITMAPV4HEADER* bmiHeader = (BITMAPV4HEADER*)data;
						bmiHeader->bV4Size = sizeof(BITMAPINFOHEADER);
						bmiHeader->bV4Width = snapSize.width;
						bmiHeader->bV4Height = snapSize.height;
						bmiHeader->bV4Planes = 1;
						bmiHeader->bV4SizeImage = dataSize;
						bmiHeader->bV4XPelsPerMeter = 1;
						bmiHeader->bV4YPelsPerMeter = 1;
						bmiHeader->bV4ClrUsed = 0;
						bmiHeader->bV4ClrImportant = 0;

						BYTE* buffer = (BYTE*)data + infoSize;

						bmiHeader->bV4BitCount = 24;
						bmiHeader->bV4V4Compression = BI_RGB;

						if (isSurface)
							ReadDataBuffer(buffer, stateData, size, pitch, isTrueColor, TRUE);
						else
						{
							BOOL isNew = glVersion > GL_VERSION_1_1;
							ReadFrameBufer(buffer, pitch, isNew, !isNew);
						}

						res = TRUE;
					}
					GlobalUnlock(hMemory);
					SetClipboardData(CF_DIB, hMemory);
				}
				GlobalFree(hMemory);
			}

			CloseClipboard();
		}
	}
	break;

	case SnapshotFile:
	case SnapshotSurfaceFile:
	{
		CHAR path[MAX_PATH];
		GetModuleFileName(NULL, path, sizeof(path));
		CHAR* ptr = StrLastChar(path, '\\');

		const CHAR* format;
		const CHAR* extension;
		DWORD offset;
		if (config.version)
		{
			StrCopy(ptr, "\\SDUMP???.*");
			format = "\\SDUMP%03d.%s";
			offset = 5;
		}
		else
		{
			StrCopy(ptr, "\\ScreenShots");
			ptr += sizeof("\\ScreenShots") - 1;

			DWORD dwAttrib = GetFileAttributes(path);
			if (dwAttrib == INVALID_FILE_ATTRIBUTES || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
				CreateDirectory(path, NULL);

			StrCopy(ptr, "\\ScreenShot???.*");
			format = "\\ScreenShot%03d.%s";
			offset = 10;
		}

		INT index = -1;
		WIN32_FIND_DATA fData;
		HANDLE hFind = FindFirstFile(path, &fData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				CHAR* p = fData.cFileName + offset;

				INT number = 0;
				DWORD count = 3;
				do
				{
					CHAR ch = *p++;
					if (ch < '0' || ch > '9')
					{
						number = 0;
						break;
					}

					INT dig = ch - '0';

					DWORD c = count;
					while (--c)
						dig *= 10;

					number += dig;
				} while (--count);

				if (index < number)
					index = number;
			} while (FindNextFile(hFind, &fData));

			FindClose(hFind);
		}

		DWORD pitch = snapSize.width * 3;
		if (pitch & 3)
			pitch = (pitch & 0xFFFFFFFC) + 4;

		Stream stream = { NULL, 0, 0 };
		if (config.snapshot.type == ImagePNG && pnglib_create_write_struct)
		{
			extension = "png";

			png_structp png_ptr = pnglib_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
			png_infop info_ptr = pnglib_create_info_struct(png_ptr);

			if (info_ptr)
			{
				pnglib_set_write_fn(png_ptr, &stream, PngLib::WriteDataIntoOutputStream, PngLib::FlushData);
				pnglib_set_filter(png_ptr, PNG_FILTER_TYPE_BASE, PNG_ALL_FILTERS);
				pnglib_set_IHDR(png_ptr, info_ptr, snapSize.width, snapSize.height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

				// png_set_compression_level
				png_ptr->flags |= PNG_FLAG_ZLIB_CUSTOM_LEVEL;
				png_ptr->zlib_level = config.snapshot.level;

				pnglib_write_info(png_ptr, info_ptr);

				BYTE* rowsData = (BYTE*)MemoryAlloc(pitch * snapSize.height);
				if (rowsData)
				{
					BYTE** list = (BYTE**)MemoryAlloc(snapSize.height * sizeof(BYTE*));
					if (list)
					{
						BYTE** item = list;
						DWORD count = snapSize.height;
						BYTE* row = rowsData + (count - 1) * pitch;
						while (count--)
						{
							*item++ = (BYTE*)row;
							row -= pitch;
						}

						if (isSurface)
							ReadDataBuffer(rowsData, stateData, size, pitch, isTrueColor, FALSE);
						else
							ReadFrameBufer(rowsData, pitch, FALSE, FALSE);

						pnglib_write_image(png_ptr, list);
						pnglib_write_end(png_ptr, NULL);

						MemoryFree(list);
					}

					MemoryFree(rowsData);
				}
			}

			pnglib_destroy_write_struct(&png_ptr, &info_ptr);
		}
		else
		{
			extension = "bmp";

			stream.size = pitch * snapSize.height + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPCOREHEADER);
			stream.data = (BYTE*)MemoryAlloc(stream.size);
			if (stream.data)
			{
				stream.position = stream.size;

				BITMAPFILEHEADER* bmpHeader = (BITMAPFILEHEADER*)stream.data;
				bmpHeader->bfType = 0x4D42;
				bmpHeader->bfSize = stream.size;
				bmpHeader->bfReserved1 = NULL;
				bmpHeader->bfReserved2 = NULL;
				bmpHeader->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPCOREHEADER);

				BITMAPCOREHEADER* dibHeader = (BITMAPCOREHEADER*)(stream.data + sizeof(BITMAPFILEHEADER));
				dibHeader->bcSize = sizeof(BITMAPCOREHEADER);
				dibHeader->bcWidth = LOWORD(snapSize.width);
				dibHeader->bcHeight = LOWORD(snapSize.height);
				dibHeader->bcPlanes = 1;
				dibHeader->bcBitCount = 24;

				BYTE* dstData = stream.data + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPCOREHEADER);
				if (isSurface)
					ReadDataBuffer(dstData, stateData, size, pitch, isTrueColor, TRUE);
				else
				{
					BOOL isNew = glVersion > GL_VERSION_1_1;
					ReadFrameBufer(dstData, pitch, isNew, !isNew);
				}
			}
		}

		if (stream.data)
		{
			StrPrint(ptr, format, ++index, extension);

			HANDLE hFile = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				DWORD write;
				DWORD written = 0;
				DWORD total = 0;

				do
				{
					total += written;
					write = stream.position - total;

					if (!write)
						break;

					if (write > 65536)
						write = 65536;
				} while (WriteFile(hFile, stream.data + total, write, &written, NULL));

				res = TRUE;

				CloseHandle(hFile);
			}

			MemoryFree(stream.data);
		}

		if (res)
		{
			StrCopy(snapshotName, ptr + 1);
			PostMessage(this->hWnd, WM_SNAPSHOT, FALSE, NULL);
		}
	}
	break;

	default:
		break;
	}

	this->isTakeSnapshot = SnapshotNone;

	if (res)
		MessageBeep(MB_ICONASTERISK);
}

DWORD __stdcall RenderThread(LPVOID lpParameter)
{
	OpenDraw* ddraw = (OpenDraw*)lpParameter;
	ddraw->hDc = ::GetDC(ddraw->hDraw);
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
					Main::ShowError(IDS_ERROR_CHOOSE_PF, "OpenDraw.cpp", __LINE__);
				else if (pfd.dwFlags & PFD_NEED_PALETTE)
					Main::ShowError(IDS_ERROR_NEED_PALETTE, "OpenDraw.cpp", __LINE__);
			}

			GL::ResetPixelFormatDescription(&pfd);
			if (::DescribePixelFormat(ddraw->hDc, glPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd) == NULL)
				Main::ShowError(IDS_ERROR_DESCRIBE_PF, "OpenDraw.cpp", __LINE__);

			if (!::SetPixelFormat(ddraw->hDc, glPixelFormat, &pfd))
				Main::ShowError(IDS_ERROR_SET_PF, "OpenDraw.cpp", __LINE__);

			if (pfd.iPixelType != PFD_TYPE_RGBA || pfd.cRedBits < 5 || pfd.cGreenBits < 6 || pfd.cBlueBits < 5)
				Main::ShowError(IDS_ERROR_BAD_PF, "OpenDraw.cpp", __LINE__);
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

				GLPixelStorei(GL_UNPACK_ROW_LENGTH, config.mode->width);

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

	SetEvent(ddraw->hCheckEvent);

	return NULL;
}

VOID OpenDraw::RenderOld()
{
	BOOL isTrueColor = config.mode->bpp != 16 || config.bpp32Hooked;

	if (this->filterState.interpolation > InterpolateLinear)
		this->filterState.interpolation = InterpolateLinear;

	DWORD glMaxTexSize;
	GLGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&glMaxTexSize);
	if (glMaxTexSize < 256)
		glMaxTexSize = 256;

	DWORD maxAllow = GetPow2(config.mode->width > config.mode->height ? config.mode->width : config.mode->height);

	DWORD maxTexSize = maxAllow < glMaxTexSize ? maxAllow : glMaxTexSize;
	DWORD glFilter = this->filterState.interpolation == InterpolateNearest ? GL_NEAREST : GL_LINEAR;

	DWORD framePerWidth = config.mode->width / maxTexSize + (config.mode->width % maxTexSize ? 1 : 0);
	DWORD framePerHeight = config.mode->height / maxTexSize + (config.mode->height % maxTexSize ? 1 : 0);
	DWORD frameCount = framePerWidth * framePerHeight;

	if (frameCount != 1 && config.zoom.allowed)
	{
		config.zoom.allowed = FALSE;
		config.zoom.menu = FALSE;
		config.zoom.enabled = FALSE;
	}

	SetEvent(this->hCheckEvent);

	Frame* frames = (Frame*)MemoryAlloc(frameCount * sizeof(Frame));
	{
		VOID* frameBuffer = AlignedAlloc(config.mode->width * config.mode->height * (!isTrueColor && glVersion > GL_VER_1_1 ? sizeof(WORD) : sizeof(DWORD)));
		{
			DWORD size = config.mode->width * config.mode->height * 4;
			VOID* back = MemoryAlloc(size);
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

						if (!config.version && config.resHooked)
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
							GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->rect.width, frame->rect.height, GL_RGBA, GL_UNSIGNED_BYTE, (DWORD*)back + frame->rect.y * config.mode->width + frame->rect.x);
						}

						GLBindTexture(GL_TEXTURE_2D, frame->id[0]);

						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glCapsClampToEdge);
						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glCapsClampToEdge);
						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glFilter);
						GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glFilter);

						GLTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

						if (isTrueColor || glVersion <= GL_VER_1_1)
							GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
						else
							GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);	
					}
				}
			}
			MemoryFree(back);

			FpsCounter* fpsCounter = new FpsCounter(FPS_ACCURACY, frameBuffer);
			{
				this->viewport.refresh = TRUE;
				this->filterState.flags = TRUE;

				BOOL isVSync = config.image.vSync;

				GLMatrixMode(GL_PROJECTION);
				GLLoadIdentity();
				GLOrtho(0.0, (GLdouble)config.mode->width, (GLdouble)config.mode->height, 0.0, 0.0, 1.0);
				GLMatrixMode(GL_MODELVIEW);
				GLLoadIdentity();

				GLBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				GLEnable(GL_TEXTURE_2D);

				GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				if (WGLSwapInterval)
					WGLSwapInterval(isVSync);

				while (!this->isFinish)
				{
					OpenDrawSurface* surface = this->attachedSurface;
					if (!surface)
					{
						Sleep(0);
						continue;
					}

					StateBufferAligned* stateBuffer = (StateBufferAligned*)(!this->bufferIndex ? surface->indexBuffer : surface->secondaryBuffer);
					if (!stateBuffer)
					{
						Sleep(0);
						continue;
					}

					Size* frameSize = &stateBuffer->size;
					if (stateBuffer->isReady || this->viewport.refresh && frameSize->width && frameSize->height)
					{
						this->bufferIndex = !this->bufferIndex;

						stateBuffer = (StateBufferAligned*)(this->bufferIndex ? surface->indexBuffer : surface->secondaryBuffer);
						if (!stateBuffer)
						{
							Sleep(0);
							continue;
						}

						stateBuffer->isReady = FALSE;
						surface->drawEnabled = TRUE;

						fpsCounter->Init();

						if (isVSync != config.image.vSync)
						{
							isVSync = config.image.vSync;
							if (WGLSwapInterval)
								WGLSwapInterval(isVSync);
						}

						if (this->CheckView(TRUE))
							GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y + this->viewport.offset, this->viewport.rectangle.width, this->viewport.rectangle.height);

						glFilter = 0;
						FilterState state = this->filterState;
						this->filterState.flags = FALSE;
						if (state.flags)
							glFilter = state.interpolation == InterpolateNearest ? GL_NEAREST : GL_LINEAR;

						GLDisable(GL_BLEND);
						if (stateBuffer->isBorder)
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
										GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameSize->width, frameSize->height, GL_RGBA, GL_UNSIGNED_BYTE, (DWORD*)stateBuffer->data + ((config.mode->height - stateBuffer->size.height) >> 1) * config.mode->width + ((config.mode->width - stateBuffer->size.width) >> 1));
									else if (glVersion > GL_VER_1_1)
										GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameSize->width, frameSize->height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (WORD*)stateBuffer->data + ((config.mode->height - stateBuffer->size.height) >> 1) * config.mode->width + ((config.mode->width - stateBuffer->size.width) >> 1));
									else
									{
										WORD* src = (WORD*)stateBuffer->data + ((config.mode->height - stateBuffer->size.height) >> 1) * config.mode->width + ((config.mode->width - stateBuffer->size.width) >> 1);
										DWORD* dst = (DWORD*)frameBuffer;

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
									if (isTrueColor)
										GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->rect.width, frame->rect.height, GL_RGBA, GL_UNSIGNED_BYTE, (DWORD*)stateBuffer->data + frame->rect.y * frameSize->width + frame->rect.x);
									else if (glVersion > GL_VER_1_1)
										GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->rect.width, frame->rect.height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (WORD*)stateBuffer->data + frame->rect.y * frameSize->width + frame->rect.x);
									else
									{
										WORD* src = (WORD*)stateBuffer->data + frame->rect.y * frameSize->width + frame->rect.x;
										DWORD* dst = (DWORD*)frameBuffer;

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

								if (!this->isTakeSnapshot && frame == frames)
									fpsCounter->Draw(stateBuffer->data);

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
										FLOAT tw = (FLOAT)stateBuffer->size.width / maxTexSize;
										FLOAT th = (FLOAT)stateBuffer->size.height / maxTexSize;

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

						SwapBuffers(this->hDc);
						GLFinish();
						fpsCounter->Calculate();
						if (this->isTakeSnapshot)
							this->TakeSnapshot(frameSize, stateBuffer->data, isTrueColor);
					}

					WaitForSingleObject(this->hDrawEvent, INFINITE);
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
	SetEvent(this->hCheckEvent);

	BOOL isTrueColor = config.mode->bpp != 16 || config.bpp32Hooked;
	DWORD maxTexSize = GetPow2(config.mode->width > config.mode->height ? config.mode->width : config.mode->height);

	FLOAT texWidth = config.mode->width == maxTexSize ? 1.0f : (FLOAT)config.mode->width / maxTexSize;
	FLOAT texHeight = config.mode->height == maxTexSize ? 1.0f : (FLOAT)config.mode->height / maxTexSize;

	DWORD texSize = (maxTexSize & 0xFFFF) | (maxTexSize << 16);

	struct {
		ShaderProgram linear;
		ShaderProgram linear_double;
		ShaderProgram hermite;
		ShaderProgram hermite_double;
		ShaderProgram cubic;
		ShaderProgram cubic_double;
	} shaders = {
		{ 0, GLSL_VER_1_10, IDR_LINEAR_VERTEX, IDR_LINEAR_FRAGMENT },
		{ 0, GLSL_VER_1_10, IDR_LINEAR_VERTEX_DOUBLE, IDR_LINEAR_FRAGMENT_DOUBLE },
		{ 0, GLSL_VER_1_10, IDR_HERMITE_VERTEX, IDR_HERMITE_FRAGMENT },
		{ 0, GLSL_VER_1_10, IDR_HERMITE_VERTEX_DOUBLE, IDR_HERMITE_FRAGMENT_DOUBLE },
		{ 0, GLSL_VER_1_10, IDR_CUBIC_VERTEX, IDR_CUBIC_FRAGMENT },
		{ 0, GLSL_VER_1_10, IDR_CUBIC_VERTEX_DOUBLE, IDR_CUBIC_FRAGMENT_DOUBLE }
	};

	{
		GLuint bufferName;
		GLGenBuffers(1, &bufferName);
		{
			GLBindBuffer(GL_ARRAY_BUFFER, bufferName);
			{
				FLOAT buffer[4][8] = {
					{ 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
					{ (FLOAT)config.mode->width, 0.0f, 0.0f, 1.0f, texWidth, 0.0f, texWidth, 0.0f },
					{ (FLOAT)config.mode->width, (FLOAT)config.mode->height, 0.0f, 1.0f, texWidth, texHeight, texWidth, texHeight },
					{ 0.0f, (FLOAT)config.mode->height, 0.0f, 1.0f, 0.0f, texHeight, 0.0f, texHeight }
				};

				{
					FLOAT mvp[4][4] = {
						{ FLOAT(2.0f / config.mode->width), 0.0f, 0.0f, 0.0f },
						{ 0.0f, FLOAT(-2.0f / config.mode->height), 0.0f, 0.0f },
						{ 0.0f, 0.0f, 2.0f, 0.0f },
						{ -1.0f, 1.0f, -1.0f, 1.0f }
					};

					for (DWORD i = 0; i < 4; ++i)
					{
						FLOAT* vector = &buffer[i][0];
						for (DWORD j = 0; j < 4; ++j)
						{
							FLOAT sum = 0.0f;
							for (DWORD v = 0; v < 4; ++v)
								sum += mvp[v][j] * vector[v];

							vector[j] = sum;
						}
					}

					GLBufferData(GL_ARRAY_BUFFER, sizeof(buffer) << 1, NULL, GL_STATIC_DRAW);
					GLBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(buffer), buffer);
					GLBufferSubData(GL_ARRAY_BUFFER, sizeof(buffer), sizeof(buffer), buffer);
				}

				{
					GLEnableVertexAttribArray(0);
					GLVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 32, (GLvoid*)0);

					GLEnableVertexAttribArray(1);
					GLVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 32, (GLvoid*)16);
				}

				struct {
					GLuint back;
					GLuint primary;
				} textureId;

				if (!config.version && config.resHooked)
					GLGenTextures(2, &textureId.back);
				else
					GLGenTextures(1, &textureId.primary);
				{
					VOID* frameBuffer = AlignedAlloc(config.mode->width * config.mode->height * (isTrueColor ? sizeof(DWORD) : sizeof(WORD)));
					{
						if (!config.version && config.resHooked)
						{
							GLActiveTexture(GL_TEXTURE1);
							GLBindTexParameter(textureId.back, GL_LINEAR);
							GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

							MemoryZero(frameBuffer, config.mode->width * config.mode->height * sizeof(DWORD));
							LoadBack(frameBuffer, config.mode->width, config.mode->height);
							GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, config.mode->width, config.mode->height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
						}

						{
							GLActiveTexture(GL_TEXTURE0);
							GLBindTexParameter(textureId.primary, GL_LINEAR);

							if (isTrueColor)
								GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
							else
								GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
						}

						PixelBuffer* pixelBuffer = new PixelBuffer((Size*)config.mode, isTrueColor);
						{
							FpsCounter* fpsCounter = new FpsCounter(FPS_ACCURACY, frameBuffer);
							{
								this->viewport.refresh = TRUE;
								this->filterState.flags = TRUE;

								BOOL borderStatus = FALSE;
								Size zoomSize = { 0, 0 };
								BOOL isVSync = config.image.vSync;
								FpsState lastFpsState = fpsState;

								GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);
								if (WGLSwapInterval)
									WGLSwapInterval(isVSync);

								while (!this->isFinish)
								{
									OpenDrawSurface* surface = this->attachedSurface;
									if (!surface)
									{
										Sleep(0);
										continue;
									}

									StateBufferAligned* stateBuffer = (StateBufferAligned*)(!this->bufferIndex ? surface->indexBuffer : surface->secondaryBuffer);
									if (!stateBuffer)
									{
										Sleep(0);
										continue;
									}

									Size* frameSize = &stateBuffer->size;
									if (stateBuffer->isReady || this->viewport.refresh && frameSize->width && frameSize->height)
									{
										this->bufferIndex = !this->bufferIndex;

										stateBuffer = (StateBufferAligned*)(this->bufferIndex ? surface->indexBuffer : surface->secondaryBuffer);
										if (!stateBuffer)
										{
											Sleep(0);
											continue;
										}

										stateBuffer->isReady = FALSE;
										surface->drawEnabled = TRUE;

										fpsCounter->Init();
										if (lastFpsState != fpsState)
										{
											lastFpsState = fpsState;
											if (!lastFpsState)
												pixelBuffer->Reset();
										}

										FilterState state = this->filterState;
										this->filterState.flags = FALSE;

										if (stateBuffer->isBorder && frameSize->width == GAME_WIDTH && frameSize->height == GAME_HEIGHT && config.border.allowed)
											stateBuffer->isBorder = FALSE;

										BOOL isSwap = FALSE;
										VOID* stateData = stateBuffer->data;
										if (pixelBuffer->Update(stateBuffer) || state.flags || this->viewport.refresh || borderStatus != stateBuffer->isBorder || lastFpsState == FpsBenchmark)
										{
											if (isVSync != config.image.vSync)
											{
												isVSync = config.image.vSync;
												if (WGLSwapInterval)
													WGLSwapInterval(isVSync);
											}

											if (this->CheckView(TRUE))
												GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y + this->viewport.offset, this->viewport.rectangle.width, this->viewport.rectangle.height);

											if (state.flags || borderStatus != stateBuffer->isBorder)
											{
												borderStatus = stateBuffer->isBorder;

												ShaderProgram* program;
												switch (state.interpolation)
												{
												case InterpolateHermite:
													program = stateBuffer->isBorder ? &shaders.hermite_double : &shaders.hermite;
													break;
												case InterpolateCubic:
													program = stateBuffer->isBorder ? &shaders.cubic_double : &shaders.cubic;
													break;
												default:
													program = stateBuffer->isBorder ? &shaders.linear_double : &shaders.linear;
													break;
												}
												UseShaderProgram(program, texSize);

												if (state.flags)
												{
													if (stateBuffer->isBorder)
													{
														GLActiveTexture(GL_TEXTURE1);
														GLBindTexFilter(textureId.back, state.interpolation != InterpolateNearest ? GL_LINEAR : GL_NEAREST);
													}

													GLActiveTexture(GL_TEXTURE0);
													GLBindTexFilter(textureId.primary, state.interpolation == InterpolateLinear || state.interpolation == InterpolateHermite ? GL_LINEAR : GL_NEAREST);
												}
											}

											// Update FPS
											if (!this->isTakeSnapshot)
												fpsCounter->Draw(stateData);

											if (stateBuffer->isZoomed)
											{
												if (zoomSize.width != frameSize->width || zoomSize.height != frameSize->height)
												{
													zoomSize = *frameSize;

													FLOAT tw = (FLOAT)frameSize->width / maxTexSize;
													FLOAT th = (FLOAT)frameSize->height / maxTexSize;

													buffer[1][4] = tw;
													buffer[2][4] = tw;
													buffer[2][5] = th;
													buffer[3][5] = th;

													GLBufferSubData(GL_ARRAY_BUFFER, 5 * 8 * sizeof(FLOAT), 3 * 8 * sizeof(FLOAT), &buffer[1]);
												}

												GLDrawArrays(GL_TRIANGLE_FAN, 4, 4);
											}
											else
												GLDrawArrays(GL_TRIANGLE_FAN, 0, 4);

											isSwap = TRUE;
										}

										if (isSwap)
										{
											SwapBuffers(this->hDc);
											GLFinish();
											fpsCounter->Calculate();
										}
										else if (state.flags)
											this->filterState.flags = TRUE;

										if (this->isTakeSnapshot)
											this->TakeSnapshot(frameSize, stateData, isTrueColor);
									}

									WaitForSingleObject(this->hDrawEvent, INFINITE);
								}
							}
							delete fpsCounter;
						}
						delete pixelBuffer;
					}
					AlignedFree(frameBuffer);
				}
				if (!config.version && config.resHooked)
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

		++shaderProgram;
	} while (--count);
}

VOID OpenDraw::RenderNew()
{
	SetEvent(this->hCheckEvent);

	BOOL isTrueColor = config.mode->bpp != 16 || config.bpp32Hooked;
	DWORD maxTexSize = GetPow2(config.mode->width > config.mode->height ? config.mode->width : config.mode->height);

	FLOAT texWidth = config.mode->width == maxTexSize ? 1.0f : (FLOAT)config.mode->width / maxTexSize;
	FLOAT texHeight = config.mode->height == maxTexSize ? 1.0f : (FLOAT)config.mode->height / maxTexSize;

	DWORD texSize = (maxTexSize & 0xFFFF) | (maxTexSize << 16);

	const CHAR* xbrzVersion = config.bpp32Hooked ? GLSL_VER_1_30_ALPHA : GLSL_VER_1_30;

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
		{ 0, GLSL_VER_1_30, IDR_LINEAR_VERTEX, IDR_LINEAR_FRAGMENT },
		{ 0, GLSL_VER_1_30, IDR_LINEAR_VERTEX_DOUBLE, IDR_LINEAR_FRAGMENT_DOUBLE },
		{ 0, GLSL_VER_1_30, IDR_HERMITE_VERTEX, IDR_HERMITE_FRAGMENT },
		{ 0, GLSL_VER_1_30, IDR_HERMITE_VERTEX_DOUBLE, IDR_HERMITE_FRAGMENT_DOUBLE },
		{ 0, GLSL_VER_1_30, IDR_CUBIC_VERTEX, IDR_CUBIC_FRAGMENT },
		{ 0, GLSL_VER_1_30, IDR_CUBIC_VERTEX_DOUBLE, IDR_CUBIC_FRAGMENT_DOUBLE },
		{ 0, xbrzVersion, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_2X },
		{ 0, xbrzVersion, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_3X },
		{ 0, xbrzVersion, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_4X },
		{ 0, xbrzVersion, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_5X },
		{ 0, xbrzVersion, IDR_XBRZ_VERTEX, IDR_XBRZ_FRAGMENT_6X },
		{ 0, GLSL_VER_1_30, IDR_SCALEHQ_VERTEX_2X, IDR_SCALEHQ_FRAGMENT_2X },
		{ 0, GLSL_VER_1_30, IDR_SCALEHQ_VERTEX_4X, IDR_SCALEHQ_FRAGMENT_4X },
		{ 0, GLSL_VER_1_30, IDR_XSAL_VERTEX, IDR_XSAL_FRAGMENT },
		{ 0, GLSL_VER_1_30, IDR_EAGLE_VERTEX, IDR_EAGLE_FRAGMENT },
		{ 0, GLSL_VER_1_30, IDR_SCALENX_VERTEX_2X, IDR_SCALENX_FRAGMENT_2X },
		{ 0, GLSL_VER_1_30, IDR_SCALENX_VERTEX_3X, IDR_SCALENX_FRAGMENT_3X }
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
						FLOAT buffer[8][8] = {
							{ 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
							{ (FLOAT)config.mode->width, 0.0f, 0.0f, 1.0f, texWidth, 0.0f, texWidth, 0.0f },
							{ (FLOAT)config.mode->width, (FLOAT)config.mode->height, 0.0f, 1.0f, texWidth, texHeight, texWidth, texHeight },
							{ 0.0f, (FLOAT)config.mode->height, 0.0f, 1.0f, 0.0f, texHeight, 0.0f, texHeight },

							{ 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f },
							{ (FLOAT)config.mode->width, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
							{ (FLOAT)config.mode->width, (FLOAT)config.mode->height, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f },
							{ 0.0f, (FLOAT)config.mode->height, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f }
						};

						{
							FLOAT mvp[4][4] = {
								{ FLOAT(2.0f / config.mode->width), 0.0f, 0.0f, 0.0f },
								{ 0.0f, FLOAT(-2.0f / config.mode->height), 0.0f, 0.0f },
								{ 0.0f, 0.0f, 2.0f, 0.0f },
								{ -1.0f, 1.0f, -1.0f, 1.0f }
							};

							for (DWORD i = 0; i < 8; ++i)
							{
								FLOAT* vector = &buffer[i][0];
								for (DWORD j = 0; j < 4; ++j)
								{
									FLOAT sum = 0.0f;
									for (DWORD v = 0; v < 4; ++v)
										sum += mvp[v][j] * vector[v];

									vector[j] = sum;
								}
							}

							GLBufferData(GL_ARRAY_BUFFER, sizeof(buffer) << 1, buffer, GL_STATIC_DRAW);

							GLBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(buffer) >> 1, buffer);
							GLBufferSubData(GL_ARRAY_BUFFER, sizeof(buffer) >> 1, sizeof(buffer) >> 1, buffer);
							GLBufferSubData(GL_ARRAY_BUFFER, sizeof(buffer), sizeof(buffer) >> 1, &buffer[4]);
							GLBufferSubData(GL_ARRAY_BUFFER, (sizeof(buffer) >> 1) * 3, sizeof(buffer) >> 1, &buffer[4]);
						}

						{
							GLEnableVertexAttribArray(0);
							GLVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 32, (GLvoid*)0);

							GLEnableVertexAttribArray(1);
							GLVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 32, (GLvoid*)16);
						}

						struct {
							GLuint back;
							GLuint primary;
							GLuint secondary;
							GLuint primaryBO;
							GLuint backBO;
						} textureId;

						if (!config.version && config.resHooked)
							GLGenTextures(2, &textureId.back);
						else
							GLGenTextures(1, &textureId.primary);
						{
							VOID* frameBuffer = AlignedAlloc(config.mode->width * config.mode->height * (isTrueColor ? sizeof(DWORD) : sizeof(WORD)));
							{
								if (!config.version && config.resHooked)
								{
									GLActiveTexture(GL_TEXTURE1);
									GLBindTexParameter(textureId.back, GL_LINEAR);
									GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

									MemoryZero(frameBuffer, config.mode->width * config.mode->height * sizeof(DWORD));
									LoadBack(frameBuffer, config.mode->width, config.mode->height);
									GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, config.mode->width, config.mode->height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
								}

								{
									GLActiveTexture(GL_TEXTURE0);
									GLBindTexParameter(textureId.primary, GL_LINEAR);

									if (isTrueColor)
										GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
									else
										GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
								}

								PixelBuffer* pixelBuffer = new PixelBuffer((Size*)config.mode, isTrueColor);
								{
									FpsCounter* fpsCounter = new FpsCounter(FPS_ACCURACY, frameBuffer);
									{
										this->viewport.refresh = TRUE;
										this->filterState.flags = TRUE;

										DWORD viewSize = 0;
										GLuint fboId = 0;
										GLuint rboId = 0;
										BOOL activeIndex = TRUE;
										BOOL zoomStatus = FALSE;
										BOOL borderStatus = FALSE;
										ShaderProgram* upscaleProgram;
										Size zoomSize = { 0, 0 };
										Size zoomFbSize = { 0, 0 };
										BOOL isVSync = config.image.vSync;
										FpsState lastFpsState = fpsState;

										GLClearColor(0.0f, 0.0f, 0.0f, 1.0f);
										if (WGLSwapInterval)
											WGLSwapInterval(isVSync);

										while (!this->isFinish)
										{
											OpenDrawSurface* surface = this->attachedSurface;
											if (!surface)
											{
												Sleep(0);
												continue;
											}

											StateBufferAligned* stateBuffer = (StateBufferAligned*)(!this->bufferIndex ? surface->indexBuffer : surface->secondaryBuffer);
											if (!stateBuffer)
											{
												Sleep(0);
												continue;
											}

											Size* frameSize = &stateBuffer->size;
											if (stateBuffer->isReady || this->viewport.refresh && frameSize->width && frameSize->height)
											{
												this->bufferIndex = !this->bufferIndex;

												stateBuffer = (StateBufferAligned*)(this->bufferIndex ? surface->indexBuffer : surface->secondaryBuffer);
												if (!stateBuffer)
												{
													Sleep(0);
													continue;
												}

												stateBuffer->isReady = FALSE;
												surface->drawEnabled = TRUE;

												fpsCounter->Init();
												if (lastFpsState != fpsState)
												{
													lastFpsState = fpsState;
													if (!lastFpsState)
														pixelBuffer->Reset();
												}

												FilterState state = this->filterState;
												this->filterState.flags = FALSE;

												if (stateBuffer->isBorder && frameSize->width == GAME_WIDTH && frameSize->height == GAME_HEIGHT && config.border.allowed)
													stateBuffer->isBorder = FALSE;

												BOOL isSwap = FALSE;
												VOID* stateData = stateBuffer->data;
												if (state.upscaling)
												{
													if (!fboId)
														GLGenFramebuffers(1, &fboId);

													DWORD newSize = MAKELONG(config.mode->width * state.value, config.mode->height * state.value);
													if (state.flags && newSize != viewSize)
														pixelBuffer->Reset();

													if (pixelBuffer->Check(stateBuffer) || state.flags || this->viewport.refresh || borderStatus != stateBuffer->isBorder || lastFpsState == FpsBenchmark)
													{
														if (isVSync != config.image.vSync)
														{
															isVSync = config.image.vSync;
															if (WGLSwapInterval)
																WGLSwapInterval(isVSync);
														}

														if (state.flags || borderStatus != stateBuffer->isBorder)
														{
															borderStatus = stateBuffer->isBorder;
															this->viewport.refresh = TRUE;
														}

														FLOAT kw = (FLOAT)frameSize->width / config.mode->width;
														FLOAT kh = (FLOAT)frameSize->height / config.mode->height;

														GLBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboId);
														{
															if (state.flags)
															{
																switch (state.upscaling)
																{
																case UpscaleScaleNx:
																	switch (state.value)
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
																	switch (state.value)
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
																	switch (state.value)
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
																	upscaleProgram = &shaders.xSal_2x;

																	break;

																default:
																	upscaleProgram = &shaders.eagle_2x;

																	break;
																}

																if (newSize != viewSize)
																{
																	viewSize = newSize;

																	if (!rboId)
																	{
																		GLGenRenderbuffers(1, &rboId);
																		{
																			GLBindRenderbuffer(GL_RENDERBUFFER, rboId);
																			GLRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, LOWORD(viewSize), HIWORD(viewSize));
																			GLBindRenderbuffer(GL_RENDERBUFFER, NULL);
																			GLFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboId);
																		}

																		GLGenTextures(!config.version && config.resHooked ? 3 : 2, (GLuint*)&textureId.secondary);
																		{
																			GLBindTexParameter(textureId.secondary, GL_LINEAR);

																			if (isTrueColor)
																				GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
																			else
																				GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTexSize, maxTexSize, GL_NONE, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
																		}
																	}

																	// Gen texture
																	DWORD idx = !config.version && config.resHooked ? 2 : 1;
																	while (idx--)
																	{
																		GLBindTexParameter(((GLuint*)&textureId.primaryBO)[idx], GL_LINEAR);
																		GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, LOWORD(viewSize), HIWORD(viewSize), GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
																	}
																}

																UseShaderProgram(upscaleProgram, texSize);

																if (!config.version && config.resHooked)
																{
																	GLViewport(0, 0, LOWORD(viewSize), HIWORD(viewSize));

																	GLFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId.backBO, 0);

																	GLActiveTexture(GL_TEXTURE1);
																	GLBindTexFilter(((GLuint*)&textureId.primary)[activeIndex], GL_LINEAR);

																	MemoryZero(frameBuffer, config.mode->width * config.mode->height * sizeof(DWORD));
																	GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, config.mode->width, config.mode->height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);

																	GLActiveTexture(GL_TEXTURE0);
																	GLBindTexFilter(textureId.back, GL_LINEAR);

																	GLDrawArrays(GL_TRIANGLE_FAN, 0, 4);
																	GLFinish();
																}

																GLFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId.primaryBO, 0);
															}

															if (stateBuffer->isZoomed)
																GLViewport(0, 0, DWORD(kw * LOWORD(viewSize)), DWORD(kh * HIWORD(viewSize)));
															else
																GLViewport(0, 0, LOWORD(viewSize), HIWORD(viewSize));

															GLActiveTexture(GL_TEXTURE1);
															GLBindTexFilter(((GLuint*)&textureId.primary)[activeIndex], GL_LINEAR);

															if (this->CheckView(FALSE) || zoomStatus != stateBuffer->isZoomed || stateBuffer->isZoomed && (zoomSize.width != frameSize->width || zoomSize.height != frameSize->height))
															{
																zoomStatus = stateBuffer->isZoomed;

																if (isTrueColor)
																{
																	DWORD* ptr = (DWORD*)frameBuffer;
																	DWORD count = frameSize->width * frameSize->height;
																	do
																		*ptr++ = 0x00FFFFFF;
																	while (--count);
																	GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameSize->width, frameSize->height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
																}
																else
																{
																	MemoryZero(frameBuffer, frameSize->width * frameSize->height * (isTrueColor ? sizeof(DWORD) : sizeof(WORD)));
																	GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameSize->width, frameSize->height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, frameBuffer);
																}
															}

															activeIndex = !activeIndex;
															GLActiveTexture(GL_TEXTURE0);
															GLBindTexFilter(((GLuint*)&textureId.primary)[activeIndex], GL_LINEAR);

															{
																if (isTrueColor)
																	GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameSize->width, frameSize->height, GL_RGBA, GL_UNSIGNED_BYTE, stateData);
																else
																	GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameSize->width, frameSize->height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, stateData);

																// Update FPS
																if (!this->isTakeSnapshot)
																	fpsCounter->Draw(stateData);

																// Draw into FBO texture
																if (stateBuffer->isZoomed)
																{
																	if (zoomSize.width != frameSize->width || zoomSize.height != frameSize->height)
																	{
																		zoomSize = *frameSize;

																		FLOAT tw = (FLOAT)frameSize->width / maxTexSize;
																		FLOAT th = (FLOAT)frameSize->height / maxTexSize;

																		buffer[1][4] = tw;
																		buffer[2][4] = tw;
																		buffer[2][5] = th;
																		buffer[3][5] = th;

																		GLBufferSubData(GL_ARRAY_BUFFER, 5 * 8 * sizeof(FLOAT), 3 * 8 * sizeof(FLOAT), &buffer[1]);
																	}

																	GLDrawArrays(GL_TRIANGLE_FAN, 4, 4);
																}
																else
																	GLDrawArrays(GL_TRIANGLE_FAN, 0, 4);
															}

															GLFinish();
														}
														GLBindFramebuffer(GL_DRAW_FRAMEBUFFER, NULL);

														ShaderProgram* program;
														switch (state.interpolation)
														{
														case InterpolateHermite:
															program = stateBuffer->isBorder ? &shaders.hermite_double : &shaders.hermite;
															break;
														case InterpolateCubic:
															program = stateBuffer->isBorder ? &shaders.cubic_double : &shaders.cubic;
															break;
														default:
															program = stateBuffer->isBorder ? &shaders.linear_double : &shaders.linear;
															break;
														}

														UseShaderProgram(program, texSize);
														{
															GLClear(GL_COLOR_BUFFER_BIT);
															GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y + this->viewport.offset, this->viewport.rectangle.width, this->viewport.rectangle.height);

															if (stateBuffer->isBorder)
															{
																GLActiveTexture(GL_TEXTURE1);
																GLBindTexFilter(textureId.backBO, state.interpolation != InterpolateNearest ? GL_LINEAR : GL_NEAREST);
															}

															GLActiveTexture(GL_TEXTURE0);
															GLBindTexFilter(textureId.primaryBO, state.interpolation == InterpolateLinear || state.interpolation == InterpolateHermite ? GL_LINEAR : GL_NEAREST);

															if (stateBuffer->isZoomed)
															{
																if (zoomFbSize.width != frameSize->width || zoomFbSize.height != frameSize->height)
																{
																	zoomFbSize = *frameSize;

																	buffer[4][5] = kh;
																	buffer[5][4] = kw;
																	buffer[5][5] = kh;
																	buffer[6][4] = kw;

																	GLBufferSubData(GL_ARRAY_BUFFER, 12 * 8 * sizeof(FLOAT), 3 * 8 * sizeof(FLOAT), &buffer[4]);
																}

																GLDrawArrays(GL_TRIANGLE_FAN, 12, 4);
															}
															else
																GLDrawArrays(GL_TRIANGLE_FAN, 8, 4);
														}
														UseShaderProgram(upscaleProgram, texSize);
														isSwap = TRUE;
													}
												}
												else
												{
													if (fboId)
													{
														GLDeleteTextures(!config.version && config.resHooked ? 3 : 2, (GLuint*)&textureId.secondary);
														GLDeleteRenderbuffers(1, &rboId);
														GLDeleteFramebuffers(1, &fboId);

														viewSize = 0;
														rboId = 0;
														fboId = 0;

														pixelBuffer->Reset();

														if (!config.version && config.resHooked)
														{
															GLActiveTexture(GL_TEXTURE1);
															GLBindTexFilter(textureId.back, state.interpolation != InterpolateNearest ? GL_LINEAR : GL_NEAREST);
														}

														GLActiveTexture(GL_TEXTURE0);
														GLBindTexFilter(textureId.primary, state.interpolation == InterpolateLinear || state.interpolation == InterpolateHermite ? GL_LINEAR : GL_NEAREST);
													}

													if (pixelBuffer->Update(stateBuffer) || state.flags || this->viewport.refresh || borderStatus != stateBuffer->isBorder || lastFpsState == FpsBenchmark)
													{
														if (isVSync != config.image.vSync)
														{
															isVSync = config.image.vSync;
															if (WGLSwapInterval)
																WGLSwapInterval(isVSync);
														}

														if (state.flags)
															this->viewport.refresh = TRUE;

														if (this->CheckView(TRUE))
															GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y + this->viewport.offset, this->viewport.rectangle.width, this->viewport.rectangle.height);

														if (state.flags || borderStatus != stateBuffer->isBorder)
														{
															borderStatus = stateBuffer->isBorder;

															ShaderProgram* program;
															switch (state.interpolation)
															{
															case InterpolateHermite:
																program = stateBuffer->isBorder ? &shaders.hermite_double : &shaders.hermite;
																break;
															case InterpolateCubic:
																program = stateBuffer->isBorder ? &shaders.cubic_double : &shaders.cubic;
																break;
															default:
																program = stateBuffer->isBorder ? &shaders.linear_double : &shaders.linear;
																break;
															}

															UseShaderProgram(program, texSize);

															if (state.flags)
															{
																if (stateBuffer->isBorder)
																{
																	GLActiveTexture(GL_TEXTURE1);
																	GLBindTexFilter(textureId.back, state.interpolation != InterpolateNearest ? GL_LINEAR : GL_NEAREST);
																}

																GLActiveTexture(GL_TEXTURE0);
																GLBindTexFilter(textureId.primary, state.interpolation == InterpolateLinear || state.interpolation == InterpolateHermite ? GL_LINEAR : GL_NEAREST);
															}
														}

														// Update FPS
														if (!this->isTakeSnapshot)
															fpsCounter->Draw(stateData);

														if (stateBuffer->isZoomed)
														{
															if (zoomSize.width != frameSize->width || zoomSize.height != frameSize->height)
															{
																zoomSize = *frameSize;

																FLOAT tw = (FLOAT)frameSize->width / maxTexSize;
																FLOAT th = (FLOAT)frameSize->height / maxTexSize;

																buffer[1][4] = tw;
																buffer[2][4] = tw;
																buffer[2][5] = th;
																buffer[3][5] = th;

																GLBufferSubData(GL_ARRAY_BUFFER, 5 * 8 * sizeof(FLOAT), 3 * 8 * sizeof(FLOAT), &buffer[1]);
															}

															GLDrawArrays(GL_TRIANGLE_FAN, 4, 4);
														}
														else
															GLDrawArrays(GL_TRIANGLE_FAN, 0, 4);

														isSwap = TRUE;
													}
												}

												if (isSwap)
												{
													SwapBuffers(this->hDc);
													GLFinish();
													fpsCounter->Calculate();
												}
												else if (state.flags)
													this->filterState.flags = TRUE;

												if (this->isTakeSnapshot)
													this->TakeSnapshot(frameSize, stateData, isTrueColor);
											}

											WaitForSingleObject(this->hDrawEvent, INFINITE);
										}

										if (fboId)
										{
											GLDeleteTextures(!config.version && config.resHooked ? 3 : 2, (GLuint*)&textureId.secondary);
											GLDeleteRenderbuffers(1, &rboId);
											GLDeleteFramebuffers(1, &fboId);
										}
									}
									delete fpsCounter;
								}
								delete pixelBuffer;
							}
							AlignedFree(frameBuffer);
						}
						if (!config.version && config.resHooked)
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

		++shaderProgram;
	} while (--count);
}

VOID OpenDraw::LoadFilterState()
{
	FilterState state;
	state.interpolation = config.image.interpolation;
	state.upscaling = config.image.upscaling;

	switch (state.upscaling)
	{
	case UpscaleScaleNx:
		state.value = config.image.scaleNx;
		break;

	case UpscaleScaleHQ:
		state.value = config.image.scaleHQ;
		break;

	case UpscaleXRBZ:
		state.value = config.image.xBRz;
		break;

	case UpscaleXSal:
		state.value = config.image.xSal;
		break;

	case UpscaleEagle:
		state.value = config.image.eagle;
		break;

	default:
		state.value = 0;
		break;
	}

	state.flags = TRUE;
	this->filterState = state;
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
		if (!config.windowedMode && !config.borderless.mode)
			this->hDraw = CreateWindowEx(
				WS_EX_CONTROLPARENT | WS_EX_TOPMOST,
				WC_DRAW,
				NULL,
				WS_VISIBLE | WS_POPUP | WS_MAXIMIZE,
				0, 0,
				rect.right, rect.bottom,
				this->hWnd,
				NULL,
				hDllModule,
				NULL);
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
	}

	this->viewport.width = rect.right;
	this->viewport.height = rect.bottom - this->viewport.offset;
	this->viewport.refresh = TRUE;

	DWORD threadId;
	SECURITY_ATTRIBUTES sAttribs;
	MemoryZero(&sAttribs, sizeof(SECURITY_ATTRIBUTES));
	sAttribs.nLength = sizeof(SECURITY_ATTRIBUTES);
	this->hDrawThread = CreateThread(&sAttribs, NULL, RenderThread, this, HIGH_PRIORITY_CLASS, &threadId);

	WaitForSingleObject(this->hCheckEvent, INFINITE);
	Window::CheckMenu(MenuAspect);
	Window::CheckMenu(MenuVSync);
	Window::CheckMenu(MenuInterpolate);
	Window::CheckMenu(MenuUpscale);
	Window::CheckMenu(MenuStretch);
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

	if (this->hDraw != this->hWnd)
	{
		DestroyWindow(this->hDraw);
		GL::ResetPixelFormat();
	}

	this->hDraw = NULL;

	/*SetClassLongPtr(this->hWnd, GCLP_HBRBACKGROUND, NULL);
	RedrawWindow(this->hWnd, NULL, NULL, RDW_INVALIDATE);*/

	ClipCursor(NULL);
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

		HWND hActive = GetForegroundWindow();
		if (config.image.aspect && !config.windowedMode && config.borderless.mode == config.borderless.real && (hActive == this->hWnd || hActive == this->hDraw))
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
	if (!Config::IsZoomed())
	{
		p->x = LONG((FLOAT)((p->x - this->viewport.rectangle.x) * config.mode->width) / this->viewport.rectangle.width);
		p->y = LONG((FLOAT)((p->y - this->viewport.rectangle.y) * config.mode->height) / this->viewport.rectangle.height);
	}
	else
	{
		FLOAT kx = config.zoom.sizeFloat.width / config.mode->width;
		FLOAT ky = config.zoom.sizeFloat.height / config.mode->height;

		p->x = LONG((FLOAT)((p->x - this->viewport.rectangle.x) * config.mode->width) / this->viewport.rectangle.width * kx) + ((config.mode->width - config.zoom.size.width) >> 1);
		p->y = LONG((FLOAT)((p->y - this->viewport.rectangle.y) * config.mode->height) / this->viewport.rectangle.height * ky) + ((config.mode->height - config.zoom.size.height) >> 1);
	}
}

OpenDraw::OpenDraw(IDrawUnknown** list)
	: IDraw7(list)
{
	this->surfaceEntries = NULL;
	this->clipperEntries = NULL;
	this->paletteEntries = NULL;

	this->attachedSurface = NULL;

	this->hDc = NULL;
	this->hWnd = NULL;
	this->hDraw = NULL;

	this->isFinish = TRUE;
	this->LoadFilterState();
	this->isTakeSnapshot = SnapshotNone;
	this->bufferIndex = FALSE;

	this->flushTime = 0.0;

	MemoryZero(&this->windowPlacement, sizeof(WINDOWPLACEMENT));
	this->hDrawEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	this->hCheckEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

OpenDraw::~OpenDraw()
{
	this->RenderStop();
	CloseHandle(this->hDrawEvent);
	CloseHandle(this->hCheckEvent);
}

VOID OpenDraw::SetFullscreenMode()
{
	if (config.windowedMode)
		GetWindowPlacement(hWnd, &this->windowPlacement);

	this->viewport.offset = config.borderless.mode ? BORDERLESS_OFFSET : 0;
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
		surface = new OpenDrawSurface((IDrawUnknown**)&this->surfaceEntries, this, SurfaceSecondary, this->attachedSurface);
		surface->CreateBuffer(lpDDSurfaceDesc->dwWidth, lpDDSurfaceDesc->dwHeight, config.mode->bpp, buffer);
	}
	else
	{
		surface = new OpenDrawSurface((IDrawUnknown**)&this->surfaceEntries, this, (lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) ? SurfacePrimary : SurfaceOther, NULL);

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