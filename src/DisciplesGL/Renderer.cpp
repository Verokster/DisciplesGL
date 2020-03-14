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
#include "Renderer.h"
#include "OpenDraw.h"
#include "Config.h"
#include "Window.h"

Renderer::Renderer(OpenDraw* ddraw)
{
	this->ddraw = ddraw;
	ddraw->renderer = this;

	this->isFinish = TRUE;
	this->program = NULL;
}

Renderer::~Renderer()
{
	if (this->ddraw->hDraw != this->ddraw->hWnd)
	{
		DestroyWindow(this->ddraw->hDraw);
		GL::ResetPixelFormat();
	}

	this->ddraw->hDraw = NULL;
	this->ddraw->renderer = NULL;
}

BOOL Renderer::Start()
{
	if (!this->isFinish || !this->ddraw->hWnd)
		return FALSE;

	this->isFinish = FALSE;

	RECT rect;
	GetClientRect(this->ddraw->hWnd, &rect);

	if (config.singleWindow)
		this->ddraw->hDraw = this->ddraw->hWnd;
	else
	{
		if (!config.windowedMode && !config.borderless.mode)
			this->ddraw->hDraw = CreateWindowEx(
				WS_EX_CONTROLPARENT | WS_EX_TOPMOST,
				WC_DRAW,
				NULL,
				WS_VISIBLE | WS_POPUP | WS_MAXIMIZE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
				0, 0,
				rect.right, rect.bottom,
				this->ddraw->hWnd,
				NULL,
				hDllModule,
				NULL);
		else
		{
			this->ddraw->hDraw = CreateWindowEx(
				WS_EX_CONTROLPARENT,
				WC_DRAW,
				NULL,
				WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
				0, 0,
				rect.right, rect.bottom,
				this->ddraw->hWnd,
				NULL,
				hDllModule,
				NULL);
		}

		Window::SetCapturePanel(this->ddraw->hDraw);
	}

	this->ddraw->LoadFilterState();
	this->ddraw->viewport.width = rect.right;
	this->ddraw->viewport.height = rect.bottom - this->ddraw->viewport.offset;
	this->ddraw->viewport.refresh = TRUE;

	return TRUE;
}

VOID Renderer::Render()
{
	OpenDrawSurface* surface = this->ddraw->attachedSurface;
	if (!surface)
		return;

	StateBufferAligned** lpStateBuffer = (StateBufferAligned**)&(!this->ddraw->bufferIndex ? surface->indexBuffer : surface->secondaryBuffer);
	StateBufferAligned* stateBuffer = *lpStateBuffer;
	if (!stateBuffer)
		return;

	Size* frameSize = &stateBuffer->size;
	BOOL ready = stateBuffer->isReady;
	BOOL force = (this->program && this->program->Check() || this->ddraw->viewport.refresh) && frameSize->width && frameSize->height;
	if (ready || force)
	{
		if (ready)
		{
			this->ddraw->bufferIndex = !this->ddraw->bufferIndex;

			lpStateBuffer = (StateBufferAligned**)&(this->ddraw->bufferIndex ? surface->indexBuffer : surface->secondaryBuffer);
			stateBuffer = *lpStateBuffer;
			if (!stateBuffer)
				return;

			stateBuffer->isReady = FALSE;
			surface->drawEnabled = TRUE;
		}
		else
			stateBuffer->isReady = FALSE;

		this->RenderFrame(ready, force, lpStateBuffer);
	}
}

BOOL Renderer::Stop()
{
	if (this->isFinish)
		return FALSE;

	this->isFinish = TRUE;
	return TRUE;
}

BOOL Renderer::CheckView(BOOL isDouble)
{
	BOOL res = this->ddraw->viewport.refresh;

	if (this->ddraw->CheckViewport(isDouble))
		this->Clear();

	return res;
}