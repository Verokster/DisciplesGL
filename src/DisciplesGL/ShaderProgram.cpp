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
#include "ShaderProgram.h"

ShaderProgram::ShaderProgram(const CHAR* version, DWORD vertexName, DWORD fragmentName, DWORD flags, ShaderProgram* last)
{
	this->last = last;

	this->flags = flags;
	this->texSize = 0;

	if (this->flags & SHADER_LEVELS)
	{
		this->colors = (Adjustment*)MemoryAlloc(sizeof(Adjustment));
		MemoryZero(this->colors, sizeof(Adjustment));
	}
	else
		this->colors = NULL;

	this->id = GLCreateProgram();

	GLBindAttribLocation(this->id, 0, "vCoord");
	GLBindAttribLocation(this->id, 1, "vTex");

	CHAR prefix[256];
	StrCopy(prefix, version);

	if (this->flags & SHADER_ALPHA)
		StrCat(prefix, "#define ALPHA\n");
	if (this->flags & SHADER_DOUBLE)
		StrCat(prefix, "#define DOUBLE\n");
	if (this->flags & SHADER_HUE_L)
		StrCat(prefix, "#define LEV_HUE_L\n");
	if (this->flags & SHADER_HUE_R)
		StrCat(prefix, "#define LEV_HUE_R\n");
	if (this->flags & SHADER_SAT)
		StrCat(prefix, "#define LEV_SAT\n");
	if (this->flags & SHADER_LEVELS_IN_RGB)
		StrCat(prefix, "#define LEV_IN_RGB\n");
	if (this->flags & SHADER_LEVELS_IN_A)
		StrCat(prefix, "#define LEV_IN_A\n");
	if (this->flags & SHADER_LEVELS_GAMMA_RGB)
		StrCat(prefix, "#define LEV_GAMMA_RGB\n");
	if (this->flags & SHADER_LEVELS_GAMMA_A)
		StrCat(prefix, "#define LEV_GAMMA_A\n");
	if (this->flags & SHADER_LEVELS_OUT_RGB)
		StrCat(prefix, "#define LEV_OUT_RGB\n");
	if (this->flags & SHADER_LEVELS_OUT_A)
		StrCat(prefix, "#define LEV_OUT_A\n");

	GLuint vShader = GL::CompileShaderSource(vertexName, prefix, GL_VERTEX_SHADER);
	GLuint fShader = GL::CompileShaderSource(fragmentName, prefix, GL_FRAGMENT_SHADER);
	{
		GLAttachShader(this->id, vShader);
		GLAttachShader(this->id, fShader);
		{
			GLLinkProgram(this->id);
		}
		GLDetachShader(this->id, fShader);
		GLDetachShader(this->id, vShader);
	}
	GLDeleteShader(fShader);
	GLDeleteShader(vShader);

	GLUseProgram(this->id);

	GLUniform1i(GLGetUniformLocation(this->id, "tex01"), 0);
	GLint loc = GLGetUniformLocation(this->id, "tex02");
	if (loc >= 0)
		GLUniform1i(loc, 1);

	if (this->flags & SHADER_TEXSIZE)
		this->loc.texSize = GLGetUniformLocation(this->id, "texSize");

	if (this->flags & SHADER_SATHUE)
		this->loc.satHue = GLGetUniformLocation(this->id, "satHue");

	if (this->flags & SHADER_LEVELS_IN)
	{
		this->loc.input.left = GLGetUniformLocation(this->id, "in_left");
		this->loc.input.right = GLGetUniformLocation(this->id, "in_right");
	}

	if (this->flags & SHADER_LEVELS_GAMMA)
		this->loc.gamma = GLGetUniformLocation(this->id, "gamma");

	if (this->flags & SHADER_LEVELS_OUT)
	{
		this->loc.output.left = GLGetUniformLocation(this->id, "out_left");
		this->loc.output.right = GLGetUniformLocation(this->id, "out_right");
	}
}

ShaderProgram::~ShaderProgram()
{
	GLDeleteProgram(this->id);

	if (this->colors)
		MemoryFree(this->colors);
}

VOID ShaderProgram::Update(DWORD texSize, Adjustment* colors)
{
	if ((this->flags & SHADER_TEXSIZE) && this->texSize != texSize)
	{
		this->texSize = texSize;
		GLUniform2f(this->loc.texSize, (FLOAT)LOWORD(texSize), (FLOAT)HIWORD(texSize));
	}

	if (this->flags & SHADER_LEVELS)
	{
		DWORD cmp = CompareAdjustments(this->colors, colors);

		if ((this->flags & SHADER_SATHUE) && (cmp & CMP_SATHUE))
		{
			this->colors->satHue = colors->satHue;
			FLOAT hue = 2.0f * colors->satHue.hueShift;
			GLUniform2f(this->loc.satHue, 4.0f * colors->satHue.saturation * colors->satHue.saturation, hue < 1.0f ? hue : hue - 1.0f);
		}

		if ((this->flags & SHADER_LEVELS_IN) && (cmp & CMP_LEVELS_IN))
		{
			this->colors->input = colors->input;
			GLUniform4f(this->loc.input.left,
				colors->input.left.red,
				colors->input.left.green,
				colors->input.left.blue,
				colors->input.left.rgb);
			GLUniform4f(this->loc.input.right,
				colors->input.right.red,
				colors->input.right.green,
				colors->input.right.blue,
				colors->input.right.rgb);
		}

		if ((this->flags & SHADER_LEVELS_GAMMA) && (cmp & CMP_LEVELS_GAMMA))
		{
			this->colors->gamma = colors->gamma;
			GLUniform4f(this->loc.gamma,
				1.0f / (FLOAT)MathPower(2.0f * colors->gamma.red, 3.32f),
				1.0f / (FLOAT)MathPower(2.0f * colors->gamma.green, 3.32f),
				1.0f / (FLOAT)MathPower(2.0f * colors->gamma.blue, 3.32f),
				1.0f / (FLOAT)MathPower(2.0f * colors->gamma.rgb, 3.32f));
		}

		if ((this->flags & SHADER_LEVELS_OUT) && (cmp & CMP_LEVELS_OUT))
		{
			this->colors->output = colors->output;
			GLUniform4f(this->loc.output.left,
				colors->output.left.red,
				colors->output.left.green,
				colors->output.left.blue,
				colors->output.left.rgb);
			GLUniform4f(this->loc.output.right,
				colors->output.right.red,
				colors->output.right.green,
				colors->output.right.blue,
				colors->output.right.rgb);
		}
	}
}

VOID ShaderProgram::Use()
{
	GLUseProgram(this->id);
}

DWORD ShaderProgram::CompareAdjustments(const Adjustment* cmp1, const Adjustment* cmp2)
{
	DWORD res = 0;
	const DWORD* src = (const DWORD*)cmp1;
	const DWORD* dst = (const DWORD*)cmp2;
	for (DWORD i = 0; i < sizeof(Adjustment) / sizeof(FLOAT); ++i)
		if (src[i] != dst[i])
			res |= (1 << i);

	return res;
}