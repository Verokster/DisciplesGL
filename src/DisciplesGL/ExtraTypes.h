/*
	MIT License

	Copyright (c) 2019 Oleksiy Ryabchun

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
#include "windows.h"
#include "GLib.h"

struct Rect {
	INT x;
	INT y;
	INT width;
	INT height;
};

struct VecSize {
	INT width;
	INT height;
};

struct TexSize {
	FLOAT width;
	FLOAT height;
};

struct Frame {
	GLuint id[2];
	Rect rect;
	POINT point;
	VecSize vSize;
	TexSize tSize;
};

struct Viewport {
	BOOL refresh;
	INT width;
	INT height;
	INT offset;
	Rect rectangle;
	POINTFLOAT viewFactor;
	POINTFLOAT clipFactor;
};

enum InterpolationFilter
{
	InterpolateNearest = 0,
	InterpolateLinear = 1,
	InterpolateHermite = 2,
	InterpolateCubic = 3
};

enum UpscalingFilter
{
	UpscaleNone = 0,
	UpscaleXRBZ = 1,
	UpscaleScaleHQ = 2,
	UpscaleXSal = 3,
	UpscaleEagle = 4,
	UpscaleScaleNx = 5
};

struct Resolution {
	WORD width;
	WORD height;
};

struct DisplayMode {
	DWORD width;
	DWORD height;
	DWORD bpp;
};

enum FpsState
{
	FpsDisabled = 0,
	FpsNormal,
	FpsBenchmark
};

struct Uniform {
	GLint location;
	DWORD value;
};

struct ShaderProgram {
	GLuint id;
	const CHAR* version;
	DWORD vertexName;
	DWORD fragmentName;
	GLfloat* mvp;
	Uniform texSize;
};

struct ConfigItems {
	BOOL version;
	BOOL windowedMode;
	DisplayMode* mode;
	Resolution resolution;

	HCURSOR cursor;
	HMENU menu;
	HICON icon;
	HFONT font;
	BOOL hd;
	BOOL bpp32Hooked;
	BOOL resHooked;
	BOOL isBorder;
	BOOL showBackBorder;
	BOOL zoomable;
	BOOL zoomImage;
	BOOL menuZoomImage;
	BOOL gameBorders;
	BOOL alwaysActive;
	BOOL coldCPU;
	BOOL borderlessMode;
	POINT randPos;

	struct {
		DWORD index;
		DOUBLE value;
		BOOL enabled;
	} speed;

	struct {
		BOOL aspect;
		BOOL vSync;
		InterpolationFilter interpolation;
		UpscalingFilter upscaling;
		INT scaleNx;
		INT xSal;
		INT eagle;
		INT scaleHQ;
		INT xBRz;
	} image;

	struct {
		BYTE fpsCounter;
		BYTE imageFilter;
		BYTE windowedMode;
		BYTE aspectRatio;
		BYTE vSync;
		BYTE showBorders;
		BYTE zoomImage;
		BYTE speedToggle;
	} keys;

	BOOL isExist;
	CHAR file[MAX_PATH];
};

struct MappedFile {
	HMODULE hModule;
	HANDLE hFile;
	HANDLE hMap;
	VOID* address;
};

enum SurfaceType
{
	SurfacePrimary,
	SurfaceSecondary,
	SurfaceOther
};

struct MenuItemData {
	HMENU hParent;
	INT index;
	UINT childId;
};

struct ResourceStream {
	VOID* data;
	DWORD position;
};