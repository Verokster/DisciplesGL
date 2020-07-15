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
		*this->colors = *config.colors.current;
	}
	else
		this->colors = NULL;

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
	if (MemoryCompare(this->colors, config.colors.current, sizeof(Adjustment)))
	{
		*this->colors = *config.colors.current;
		return TRUE;
	}

	return FALSE;
}

VOID ShaderGroup::Use(DWORD texSize, BOOL isBack)
{
	DWORD flags = NULL;
	if ((this->flags & SHADER_LEVELS) && MemoryCompare(this->colors, &defaultColors, sizeof(Adjustment)))
		flags |= SHADER_LEVELS;
	if ((this->flags & SHADER_DOUBLE) && isBack)
		flags |= SHADER_DOUBLE;
	if (this->flags & SHADER_ALPHA)
		flags |= SHADER_ALPHA;

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