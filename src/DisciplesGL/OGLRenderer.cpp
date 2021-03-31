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
#include "OGLRenderer.h"
#include "Resource.h"
#include "Config.h"
#include "Main.h"
#include "GLib.h"

DWORD OGLRenderer::GetPow2(DWORD value)
{
	DWORD res = 1;
	while (res < value)
		res <<= 1;
	return res;
}

VOID OGLRenderer::GLBindTexFilter(GLuint textureId, GLint filter)
{
	GLBindTexture(GL_TEXTURE_2D, textureId);
	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
}

VOID OGLRenderer::GLBindTexParameter(GLuint textureId, GLint filter)
{
	GLBindTexture(GL_TEXTURE_2D, textureId);
	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
}

OGLRenderer::OGLRenderer(OpenDraw* ddraw)
	: Renderer(ddraw)
{
	this->singleThread = config.singleThread;
}

BOOL OGLRenderer::Start()
{
	if (!Renderer::Start())
		return FALSE;

	if (this->singleThread)
	{
		this->hRc = GL::Init(this->ddraw->hDraw, &this->hDc);
		this->Begin();
	}
	else
	{
		this->hDrawEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		DWORD threadId;
		SECURITY_ATTRIBUTES sAttribs = { sizeof(SECURITY_ATTRIBUTES), NULL, FALSE };
		this->hDrawThread = CreateThread(&sAttribs, NULL, OGLRenderer::RenderThread, this, HIGH_PRIORITY_CLASS, &threadId);
	}
	
	return TRUE;
}

BOOL OGLRenderer::Stop()
{
	if (!Renderer::Stop())
		return FALSE;

	if (this->singleThread)
	{
		this->End();
		GL::Release(this->ddraw->hDraw, &this->hDc, &this->hRc);
	}
	else
	{
		SetEvent(this->hDrawEvent);
		WaitForSingleObject(this->hDrawThread, INFINITE);
		CloseHandle(this->hDrawThread);
		CloseHandle(this->hDrawEvent);
	}

	return TRUE;
}

DWORD OGLRenderer::RenderThread(LPVOID lpParameter)
{
	OGLRenderer* renderer = (OGLRenderer*)lpParameter;
	renderer->hRc = GL::Init(renderer->ddraw->hDraw, &renderer->hDc);
	if (renderer->hRc)
		renderer->Cycle();

	GL::Release(renderer->ddraw->hDraw, &renderer->hDc, &renderer->hRc);
	return NULL;
}

VOID OGLRenderer::Clear()
{
	GLClear(GL_COLOR_BUFFER_BIT);
}

VOID OGLRenderer::RenderFrame(BOOL ready, BOOL force, StateBufferAligned** lpStateBuffer)
{
	StateBufferAligned* stateBuffer = *lpStateBuffer;
	Size* frameSize = &stateBuffer->size;

	if (force)
		this->pixelBuffer->Reset();

	if (this->RenderInner(ready, force, lpStateBuffer))
	{
		SwapBuffers(this->hDc);
		GLFinish();
	}

	if (this->ddraw->isTakeSnapshot)
		this->ddraw->TakeSnapshot(frameSize, stateBuffer->data, this->isTrueColor);
}

VOID OGLRenderer::Cycle()
{
	this->Begin();

	while (!this->isFinish)
	{
		this->Render();
		WaitForSingleObject(this->hDrawEvent, INFINITE);
	}

	this->End();
}

VOID OGLRenderer::Redraw()
{
	if (this->singleThread)
		this->Render();
	else
		SetEvent(this->hDrawEvent);
}

BOOL OGLRenderer::ReadFrame(BYTE* dstData, Rect* rect, DWORD pitch, BOOL isBGR, BOOL* rev)
{
	GLenum format;

	if (isBGR)
	{
		if (config.gl.version.real > GL_VER_1_1)
			format = GL_BGR_EXT;
		else
		{
			format = GL_RGB;
			*rev = TRUE;
		}
	}
	else
		format = GL_RGB;

	GLReadPixels(rect->x, rect->y, rect->width, rect->height, format, GL_UNSIGNED_BYTE, dstData);
	
	return TRUE;
}
