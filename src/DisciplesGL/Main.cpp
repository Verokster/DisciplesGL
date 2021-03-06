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
#include "Main.h"
#include "Config.h"
#include "Hooks.h"
#include "PngLib.h"
#include "Resource.h"

OpenDraw* drawList;

namespace Main
{
	HRESULT __stdcall DrawEnumerateEx(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags)
	{
		static const GUID id = { 0x51221AA6, 0xC5DA, 0x468F, 0x82, 0x31, 0x68, 0x0E, 0xC9, 0x03, 0xA3, 0xB8 };
		lpCallback((GUID*)&id, "OpenGL Wrapper", "OpenGL Wrapper", lpContext, NULL);
		return DD_OK;
	}

	HRESULT __stdcall DrawCreate(GUID* lpGUID, LPDIRECTDRAW* lplpDD, IUnknown* pUnkOuter)
	{
		*lplpDD = (LPDIRECTDRAW) new OpenDraw((IDrawUnknown**)&drawList);
		return DD_OK;
	}

	HRESULT __stdcall DrawCreateEx(GUID* lpGuid, LPVOID* lplpDD, REFIID iid, IUnknown* pUnkOuter)
	{
		*lplpDD = new OpenDraw((IDrawUnknown**)&drawList);
		return DD_OK;
	}

	OpenDraw* FindOpenDrawByWindow(HWND hWnd)
	{
		OpenDraw* ddraw = drawList;
		while (ddraw)
		{
			if (ddraw->hWnd == hWnd || ddraw->hDraw == hWnd)
				return ddraw;

			ddraw = (OpenDraw*)ddraw->last;
		}

		return NULL;
	}

	BOOL LoadResource(LPCSTR name, Stream* stream)
	{
		HGLOBAL hResourceData;
		HRSRC hResource = FindResource(hDllModule, name, RT_RCDATA);
		if (hResource)
		{
			hResourceData = LoadResource(hDllModule, hResource);
			if (hResourceData)
			{
				stream->data = (BYTE*)LockResource(hResourceData);
				if (stream->data)
				{
					stream->size = SizeofResource(hDllModule, hResource);
					return TRUE;
				}
			}
		}

		return FALSE;
	}

	VOID ShowError(UINT id, CHAR* file, DWORD line)
	{
		CHAR message[256];
		LoadString(hDllModule, id, message, sizeof(message));
		ShowError(message, file, line);
	}

	VOID ShowError(CHAR* message, CHAR* file, DWORD line)
	{
		CHAR dest[400];
		StrPrint(dest, "%s\n\n\nFILE %s\nLINE %d", message, file, line);

		Hooks::MessageBoxHook(NULL, dest, "Error", MB_OK | MB_ICONERROR | MB_TASKMODAL);

		Exit(EXIT_FAILURE);
	}

	VOID ShowInfo(UINT id)
	{
		CHAR message[256];
		LoadString(hDllModule, id, message, sizeof(message));
		ShowInfo(message);
	}

	VOID ShowInfo(CHAR* message)
	{
		Hooks::MessageBoxHook(NULL, message, "Information", MB_OK | MB_ICONASTERISK | MB_TASKMODAL);
	}

	BOOL ShowWarn(UINT id)
	{
		CHAR message[256];
		LoadString(hDllModule, id, message, sizeof(message));
		return ShowWarn(message);
	}

	BOOL ShowWarn(CHAR* message)
	{
		return Hooks::MessageBoxHook(NULL, message, "Warning", MB_OKCANCEL | MB_ICONWARNING | MB_TASKMODAL) == IDOK;
	}

#ifdef _DEBUG
	VOID CheckError(CHAR* file, DWORD line)
	{
		DWORD statusCode = GLGetError();

		CHAR* message;

		if (statusCode != GL_NO_ERROR)
		{
			switch (statusCode)
			{
			case GL_INVALID_ENUM:
				message = "GL_INVALID_ENUM";
				break;

			case GL_INVALID_VALUE:
				message = "GL_INVALID_VALUE";
				break;

			case GL_INVALID_OPERATION:
				message = "GL_INVALID_OPERATION";
				break;

			case GL_INVALID_FRAMEBUFFER_OPERATION:
				message = "GL_INVALID_FRAMEBUFFER_OPERATION";
				break;

			case GL_OUT_OF_MEMORY:
				message = "GL_OUT_OF_MEMORY";
				break;

			case GL_STACK_UNDERFLOW:
				message = "GL_STACK_UNDERFLOW";
				break;

			case GL_STACK_OVERFLOW:
				message = "GL_STACK_OVERFLOW";
				break;

			default:
				message = "GL_UNDEFINED";
				break;
			}

			ShowError(message, file, line);
		}
	}
#endif

	VOID LoadBack(VOID* buffer, DWORD width, DWORD height)
	{
		Stream stream = {};
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
							while (count)
							{
								--count;

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
								while (count)
								{
									--count;
									*dst++ = *src++;
									*dst++ = *src++;
									*dst++ = *src++;
									*dst++ = 0xFF;
								}

								if (config.renderer == RendererGDI)
								{
									DWORD* pal = palette;
									count = (DWORD)info_ptr->num_palette;
									while (count)
									{
										--count;
										*pal++ = _byteswap_ulong(_rotl(*pal, 8));
									}
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
							while (divH)
							{
								--divH;
								DWORD cheight = !divH && modY ? modY : (DWORD)infoHeight;

								DWORD* dstW = dstH;

								DWORD divW = divX;
								while (divW)
								{
									--divW;
									DWORD cwidth = !divW && modX ? modX : (DWORD)infoWidth;

									BYTE* srcData = data + bytesInRow * startY;
									DWORD* dstData = dstW;

									DWORD copyY = startY;
									DWORD copyHeight = cheight;
									while (copyHeight)
									{
										--copyHeight;
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
											while (copyWidth)
											{
												--copyWidth;
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
											while (copyWidth)
											{
												--copyWidth;
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
}