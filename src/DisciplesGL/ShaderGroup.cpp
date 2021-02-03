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
#include "ShaderGroup.h"
#include "Config.h"

ShaderGroup::ShaderGroup(const CHAR* version, DWORD vertexName, DWORD fragmentName, DWORD flags)
{
	this->version = version;
	this->vertexName = vertexName;
	this->fragmentName = fragmentName;
	this->flags = flags;

	if (this->flags & SHADER_LEVELS)
	{
		this->colors = (Adjustment*)MemoryAlloc(sizeof(Adjustment));
		this->update = TRUE;
	}
	else
	{
		this->colors = NULL;
		this->update = FALSE;
	}

	this->current = NULL;
	this->list = NULL;
}

ShaderGroup::~ShaderGroup()
{
	ShaderProgram* item = this->list;
	while (item)
	{
		ShaderProgram* last = item->last;
		delete item;
		item = last;
	}

	if (this->colors)
		MemoryFree(this->colors);
}

BOOL ShaderGroup::Check()
{
	return this->update = this->update || (this->flags & SHADER_LEVELS) && MemoryCompare(this->colors, config.colors.current, sizeof(Adjustment));
}

VOID ShaderGroup::Use(DWORD texSize, BOOL isBack)
{
	if (this->update)
	{
		this->update = FALSE;
		*this->colors = *config.colors.current;
	}

	DWORD flags = this->flags & (SHADER_TEXSIZE | SHADER_ALPHA);
	if ((this->flags & SHADER_DOUBLE) && isBack)
		flags |= SHADER_DOUBLE;

	if (this->flags & SHADER_LEVELS)
	{
		DWORD cmp = ShaderProgram::CompareAdjustments(this->colors, &defaultColors);
		if ((cmp & CMP_HUE) && (this->flags & SHADER_HUE))
			flags |= this->colors->satHue.hueShift < 0.5f ? SHADER_HUE_L : SHADER_HUE_R;
		if (cmp & CMP_SAT)
			flags |= this->flags & SHADER_SAT;
		if (cmp & CMP_LEVELS_IN_RGB)
			flags |= this->flags & SHADER_LEVELS_IN_RGB;
		if (cmp & CMP_LEVELS_IN_A)
			flags |= this->flags & SHADER_LEVELS_IN_A;
		if (cmp & CMP_LEVELS_GAMMA_RGB)
			flags |= this->flags & SHADER_LEVELS_GAMMA_RGB;
		if (cmp & CMP_LEVELS_GAMMA_A)
			flags |= this->flags & SHADER_LEVELS_GAMMA_A;
		if (cmp & CMP_LEVELS_OUT_RGB)
			flags |= this->flags & SHADER_LEVELS_OUT_RGB;
		if (cmp & CMP_LEVELS_OUT_A)
			flags |= this->flags & SHADER_LEVELS_OUT_A;
	}

	if (this->current && this->current->flags == flags)
		this->current->Use();
	else
	{
		this->current = NULL;

		ShaderProgram* item = this->list;
		while (item)
		{
			if (item->flags == flags)
			{
				this->current = item;
				this->current->Use();
				break;
			}

			item = item->last;
		}

		if (!this->current)
			this->current = this->list = new ShaderProgram(this->version, this->vertexName, this->fragmentName, flags, this->list);
	}

	this->current->Update(texSize, this->colors);
}