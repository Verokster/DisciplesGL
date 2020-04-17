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

#define M_PI 3.14159265358979323846f

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
	if (this->flags & SHADER_LEVELS)
		StrCat(prefix, "#define LEVELS\n");

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

	this->loc.texSize = GLGetUniformLocation(this->id, "texSize");

	if (this->flags & SHADER_LEVELS)
	{
		this->loc.hue = GLGetUniformLocation(this->id, "hue");
		this->loc.sat = GLGetUniformLocation(this->id, "sat");
		this->loc.input.left = GLGetUniformLocation(this->id, "in_left");
		this->loc.input.right = GLGetUniformLocation(this->id, "in_right");
		this->loc.gamma = GLGetUniformLocation(this->id, "gamma");
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
	if (this->loc.texSize >= 0 && this->texSize != texSize)
	{
		this->texSize = texSize;
		GLUniform2f(this->loc.texSize, (FLOAT)LOWORD(texSize), (FLOAT)HIWORD(texSize));
	}

	if ((this->flags & SHADER_LEVELS) && MemoryCompare(this->colors, colors, sizeof(Adjustment)))
	{
		*this->colors = *colors;

		GLUniform1f(this->loc.hue, (2.0f * colors->hueShift - 1.0f) * M_PI);
		GLUniform1f(this->loc.sat, (FLOAT)MathPower(2.0f * colors->saturation, 1.5849625f));

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

		GLUniform4f(this->loc.gamma,
			1.0f / (FLOAT)MathPower(2.0f * colors->gamma.red, 3.32f),
			1.0f / (FLOAT)MathPower(2.0f * colors->gamma.green, 3.32f),
			1.0f / (FLOAT)MathPower(2.0f * colors->gamma.blue, 3.32f),
			1.0f / (FLOAT)MathPower(2.0f * colors->gamma.rgb, 3.32f));

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

VOID ShaderProgram::Use()
{
	GLUseProgram(this->id);
}