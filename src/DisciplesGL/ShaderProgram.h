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

#include "Allocation.h"
#include "ExtraTypes.h"

#define SHADER_DOUBLE 0x1
#define SHADER_LEVELS 0x2
#define SHADER_ALPHA 0x4

class ShaderProgram : public Allocation {
private:
	GLuint id;
	DWORD texSize;
	struct {
		GLint texSize;
		GLint hue;
		GLint sat;
		struct {
			GLint left;
			GLint right;
		} input;
		GLint gamma;
		struct {
			GLint left;
			GLint right;
		} output;
	} loc;

	Adjustment* colors;

public:
	ShaderProgram* last;
	DWORD flags;
	ShaderProgram(const CHAR*, DWORD, DWORD, DWORD, ShaderProgram*);
	~ShaderProgram();

	VOID Use();
	VOID Update(DWORD, Adjustment*);
};
