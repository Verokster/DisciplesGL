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
#include "Main.h"
#include "Config.h"
#include "Hooks.h"

OpenDraw* drawList;

namespace Main
{
	HRESULT __stdcall DrawEnumerateEx(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags)
	{
		GUID id = { 0x51221AA6, 0xC5DA, 0x468F, 0x82, 0x31, 0x68, 0x0E, 0xC9, 0x03, 0xA3, 0xB8 };
		lpCallback(&id, "OpenGL Wrapper", "OpenGL Wrapper", lpContext, NULL);
		return DD_OK;
	}

	HRESULT __stdcall DrawCreate(GUID* lpGUID, LPDIRECTDRAW* lplpDD, IUnknown* pUnkOuter)
	{
		*lplpDD = (LPDIRECTDRAW)new OpenDraw((IDrawUnknown**)&drawList);
		return DD_OK;
	}

	HRESULT __stdcall DrawCreateEx(GUID* lpGuid, LPVOID* lplpDD, REFIID iid, IUnknown* pUnkOuter)
	{
		*lplpDD = new OpenDraw((IDrawUnknown**)&drawList);
		return DD_OK;
	}

	OpenDraw* __fastcall FindOpenDrawByWindow(HWND hWnd)
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

	VOID __fastcall ShowError(UINT id, CHAR* file, DWORD line)
	{
		CHAR message[256];
		LoadString(hDllModule, id, message, sizeof(message));
		ShowError(message, file, line);
	}

	VOID __fastcall ShowError(CHAR* message, CHAR* file, DWORD line)
	{
		CHAR dest[400];
		StrPrint(dest, "%s\n\n\nFILE %s\nLINE %d", message, file, line);

		ULONG_PTR cookie = NULL;
		if (hActCtx && hActCtx != INVALID_HANDLE_VALUE && !ActivateActCtxC(hActCtx, &cookie))
			cookie = NULL;

		MessageBox(NULL, dest, "Error", MB_OK | MB_ICONERROR);

		if (cookie)
			DeactivateActCtxC(0, cookie);

		Exit(EXIT_FAILURE);
	}

	VOID __fastcall ShowInfo(UINT id)
	{
		CHAR message[256];
		LoadString(hDllModule, id, message, sizeof(message));
		ShowInfo(message);
	}

	VOID __fastcall ShowInfo(CHAR* message)
	{
		ULONG_PTR cookie = NULL;
		if (hActCtx && hActCtx != INVALID_HANDLE_VALUE && !ActivateActCtxC(hActCtx, &cookie))
			cookie = NULL;

		MessageBox(NULL, message, "Information", MB_OK | MB_ICONASTERISK);

		if (cookie)
			DeactivateActCtxC(0, cookie);
	}

#ifdef _DEBUG
	VOID __fastcall CheckError(CHAR* file, DWORD line)
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
}