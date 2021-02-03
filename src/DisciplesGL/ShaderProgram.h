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

#define SHADER_TEXSIZE 0x1
#define SHADER_DOUBLE 0x2
#define SHADER_ALPHA 0x4
#define SHADER_HUE_L 0x8
#define SHADER_HUE_R 0x10
#define SHADER_SAT 0x20
#define SHADER_LEVELS_IN_RGB 0x40
#define SHADER_LEVELS_IN_A 0x80
#define SHADER_LEVELS_GAMMA_RGB 0x100
#define SHADER_LEVELS_GAMMA_A 0x200
#define SHADER_LEVELS_OUT_RGB 0x400
#define SHADER_LEVELS_OUT_A 0x800

#define SHADER_HUE (SHADER_HUE_L | SHADER_HUE_R)
#define SHADER_SATHUE (SHADER_SAT | SHADER_HUE)

#define SHADER_LEVELS_IN (SHADER_LEVELS_IN_RGB | SHADER_LEVELS_IN_A)
#define SHADER_LEVELS_GAMMA (SHADER_LEVELS_GAMMA_RGB | SHADER_LEVELS_GAMMA_A)
#define SHADER_LEVELS_OUT (SHADER_LEVELS_OUT_RGB | SHADER_LEVELS_OUT_A)

#define SHADER_LEVELS (SHADER_SATHUE | SHADER_LEVELS_IN | SHADER_LEVELS_GAMMA | SHADER_LEVELS_OUT)

#define CMP_HUE 0x000001
#define CMP_SAT 0x000002
#define CMP_LEVELS_IN_A 0x000044
#define CMP_LEVELS_IN_RGB 0x0003B8
#define CMP_LEVELS_GAMMA_A 0x000400
#define CMP_LEVELS_GAMMA_RGB 0x003800
#define CMP_LEVELS_OUT_A 0x044000
#define CMP_LEVELS_OUT_RGB 0x3B8000

#define CMP_SATHUE (CMP_SAT | CMP_HUE)
#define CMP_LEVELS_IN (CMP_LEVELS_IN_RGB | CMP_LEVELS_IN_A)
#define CMP_LEVELS_GAMMA (CMP_LEVELS_GAMMA_RGB | CMP_LEVELS_GAMMA_A)
#define CMP_LEVELS_OUT (CMP_LEVELS_OUT_RGB | CMP_LEVELS_OUT_A)

class ShaderProgram : public Allocation {
private:
	GLuint id;
	DWORD texSize;
	struct {
		GLint texSize;
		GLint satHue;
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
	static DWORD CompareAdjustments(const Adjustment*, const Adjustment*);
};
