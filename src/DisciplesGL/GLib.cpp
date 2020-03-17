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
#include "Resource.h"
#include "GLib.h"
#include "Config.h"

#define PREFIX_GL "gl"
#define PREFIX_WGL "wgl"

WGLCREATECONTEXTATTRIBS WGLCreateContextAttribs;
WGLCHOOSEPIXELFORMAT WGLChoosePixelFormat;
WGLGETEXTENSIONSSTRING WGLGetExtensionsString;
WGLSWAPINTERVAL WGLSwapInterval;

GLGETSTRING GLGetString;
GLVERTEX2S GLVertex2s;
GLTEXCOORD2F GLTexCoord2f;
GLBEGIN GLBegin;
GLEND GLEnd;
GLVIEWPORT GLViewport;
GLMATRIXMODE GLMatrixMode;
GLLOADIDENTITY GLLoadIdentity;
GLORTHO GLOrtho;
GLFINISH GLFinish;
GLENABLE GLEnable;
GLDISABLE GLDisable;
GLBINDTEXTURE GLBindTexture;
GLDELETETEXTURES GLDeleteTextures;
GLTEXPARAMETERI GLTexParameteri;
GLTEXENVI GLTexEnvi;
GLGETTEXIMAGE GLGetTexImage;
GLTEXIMAGE2D GLTexImage2D;
GLTEXSUBIMAGE2D GLTexSubImage2D;
GLGENTEXTURES GLGenTextures;
GLGETINTEGERV GLGetIntegerv;
GLCLEAR GLClear;
GLCLEARCOLOR GLClearColor;
GLBLENDFUNC GLBlendFunc;
GLREADPIXELS GLReadPixels;
GLPIXELSTOREI GLPixelStorei;

#ifdef _DEBUG
GLGETERROR GLGetError;
#endif

GLACTIVETEXTURE GLActiveTexture;
GLGENBUFFERS GLGenBuffers;
GLDELETEBUFFERS GLDeleteBuffers;
GLBINDBUFFER GLBindBuffer;
GLBUFFERDATA GLBufferData;
GLBUFFERSUBDATA GLBufferSubData;
GLDRAWARRAYS GLDrawArrays;

GLENABLEVERTEXATTRIBARRAY GLEnableVertexAttribArray;
GLVERTEXATTRIBPOINTER GLVertexAttribPointer;

GLCREATESHADER GLCreateShader;
GLDELETESHADER GLDeleteShader;
GLDELETEPROGRAM GLDeleteProgram;
GLCREATEPROGRAM GLCreateProgram;
GLSHADERSOURCE GLShaderSource;
GLCOMPILESHADER GLCompileShader;
GLATTACHSHADER GLAttachShader;
GLDETACHSHADER GLDetachShader;
GLLINKPROGRAM GLLinkProgram;
GLUSEPROGRAM GLUseProgram;
GLGETSHADERIV GLGetShaderiv;
GLGETSHADERINFOLOG GLGetShaderInfoLog;

GLBINDATTRIBLOCATION GLBindAttribLocation;
GLGETUNIFORMLOCATION GLGetUniformLocation;

GLUNIFORM1I GLUniform1i;
GLUNIFORM1F GLUniform1f;
GLUNIFORM2F GLUniform2f;
GLUNIFORM4F GLUniform4f;

GLGENVERTEXARRAYS GLGenVertexArrays;
GLBINDVERTEXARRAY GLBindVertexArray;
GLDELETEVERTEXARRAYS GLDeleteVertexArrays;

GLGENFRAMEBUFFERS GLGenFramebuffers;
GLDELETEFRAMEBUFFERS GLDeleteFramebuffers;
GLBINDFRAMEBUFFER GLBindFramebuffer;
GLFRAMEBUFFERTEXTURE2D GLFramebufferTexture2D;

GLGENRENDERBUFFERS GLGenRenderbuffers;
GLDELETERENDERBUFFERS GLDeleteRenderbuffers;
GLBINDRENDERBUFFER GLBindRenderbuffer;
GLRENDERBUFFERSTORAGE GLRenderbufferStorage;
GLFRAMEBUFFERRENDERBUFFER GLFramebufferRenderbuffer;

HMODULE hGLModule;

namespace GL
{
#pragma optimize("s", on)
	VOID __fastcall LoadFunction(CHAR* buffer, const CHAR* prefix, const CHAR* name, PROC* func, const CHAR* sufix = NULL)
	{
		StrCopy(buffer, prefix);
		StrCat(buffer, name);

		if (sufix)
			StrCat(buffer, sufix);

		*func = wglGetProcAddress(buffer);
		if ((INT)*func >= -1 && (INT)*func <= 3)
		{
			if (!hGLModule)
				hGLModule = GetModuleHandle("OPENGL32.dll");
			*func = GetProcAddress(hGLModule, buffer);

			if (!*func && !sufix)
			{
				LoadFunction(buffer, prefix, name, func, "EXT");
				if (!*func)
					LoadFunction(buffer, prefix, name, func, "ARB");
			}
		}
	}

	BOOL __fastcall GetContext(HDC hDc, HGLRC* lpHRc, DWORD major, DWORD minor, BOOL showError)
	{
		DWORD wglAttributes[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, major,
			WGL_CONTEXT_MINOR_VERSION_ARB, minor,
			0
		};

		HGLRC hRc = WGLCreateContextAttribs(hDc, NULL, wglAttributes);
		if (hRc)
		{
			wglMakeCurrent(hDc, hRc);
			wglDeleteContext(*lpHRc);
			*lpHRc = hRc;

			return TRUE;
		}
		else if (showError)
		{
			DWORD errorCode = GetLastError();
			if (errorCode == ERROR_INVALID_VERSION_ARB)
				Main::ShowError(IDS_ERROR_ARB_VERSION, "GLib.cpp", __LINE__);
			else if (errorCode == ERROR_INVALID_PROFILE_ARB)
				Main::ShowError(IDS_ERROR_ARB_PROFILE, "GLib.cpp", __LINE__);
		}

		return FALSE;
	}

	VOID __fastcall CreateContextAttribs(HDC hDc, HGLRC* hRc)
	{
		CHAR buffer[256];
		LoadFunction(buffer, PREFIX_WGL, "CreateContextAttribs", (PROC*)&WGLCreateContextAttribs, "ARB");

		if (WGLCreateContextAttribs)
		{
			if (!GetContext(hDc, hRc, 3, 0, FALSE) && !GetContext(hDc, hRc, 2, 0, FALSE))
				GetContext(hDc, hRc, 1, 4, TRUE);
		}

		LoadFunction(buffer, PREFIX_WGL, "GetExtensionsString", (PROC*)&WGLGetExtensionsString, "EXT");
		if (WGLGetExtensionsString)
		{
			CHAR* extensions = (CHAR*)WGLGetExtensionsString();
			if (StrStr(extensions, "WGL_EXT_swap_control"))
				LoadFunction(buffer, PREFIX_WGL, "SwapInterval", (PROC*)&WGLSwapInterval, "EXT");
		}

		LoadFunction(buffer, PREFIX_GL, "GetString", (PROC*)&GLGetString);
		LoadFunction(buffer, PREFIX_GL, "TexCoord2f", (PROC*)&GLTexCoord2f);
		LoadFunction(buffer, PREFIX_GL, "Vertex2s", (PROC*)&GLVertex2s);
		LoadFunction(buffer, PREFIX_GL, "Begin", (PROC*)&GLBegin);
		LoadFunction(buffer, PREFIX_GL, "End", (PROC*)&GLEnd);
		LoadFunction(buffer, PREFIX_GL, "Viewport", (PROC*)&GLViewport);
		LoadFunction(buffer, PREFIX_GL, "MatrixMode", (PROC*)&GLMatrixMode);
		LoadFunction(buffer, PREFIX_GL, "LoadIdentity", (PROC*)&GLLoadIdentity);
		LoadFunction(buffer, PREFIX_GL, "Ortho", (PROC*)&GLOrtho);
		LoadFunction(buffer, PREFIX_GL, "Finish", (PROC*)&GLFinish);
		LoadFunction(buffer, PREFIX_GL, "Enable", (PROC*)&GLEnable);
		LoadFunction(buffer, PREFIX_GL, "Disable", (PROC*)&GLDisable);
		LoadFunction(buffer, PREFIX_GL, "BindTexture", (PROC*)&GLBindTexture);
		LoadFunction(buffer, PREFIX_GL, "DeleteTextures", (PROC*)&GLDeleteTextures);
		LoadFunction(buffer, PREFIX_GL, "TexParameteri", (PROC*)&GLTexParameteri);
		LoadFunction(buffer, PREFIX_GL, "TexEnvi", (PROC*)&GLTexEnvi);
		LoadFunction(buffer, PREFIX_GL, "GetTexImage", (PROC*)&GLGetTexImage);
		LoadFunction(buffer, PREFIX_GL, "TexImage2D", (PROC*)&GLTexImage2D);
		LoadFunction(buffer, PREFIX_GL, "TexSubImage2D", (PROC*)&GLTexSubImage2D);
		LoadFunction(buffer, PREFIX_GL, "GenTextures", (PROC*)&GLGenTextures);
		LoadFunction(buffer, PREFIX_GL, "GetIntegerv", (PROC*)&GLGetIntegerv);
		LoadFunction(buffer, PREFIX_GL, "Clear", (PROC*)&GLClear);
		LoadFunction(buffer, PREFIX_GL, "ClearColor", (PROC*)&GLClearColor);
		LoadFunction(buffer, PREFIX_GL, "BlendFunc", (PROC*)&GLBlendFunc);
		LoadFunction(buffer, PREFIX_GL, "ReadPixels", (PROC*)&GLReadPixels);
		LoadFunction(buffer, PREFIX_GL, "PixelStorei", (PROC*)&GLPixelStorei);

#ifdef _DEBUG
		LoadFunction(buffer, PREFIX_GL, "GetError", (PROC*)&GLGetError);
#endif

		LoadFunction(buffer, PREFIX_GL, "ActiveTexture", (PROC*)&GLActiveTexture);
		LoadFunction(buffer, PREFIX_GL, "GenBuffers", (PROC*)&GLGenBuffers);
		LoadFunction(buffer, PREFIX_GL, "DeleteBuffers", (PROC*)&GLDeleteBuffers);
		LoadFunction(buffer, PREFIX_GL, "BindBuffer", (PROC*)&GLBindBuffer);
		LoadFunction(buffer, PREFIX_GL, "BufferData", (PROC*)&GLBufferData);
		LoadFunction(buffer, PREFIX_GL, "BufferSubData", (PROC*)&GLBufferSubData);
		LoadFunction(buffer, PREFIX_GL, "DrawArrays", (PROC*)&GLDrawArrays);

		LoadFunction(buffer, PREFIX_GL, "EnableVertexAttribArray", (PROC*)&GLEnableVertexAttribArray);
		LoadFunction(buffer, PREFIX_GL, "VertexAttribPointer", (PROC*)&GLVertexAttribPointer);

		LoadFunction(buffer, PREFIX_GL, "CreateShader", (PROC*)&GLCreateShader);
		LoadFunction(buffer, PREFIX_GL, "DeleteShader", (PROC*)&GLDeleteShader);
		LoadFunction(buffer, PREFIX_GL, "CreateProgram", (PROC*)&GLCreateProgram);
		LoadFunction(buffer, PREFIX_GL, "DeleteProgram", (PROC*)&GLDeleteProgram);
		LoadFunction(buffer, PREFIX_GL, "ShaderSource", (PROC*)&GLShaderSource);
		LoadFunction(buffer, PREFIX_GL, "CompileShader", (PROC*)&GLCompileShader);
		LoadFunction(buffer, PREFIX_GL, "AttachShader", (PROC*)&GLAttachShader);
		LoadFunction(buffer, PREFIX_GL, "DetachShader", (PROC*)&GLDetachShader);
		LoadFunction(buffer, PREFIX_GL, "LinkProgram", (PROC*)&GLLinkProgram);
		LoadFunction(buffer, PREFIX_GL, "UseProgram", (PROC*)&GLUseProgram);
		LoadFunction(buffer, PREFIX_GL, "GetShaderiv", (PROC*)&GLGetShaderiv);
		LoadFunction(buffer, PREFIX_GL, "GetShaderInfoLog", (PROC*)&GLGetShaderInfoLog);

		LoadFunction(buffer, PREFIX_GL, "BindAttribLocation", (PROC*)&GLBindAttribLocation);
		LoadFunction(buffer, PREFIX_GL, "GetUniformLocation", (PROC*)&GLGetUniformLocation);

		LoadFunction(buffer, PREFIX_GL, "Uniform1i", (PROC*)&GLUniform1i);
		LoadFunction(buffer, PREFIX_GL, "Uniform1f", (PROC*)&GLUniform1f);
		LoadFunction(buffer, PREFIX_GL, "Uniform2f", (PROC*)&GLUniform2f);
		LoadFunction(buffer, PREFIX_GL, "Uniform4f", (PROC*)&GLUniform4f);

		LoadFunction(buffer, PREFIX_GL, "GenVertexArrays", (PROC*)&GLGenVertexArrays);
		LoadFunction(buffer, PREFIX_GL, "BindVertexArray", (PROC*)&GLBindVertexArray);
		LoadFunction(buffer, PREFIX_GL, "DeleteVertexArrays", (PROC*)&GLDeleteVertexArrays);

		LoadFunction(buffer, PREFIX_GL, "GenFramebuffers", (PROC*)&GLGenFramebuffers);
		LoadFunction(buffer, PREFIX_GL, "DeleteFramebuffers", (PROC*)&GLDeleteFramebuffers);
		LoadFunction(buffer, PREFIX_GL, "BindFramebuffer", (PROC*)&GLBindFramebuffer);
		LoadFunction(buffer, PREFIX_GL, "FramebufferTexture2D", (PROC*)&GLFramebufferTexture2D);

		LoadFunction(buffer, PREFIX_GL, "GenRenderbuffers", (PROC*)&GLGenRenderbuffers);
		LoadFunction(buffer, PREFIX_GL, "DeleteRenderbuffers", (PROC*)&GLDeleteRenderbuffers);
		LoadFunction(buffer, PREFIX_GL, "BindRenderbuffer", (PROC*)&GLBindRenderbuffer);
		LoadFunction(buffer, PREFIX_GL, "RenderbufferStorage", (PROC*)&GLRenderbufferStorage);
		LoadFunction(buffer, PREFIX_GL, "FramebufferRenderbuffer", (PROC*)&GLFramebufferRenderbuffer);

		if (!config.gl.version.real)
		{
			if (GLGetString)
			{
				CHAR* strVer = (CHAR*)GLGetString(GL_VERSION);
				if (strVer && *strVer >= '0' && *strVer <= '9')
				{
					BYTE* ver = (BYTE*)&config.gl.version.real;

					BOOL appears = FALSE;
					CHAR* p = strVer;
					for (DWORD charIdx = 0, byteIdx = 0; byteIdx < 4; ++p)
					{
						if (*p >= '0' && *p <= '9')
						{
							appears = FALSE;

							*ver = *ver * 10 + (*p - '0');
						}
						else
						{
							if (*p != '.' || appears)
							{
								if (config.gl.version.real)
								{
									BYTE* ver = (BYTE*)&config.gl.version.real + 3;
									while (!*ver)
										config.gl.version.real <<= 8;
								}

								break;
							}

							appears = TRUE;
							config.gl.version.real <<= 8;
							++byteIdx;
							charIdx = 0;
						}
					}
				}
				else
					config.gl.version.real = GL_VER_1_1;

				if (config.gl.version.real < GL_VER_1_2)
				{
					CHAR* extensions = (CHAR*)GLGetString(GL_EXTENSIONS);
					if (extensions)
						config.gl.caps.clampToEdge = (StrStr(extensions, "GL_EXT_texture_edge_clamp") || StrStr(extensions, "GL_SGIS_texture_edge_clamp")) ? GL_CLAMP_TO_EDGE : GL_CLAMP;
				}
				else
					config.gl.caps.clampToEdge = GL_CLAMP_TO_EDGE;
			}

			if (!config.gl.version.real)
				config.gl.version.real = GL_VER_1_1;
			else if (config.gl.version.real >= GL_VER_2_0)
			{
				DWORD check = max(config.mode->width, config.mode->height);
				DWORD size = 1;
				while (size < check)
					size <<= 1;

				DWORD maxSize;
				GLGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&maxSize);
				if (maxSize < size)
					config.gl.version.real = GL_VER_1_1;
			}
		}
	}

	VOID __fastcall ResetPixelFormatDescription(PIXELFORMATDESCRIPTOR* pfd)
	{
		MemoryZero(pfd, sizeof(PIXELFORMATDESCRIPTOR));
		pfd->nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd->nVersion = 1;
	}

	VOID __fastcall PreparePixelFormatDescription(PIXELFORMATDESCRIPTOR* pfd)
	{
		ResetPixelFormatDescription(pfd);

		INT bpp = 0;
		HDC hDc = GetDC(NULL);
		if (hDc)
		{
			bpp = GetDeviceCaps(hDc, BITSPIXEL);
			ReleaseDC(NULL, hDc);
		}

		pfd->dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DEPTH_DONTCARE | PFD_SWAP_EXCHANGE;
		pfd->cColorBits = (bpp == 16 || bpp == 24) ? (BYTE)bpp : 32;
	}

	INT __fastcall PreparePixelFormat(PIXELFORMATDESCRIPTOR* pfd)
	{
		PreparePixelFormatDescription(pfd);

		INT res = 0;

		HWND hWnd = CreateWindowEx(
			WS_EX_APPWINDOW,
			WC_DRAW,
			NULL,
			WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			0, 0,
			1, 1,
			NULL,
			NULL,
			hDllModule,
			NULL);

		if (hWnd)
		{
			HDC hDc = GetDC(hWnd);
			if (hDc)
			{
				res = ::ChoosePixelFormat(hDc, pfd);
				if (res && ::SetPixelFormat(hDc, res, pfd))
				{
					HGLRC hRc = wglCreateContext(hDc);
					if (hRc)
					{
						if (wglMakeCurrent(hDc, hRc))
						{
							CHAR buffer[32];
							LoadFunction(buffer, PREFIX_WGL, "ChoosePixelFormat", (PROC*)&WGLChoosePixelFormat, "ARB");
							if (WGLChoosePixelFormat)
							{
								INT glAttributes[] = {
									WGL_DRAW_TO_WINDOW_ARB, (pfd->dwFlags & PFD_DRAW_TO_WINDOW) ? GL_TRUE : GL_FALSE,
									WGL_SUPPORT_OPENGL_ARB, (pfd->dwFlags & PFD_SUPPORT_OPENGL) ? GL_TRUE : GL_FALSE,
									WGL_DOUBLE_BUFFER_ARB, (pfd->dwFlags & PFD_DOUBLEBUFFER) ? GL_TRUE : GL_FALSE,
									WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
									WGL_COLOR_BITS_ARB, pfd->cColorBits,
									WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
									WGL_SWAP_METHOD_ARB, (pfd->dwFlags & PFD_SWAP_EXCHANGE) ? WGL_SWAP_EXCHANGE_ARB : WGL_SWAP_COPY_ARB,
									0
								};

								INT piFormat;
								UINT nNumFormats;
								if (WGLChoosePixelFormat(hDc, glAttributes, NULL, 1, &piFormat, &nNumFormats) && nNumFormats)
									res = piFormat;
							}

							wglMakeCurrent(hDc, NULL);
						}

						wglDeleteContext(hRc);
					}
				}

				ReleaseDC(hWnd, hDc);
			}

			DestroyWindow(hWnd);
		}

		return res;
	}

	VOID __fastcall ResetPixelFormat()
	{
		HWND hWnd = CreateWindowEx(
			WS_EX_APPWINDOW,
			WC_DRAW,
			NULL,
			WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			0, 0,
			1, 1,
			NULL,
			NULL,
			hDllModule,
			NULL);

		if (hWnd)
		{
			HDC hDc = GetDC(hWnd);
			if (hDc)
			{
				PIXELFORMATDESCRIPTOR pfd;
				PreparePixelFormatDescription(&pfd);

				INT res = ::ChoosePixelFormat(hDc, &pfd);
				if (res)
					::SetPixelFormat(hDc, res, &pfd);

				ReleaseDC(hWnd, hDc);
			}

			DestroyWindow(hWnd);
		}
	}

	GLuint __fastcall CompileShaderSource(DWORD name, const CHAR* version, GLenum type)
	{
		HGLOBAL hResourceData;
		LPVOID pData = NULL;
		HRSRC hResource = FindResource(hDllModule, MAKEINTRESOURCE(name), RT_RCDATA);
		if (hResource)
		{
			hResourceData = LoadResource(hDllModule, hResource);
			if (hResourceData)
				pData = LockResource(hResourceData);
		}

		if (!pData)
			Main::ShowError(IDS_ERROR_LOAD_RESOURCE, "GLib.cpp", __LINE__);

		GLuint shader = GLCreateShader(type);

		DWORD pre = StrLength(version);
		DWORD length = SizeofResource(hDllModule, hResource);
		DWORD size = length + pre;
		CHAR* source = (CHAR*)MemoryAlloc(size + 1);
		const GLchar* srcData[] = { source };
		{
			MemoryCopy(source, version, pre);
			MemoryCopy(source + pre, pData, length);
			*(source + size) = NULL;

			GLShaderSource(shader, 1, srcData, NULL);
		}
		MemoryFree(source);

		GLint result;
		GLCompileShader(shader);

		GLGetShaderiv(shader, GL_COMPILE_STATUS, &result);
		if (!result)
		{
			GLGetShaderiv(shader, GL_INFO_LOG_LENGTH, &result);

			if (!result)
				Main::ShowError(IDS_ERROR_COMPILE_SHADER, "GLib.cpp", __LINE__);
			else
			{
				CHAR data[1024];
				GLGetShaderInfoLog(shader, sizeof(data), &result, data);
				Main::ShowError(data, "GLib.cpp", __LINE__);
			}
		}

		return shader;
	}

	HGLRC __fastcall Init(HWND hWnd, HDC* lpHDC)
	{
		HGLRC hRc = NULL;
		*lpHDC = ::GetDC(hWnd);
		if (*lpHDC)
		{
			if (!::GetPixelFormat(*lpHDC))
			{
				PIXELFORMATDESCRIPTOR pfd;
				INT glPixelFormat = GL::PreparePixelFormat(&pfd);
				if (!glPixelFormat)
				{
					glPixelFormat = ::ChoosePixelFormat(*lpHDC, &pfd);
					if (!glPixelFormat)
						Main::ShowError(IDS_ERROR_CHOOSE_PF, "OpenDraw.cpp", __LINE__);
					else if (pfd.dwFlags & PFD_NEED_PALETTE)
						Main::ShowError(IDS_ERROR_NEED_PALETTE, "OpenDraw.cpp", __LINE__);
				}

				GL::ResetPixelFormatDescription(&pfd);
				if (::DescribePixelFormat(*lpHDC, glPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd) == NULL)
					Main::ShowError(IDS_ERROR_DESCRIBE_PF, "OpenDraw.cpp", __LINE__);

				if (!::SetPixelFormat(*lpHDC, glPixelFormat, &pfd))
					Main::ShowError(IDS_ERROR_SET_PF, "OpenDraw.cpp", __LINE__);

				if (pfd.iPixelType != PFD_TYPE_RGBA || pfd.cRedBits < 5 || pfd.cGreenBits < 6 || pfd.cBlueBits < 5)
					Main::ShowError(IDS_ERROR_BAD_PF, "OpenDraw.cpp", __LINE__);
			}

			hRc = wglCreateContext(*lpHDC);
			if (hRc && wglMakeCurrent(*lpHDC, hRc))
				GL::CreateContextAttribs(*lpHDC, &hRc);
		}

		return hRc;
	}

	VOID __fastcall Release(HWND hWnd, HDC* lpHDC, HGLRC* lpHRC)
	{
		if (*lpHDC)
		{
			if (*lpHRC)
			{
				wglMakeCurrent(*lpHDC, NULL);
				wglDeleteContext(*lpHRC);
				*lpHRC = NULL;
			}

			::ReleaseDC(hWnd, *lpHDC);
			*lpHDC = NULL;
		}
	}
#pragma optimize("", on)
}