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

#pragma once

#include "OGLRenderer.h"

#pragma once
class NewRenderer : public OGLRenderer {
private:
	DWORD texSize;
	ShaderGroup* upscaleProgram;
	Size zoomSize;
	Size zoomFbSize;
	GLuint fboId;
	GLuint rboId;
	VOID* frameBuffer;
	DWORD viewSize;
	BOOL activeIndex;
	BOOL zoomStatus;
	FLOAT buffer[8][8];
	GLuint bufferName;
	GLuint arrayName;
	
	struct {
		ShaderGroup* linear;
		ShaderGroup* hermite;
		ShaderGroup* cubic;
		ShaderGroup* xBRz_2x;
		ShaderGroup* xBRz_3x;
		ShaderGroup* xBRz_4x;
		ShaderGroup* xBRz_5x;
		ShaderGroup* xBRz_6x;
		ShaderGroup* scaleHQ_2x;
		ShaderGroup* scaleHQ_4x;
		ShaderGroup* xSal_2x;
		ShaderGroup* eagle_2x;
		ShaderGroup* scaleNx_2x;
		ShaderGroup* scaleNx_3x;
	} shaders;

	struct {
		GLuint back;
		GLuint primary;
		GLuint secondary;
		GLuint primaryBO;
		GLuint backBO;
	} textureId;

	VOID Begin();
	VOID End();
	BOOL RenderInner(BOOL, BOOL, StateBufferAligned**);

public:
	NewRenderer(OpenDraw*);
};
