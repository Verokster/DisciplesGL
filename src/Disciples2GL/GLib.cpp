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
#include "GLib.h"

#define PREFIX_GL "gl"
#define PREFIX_WGL "wgl"

WGLGETPROCADDRESS WGLGetProcAddress;
WGLMAKECURRENT WGLMakeCurrent;
WGLCREATECONTEXT WGLCreateContext;
WGLDELETECONTEXT WGLDeleteContext;
WGLCREATECONTEXTATTRIBSARB WGLCreateContextAttribs;
WGLSWAPINTERVAL WGLSwapInterval;

GLGETSTRING GLGetString;
GLVERTEX2S GLVertex2s;
GLCOLOR4UBV GLColor4ubv;
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
GLTEXSUBIMAGE1D GLTexSubImage1D;
GLTEXIMAGE2D GLTexImage2D;
GLTEXSUBIMAGE2D GLTexSubImage2D;
GLGENTEXTURES GLGenTextures;
GLGETINTEGERV GLGetIntegerv;
GLCLEAR GLClear;
GLCLEARCOLOR GLClearColor;
GLCOLORMASK GLColorMask;
GLSTENCILFUNC GLStencilFunc;
GLSTENCILOP GLStencilOp;

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

GLGETATTRIBLOCATION GLGetAttribLocation;
GLGETUNIFORMLOCATION GLGetUniformLocation;

GLUNIFORM1I GLUniform1i;
GLUNIFORM2F GLUniform2f;
GLUNIFORMMATRIX4FV GLUniformMatrix4fv;

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

HMODULE hModule;

DWORD glVersion;
DWORD glCapsClampToEdge;
BOOL glCapsBGRA;

namespace GL
{
	BOOL __fastcall Load()
	{
		if (!hModule)
			hModule = LoadLibrary("OPENGL32.dll");

		if (!hModule)
			return FALSE;

		WGLGetProcAddress = (WGLGETPROCADDRESS)GetProcAddress(hModule, "wglGetProcAddress");
		WGLMakeCurrent = (WGLMAKECURRENT)GetProcAddress(hModule, "wglMakeCurrent");
		WGLCreateContext = (WGLCREATECONTEXT)GetProcAddress(hModule, "wglCreateContext");
		WGLDeleteContext = (WGLDELETECONTEXT)GetProcAddress(hModule, "wglDeleteContext");

		return TRUE;
	}

	VOID __fastcall Free()
	{
		if (FreeLibrary(hModule))
			hModule = NULL;
	}

	VOID __fastcall LoadFunction(CHAR* buffer, const CHAR* prefix, const CHAR* name, PROC* func, const CHAR* sufix = NULL)
	{
		StrCopy(buffer, prefix);
		StrCat(buffer, name);

		if (sufix)
			StrCat(buffer, sufix);

		if (WGLGetProcAddress)
			*func = WGLGetProcAddress(buffer);

		if ((INT)*func >= -1 && (INT)*func <= 3)
			*func = GetProcAddress(hModule, buffer);

		if (!sufix && !*func)
		{
			LoadFunction(buffer, prefix, name, func, "EXT");
			if (!*func)
				LoadFunction(buffer, prefix, name, func, "ARB");
		}
	}

	BOOL __fastcall GetContext(HDC hDc, HGLRC* lpHRc, BOOL showError, DWORD* wglAttributes)
	{
		HGLRC hRc = WGLCreateContextAttribs(hDc, NULL, wglAttributes);
		if (hRc)
		{
			WGLMakeCurrent(hDc, hRc);
			WGLDeleteContext(*lpHRc);
			*lpHRc = hRc;

			return TRUE;
		}
		else if (showError)
		{
			DWORD errorCode = GetLastError();
			if (errorCode == ERROR_INVALID_VERSION_ARB)
				Main::ShowError("Invalid ARB version", __FILE__, __LINE__);
			else if (errorCode == ERROR_INVALID_PROFILE_ARB)
				Main::ShowError("Invalid ARB profile", __FILE__, __LINE__);
		}

		return FALSE;
	}

	BOOL __fastcall GetContext(HDC hDc, HGLRC* lpHRc, DWORD major, DWORD minor, BOOL showError)
	{
		DWORD wglAttributes[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, major,
			WGL_CONTEXT_MINOR_VERSION_ARB, minor,
			0
		};

		return GetContext(hDc, lpHRc, showError, wglAttributes);
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

		LoadFunction(buffer, "wgl", "SwapInterval", (PROC*)&WGLSwapInterval, "EXT");

		LoadFunction(buffer, PREFIX_GL, "GetString", (PROC*)&GLGetString);
		LoadFunction(buffer, PREFIX_GL, "TexCoord2f", (PROC*)&GLTexCoord2f);
		LoadFunction(buffer, PREFIX_GL, "Vertex2s", (PROC*)&GLVertex2s);
		LoadFunction(buffer, PREFIX_GL, "Color4ubv", (PROC*)&GLColor4ubv);
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
		LoadFunction(buffer, PREFIX_GL, "TexSubImage1D", (PROC*)&GLTexSubImage1D);
		LoadFunction(buffer, PREFIX_GL, "TexImage2D", (PROC*)&GLTexImage2D);
		LoadFunction(buffer, PREFIX_GL, "TexSubImage2D", (PROC*)&GLTexSubImage2D);
		LoadFunction(buffer, PREFIX_GL, "GenTextures", (PROC*)&GLGenTextures);
		LoadFunction(buffer, PREFIX_GL, "GetIntegerv", (PROC*)&GLGetIntegerv);
		LoadFunction(buffer, PREFIX_GL, "Clear", (PROC*)&GLClear);
		LoadFunction(buffer, PREFIX_GL, "ClearColor", (PROC*)&GLClearColor);
		LoadFunction(buffer, PREFIX_GL, "ColorMask", (PROC*)&GLColorMask);
		LoadFunction(buffer, PREFIX_GL, "StencilFunc", (PROC*)&GLStencilFunc);
		LoadFunction(buffer, PREFIX_GL, "StencilOp", (PROC*)&GLStencilOp);

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

		LoadFunction(buffer, PREFIX_GL, "GetAttribLocation", (PROC*)&GLGetAttribLocation);
		LoadFunction(buffer, PREFIX_GL, "GetUniformLocation", (PROC*)&GLGetUniformLocation);

		LoadFunction(buffer, PREFIX_GL, "Uniform1i", (PROC*)&GLUniform1i);
		LoadFunction(buffer, PREFIX_GL, "Uniform2f", (PROC*)&GLUniform2f);
		LoadFunction(buffer, PREFIX_GL, "UniformMatrix4fv", (PROC*)&GLUniformMatrix4fv);

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

		glVersion = NULL;
		if (GLGetString)
		{
			CHAR* strVer = (CHAR*)GLGetString(GL_VERSION);
			if (strVer && *strVer >= '0' && *strVer <= '9')
			{
				BYTE* ver = (BYTE*)&glVersion;

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
							if (glVersion)
							{
								BYTE* ver = (BYTE*)&glVersion + 3;
								while (!*ver)
									glVersion <<= 8;
							}

							break;
						}

						appears = TRUE;
						glVersion <<= 8;
						++byteIdx;
						charIdx = 0;
					}
				}
			}
			else
				glVersion = GL_VER_1_1;

			if (glVersion < GL_VER_1_2)
			{
				CHAR* extensions = (CHAR*)GLGetString(GL_EXTENSIONS);
				if (extensions)
				{
					glCapsClampToEdge = (StrStr(extensions, "GL_EXT_texture_edge_clamp") || StrStr(extensions, "GL_SGIS_texture_edge_clamp")) ? GL_CLAMP_TO_EDGE : GL_CLAMP;
					glCapsBGRA = StrStr(extensions, "GL_EXT_bgra") || StrStr(extensions, "GL_EXT_texture_format_BGRA8888") || StrStr(extensions, "GL_APPLE_texture_format_BGRA8888");
				}
			}
			else
			{
				glCapsClampToEdge = GL_CLAMP_TO_EDGE;
				glCapsBGRA = TRUE;
			}
		}

		if (!glVersion)
			glVersion = GL_VER_1_1;
	}

	VOID __fastcall PreparePixelFormatDescription(PIXELFORMATDESCRIPTOR* pfd)
	{
		DEVMODE devMode;
		MemoryZero(&devMode, sizeof(DEVMODE));
		devMode.dmSize = sizeof(DEVMODE);

		DWORD bpp;
		if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode) && devMode.dmBitsPerPel >= 16 && devMode.dmBitsPerPel <= 32)
			bpp = devMode.dmBitsPerPel;
		else
			bpp = 32;

		MemoryZero(pfd, sizeof(PIXELFORMATDESCRIPTOR));
		pfd->nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd->nVersion = 1;
		pfd->dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DEPTH_DONTCARE | PFD_STEREO_DONTCARE | PFD_SWAP_EXCHANGE;
		pfd->iPixelType = PFD_TYPE_RGBA;
		pfd->cColorBits = LOBYTE(bpp);
		pfd->cStencilBits = 8;
		pfd->iLayerType = PFD_MAIN_PLANE;
	}

	INT __fastcall PreparePixelFormat(PIXELFORMATDESCRIPTOR* pfd)
	{
		INT res = 0;

		HWND hWnd = CreateWindowEx(
			WS_EX_APPWINDOW,
			WC_DRAW,
			"DUMMY",
			WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			0, 0,
			1, 1,
			NULL,
			NULL,
			hDllModule,
			NULL
		);

		if (hWnd)
		{
			HDC hDc = GetDC(hWnd);
			if (hDc)
			{
				res = ChoosePixelFormat(hDc, pfd);
				if (res)
				{
					if (SetPixelFormat(hDc, res, pfd))
					{
						HGLRC hRc = WGLCreateContext(hDc);
						if (hRc)
						{
							if (WGLMakeCurrent(hDc, hRc))
							{
								WGLCHOOSEPIXELFORMATARB WGLChoosePixelFormatARB = (WGLCHOOSEPIXELFORMATARB)WGLGetProcAddress("wglChoosePixelFormatARB");
								if (WGLChoosePixelFormatARB)
								{
									INT piFormats[128];
									UINT nNumFormats;

									INT glAttributes[] = {
										WGL_DRAW_TO_WINDOW_ARB, (pfd->dwFlags & PFD_DRAW_TO_WINDOW) ? GL_TRUE : GL_FALSE,
										WGL_SUPPORT_OPENGL_ARB, (pfd->dwFlags & PFD_SUPPORT_OPENGL) ? GL_TRUE : GL_FALSE,
										WGL_DOUBLE_BUFFER_ARB, (pfd->dwFlags & PFD_DOUBLEBUFFER) ? GL_TRUE : GL_FALSE,
										WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
										WGL_COLOR_BITS_ARB, pfd->cColorBits,
										WGL_STENCIL_BITS_ARB, pfd->cStencilBits,
										WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
										WGL_SWAP_METHOD_ARB, (pfd->dwFlags & PFD_SWAP_EXCHANGE) ? WGL_SWAP_EXCHANGE_ARB : WGL_SWAP_COPY_ARB,
										0
									};

									if (WGLChoosePixelFormatARB(hDc, glAttributes, NULL, sizeof(piFormats) / sizeof(INT), piFormats, &nNumFormats) && nNumFormats)
										res = piFormats[0];
								}

								WGLMakeCurrent(hDc, NULL);
							}

							WGLDeleteContext(hRc);
						}
					}
				}

				ReleaseDC(hWnd, hDc);
			}

			DestroyWindow(hWnd);
		}

		return res;
	}

	GLuint __fastcall CompileShaderSource(DWORD name, const CHAR* version, GLenum type)
	{
		HRSRC hResource = FindResource(hDllModule, MAKEINTRESOURCE(name), RT_RCDATA);
		if (!hResource)
			Main::ShowError("FindResource failed", __FILE__, __LINE__);

		HGLOBAL hResourceData = LoadResource(hDllModule, hResource);
		if (!hResourceData)
			Main::ShowError("LoadResource failed", __FILE__, __LINE__);

		LPVOID pData = LockResource(hResourceData);
		if (!pData)
			Main::ShowError("LockResource failed", __FILE__, __LINE__);

		GLuint shader = GLCreateShader(type);

		const GLchar* source[] = { "#version ", version, "\n", (GLchar*)pData };
		const GLint lengths[] = { 9, 3, 1, (GLint)SizeofResource(hDllModule, hResource) };
		GLShaderSource(shader, 4, source, lengths);

		GLint result;
		GLCompileShader(shader);

		GLGetShaderiv(shader, GL_COMPILE_STATUS, &result);
		if (!result)
		{
			GLGetShaderiv(shader, GL_INFO_LOG_LENGTH, &result);

			if (!result)
				Main::ShowError("Compile shader failed", __FILE__, __LINE__);
			else
			{
				CHAR data[512];
				GLGetShaderInfoLog(shader, sizeof(data), &result, data);
				Main::ShowError(data, __FILE__, __LINE__);
			}
		}

		return shader;
	}

	DWORD __stdcall ResetThread(LPVOID lpParameter)
	{
		PIXELFORMATDESCRIPTOR pfd;
		GL::PreparePixelFormatDescription(&pfd);
		GL::PreparePixelFormat(&pfd);

		return NULL;
	}

	VOID __fastcall ResetContext()
	{
		DWORD threadId;
		SECURITY_ATTRIBUTES sAttribs;
		MemoryZero(&sAttribs, sizeof(SECURITY_ATTRIBUTES));
		sAttribs.nLength = sizeof(SECURITY_ATTRIBUTES);
		HANDLE hDummy = CreateThread(&sAttribs, NULL, ResetThread, NULL, NORMAL_PRIORITY_CLASS, &threadId);
		WaitForSingleObject(hDummy, INFINITE);
		CloseHandle(hDummy);
	}
}