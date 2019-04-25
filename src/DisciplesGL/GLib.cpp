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
#include "Resource.h"
#include "GLib.h"

#define PREFIX_GL "gl"
#define PREFIX_WGL "wgl"

WGLGETPROCADDRESS WGLGetProcAddress;
WGLMAKECURRENT WGLMakeCurrent;
WGLCREATECONTEXT WGLCreateContext;
WGLDELETECONTEXT WGLDeleteContext;
WGLCREATECONTEXTATTRIBSARB WGLCreateContextAttribs;
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
GLCOLORMASK GLColorMask;

#ifdef _DEBUG
GLGETERROR GLGetError;
#endif

GLACTIVETEXTURE GLActiveTexture;
GLGENBUFFERS GLGenBuffers;
GLDELETEBUFFERS GLDeleteBuffers;
GLBINDBUFFER GLBindBuffer;
GLBUFFERDATA GLBufferData;
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

HMODULE hGLModule;

DWORD glVersion;
DWORD glCapsClampToEdge;

namespace GL
{
#pragma optimize("s", on)
	BOOL __fastcall Load()
	{
		if (!hGLModule)
			hGLModule = LoadLibrary("OPENGL32.dll");

		if (!hGLModule)
			return FALSE;

		WGLGetProcAddress = (WGLGETPROCADDRESS)GetProcAddress(hGLModule, "wglGetProcAddress");
		WGLMakeCurrent = (WGLMAKECURRENT)GetProcAddress(hGLModule, "wglMakeCurrent");
		WGLCreateContext = (WGLCREATECONTEXT)GetProcAddress(hGLModule, "wglCreateContext");
		WGLDeleteContext = (WGLDELETECONTEXT)GetProcAddress(hGLModule, "wglDeleteContext");

		return TRUE;
	}

	VOID __fastcall Free()
	{
		if (hGLModule && FreeLibrary(hGLModule))
			hGLModule = NULL;
	}

	VOID __fastcall LoadFunction(CHAR* buffer, const CHAR* prefix, const CHAR* name, PROC* func, const CHAR* sufix = NULL)
	{
		if (*func)
			return;

		StrCopy(buffer, prefix);
		StrCat(buffer, name);

		if (sufix)
			StrCat(buffer, sufix);

		if (WGLGetProcAddress)
			*func = WGLGetProcAddress(buffer);

		if ((INT)*func >= -1 && (INT)*func <= 3)
			*func = GetProcAddress(hGLModule, buffer);

		if (!sufix)
		{
			LoadFunction(buffer, prefix, name, func, "EXT");
			if (!*func)
				LoadFunction(buffer, prefix, name, func, "ARB");
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
			WGLMakeCurrent(hDc, hRc);
			WGLDeleteContext(*lpHRc);
			*lpHRc = hRc;

			return TRUE;
		}
		else if (showError)
		{
			DWORD errorCode = GetLastError();
			if (errorCode == ERROR_INVALID_VERSION_ARB)
				Main::ShowError(IDS_ERROR_ARB_VERSION, __FILE__, __LINE__);
			else if (errorCode == ERROR_INVALID_PROFILE_ARB)
				Main::ShowError(IDS_ERROR_ARB_PROFILE, __FILE__, __LINE__);
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
		LoadFunction(buffer, PREFIX_GL, "ColorMask", (PROC*)&GLColorMask);

#ifdef _DEBUG
		LoadFunction(buffer, PREFIX_GL, "GetError", (PROC*)&GLGetError);
#endif

		LoadFunction(buffer, PREFIX_GL, "ActiveTexture", (PROC*)&GLActiveTexture);
		LoadFunction(buffer, PREFIX_GL, "GenBuffers", (PROC*)&GLGenBuffers);
		LoadFunction(buffer, PREFIX_GL, "DeleteBuffers", (PROC*)&GLDeleteBuffers);
		LoadFunction(buffer, PREFIX_GL, "BindBuffer", (PROC*)&GLBindBuffer);
		LoadFunction(buffer, PREFIX_GL, "BufferData", (PROC*)&GLBufferData);
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
					glCapsClampToEdge = (StrStr(extensions, "GL_EXT_texture_edge_clamp") || StrStr(extensions, "GL_SGIS_texture_edge_clamp")) ? GL_CLAMP_TO_EDGE : GL_CLAMP;
			}
			else
				glCapsClampToEdge = GL_CLAMP_TO_EDGE;
		}

		if (!glVersion)
			glVersion = GL_VER_1_1;
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

		pfd->dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DEPTH_DONTCARE | PFD_STEREO_DONTCARE | PFD_SWAP_EXCHANGE;
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
			NULL
		);

		if (hWnd)
		{
			HDC hDc = GetDC(hWnd);
			if (hDc)
			{
				res = ::ChoosePixelFormat(hDc, pfd);
				if (res && ::SetPixelFormat(hDc, res, pfd))
				{
					HGLRC hRc = WGLCreateContext(hDc);
					if (hRc)
					{
						if (WGLMakeCurrent(hDc, hRc))
						{
							CHAR buffer[32];
							LoadFunction(buffer, PREFIX_WGL, "ChoosePixelFormat", (PROC*)&WGLChoosePixelFormat, "ARB");
							if (WGLChoosePixelFormat)
							{
								INT piFormats[128];
								UINT nNumFormats = 0;

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

								if (WGLChoosePixelFormat(hDc, glAttributes, NULL, sizeof(piFormats) / sizeof(INT), piFormats, &nNumFormats) && nNumFormats)
									res = piFormats[0];
							}

							WGLMakeCurrent(hDc, NULL);
						}

						WGLDeleteContext(hRc);
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
			Main::ShowError(IDS_ERROR_LOAD_RESOURCE, __FILE__, __LINE__);

		GLuint shader = GLCreateShader(type);

		DWORD length = SizeofResource(hDllModule, hResource);
		DWORD size = length + 13;
		CHAR* source = (CHAR*)MemoryAlloc(size + 1);
		const GLchar* srcData[] = { source };
		{
			MemoryCopy(source, version, 13);
			MemoryCopy(source + 13, pData, length);
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
				Main::ShowError(IDS_ERROR_COMPILE_SHADER, __FILE__, __LINE__);
			else
			{
				CHAR data[1024];
				GLGetShaderInfoLog(shader, sizeof(data), &result, data);
				Main::ShowError(data, __FILE__, __LINE__);
			}
		}

		return shader;
	}

	DWORD __stdcall ResetThread(LPVOID lpParameter)
	{
		PIXELFORMATDESCRIPTOR pfd;
		GL::PreparePixelFormat(&pfd);

		return NULL;
	}

	VOID __fastcall ResetContext()
	{
		HANDLE hThread = CreateThread(NULL, NULL, ResetThread, NULL, NORMAL_PRIORITY_CLASS, NULL);
		if (hThread)
		{
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
		}
	}
#pragma optimize("", on)
}