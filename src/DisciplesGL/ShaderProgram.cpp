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
#include "Config.h"

#define M_PI 3.14159265358979323846f

ShaderProgram::ShaderProgram(const CHAR* version, DWORD vertexName, DWORD fragmentName)
{
	this->id = 0;
	this->version = version;
	this->vertexName = vertexName;
	this->fragmentName = fragmentName;
	this->texSize = 0;
	MemorySet(&this->loc, 0xFF, sizeof(this->loc));
}

VOID ShaderProgram::UpdateLevels()
{
	this->colors = *config.colors.current;
	this->update = FALSE;

	GLUniform1f(this->loc.hue, (2.0f * this->colors.hueShift - 1.0f) * M_PI);
	GLUniform1f(this->loc.sat, (FLOAT)MathPower(2.0f * this->colors.saturation, 1.5849625f));

	GLUniform4f(this->loc.input.left,
		this->colors.input.left.red,
		this->colors.input.left.green,
		this->colors.input.left.blue,
		this->colors.input.left.rgb);

	GLUniform4f(this->loc.input.right,
		this->colors.input.right.red,
		this->colors.input.right.green,
		this->colors.input.right.blue,
		this->colors.input.right.rgb);

	GLUniform4f(this->loc.gamma,
		1.0f / (FLOAT)MathPower(2.0f * this->colors.gamma.red, 3.32f),
		1.0f / (FLOAT)MathPower(2.0f * this->colors.gamma.green, 3.32f),
		1.0f / (FLOAT)MathPower(2.0f * this->colors.gamma.blue, 3.32f),
		1.0f / (FLOAT)MathPower(2.0f * this->colors.gamma.rgb, 3.32f));

	GLUniform4f(this->loc.output.left,
		this->colors.output.left.red,
		this->colors.output.left.green,
		this->colors.output.left.blue,
		this->colors.output.left.rgb);

	GLUniform4f(this->loc.output.right,
		this->colors.output.right.red,
		this->colors.output.right.green,
		this->colors.output.right.blue,
		this->colors.output.right.rgb);
}

BOOL ShaderProgram::Check()
{
	return this->update = this->loc.hue >= 0 && MemoryCompare(&this->colors, config.colors.current, sizeof(Adjustment));
}

VOID ShaderProgram::Use(DWORD texSize)
{
	if (!this->id)
	{
		this->id = GLCreateProgram();

		GLBindAttribLocation(this->id, 0, "vCoord");
		GLBindAttribLocation(this->id, 1, "vTex");

		GLuint vShader = GL::CompileShaderSource(this->vertexName, this->version, GL_VERTEX_SHADER);
		GLuint fShader = GL::CompileShaderSource(this->fragmentName, this->version, GL_FRAGMENT_SHADER);
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
		if (this->loc.texSize >= 0)
		{
			this->texSize = texSize;
			GLUniform2f(this->loc.texSize, (FLOAT)LOWORD(texSize), (FLOAT)HIWORD(texSize));
		}

		this->loc.hue = GLGetUniformLocation(this->id, "hue");
		if (this->loc.hue >= 0)
		{
			this->loc.sat = GLGetUniformLocation(this->id, "sat");
			this->loc.input.left = GLGetUniformLocation(this->id, "in_left");
			this->loc.input.right = GLGetUniformLocation(this->id, "in_right");
			this->loc.gamma = GLGetUniformLocation(this->id, "gamma");
			this->loc.output.left = GLGetUniformLocation(this->id, "out_left");
			this->loc.output.right = GLGetUniformLocation(this->id, "out_right");

			this->UpdateLevels();
		}
		else
			this->update = FALSE;
	}
	else
	{
		GLUseProgram(this->id);

		if (this->loc.texSize >= 0 && this->texSize != texSize)
		{
			this->texSize = texSize;
			GLUniform2f(this->loc.texSize, (FLOAT)LOWORD(texSize), (FLOAT)HIWORD(texSize));
		}

		if (this->update)
			this->UpdateLevels();
	}
}

VOID ShaderProgram::Release()
{
	if (this->id)
		GLDeleteProgram(this->id);

	delete this;
}
