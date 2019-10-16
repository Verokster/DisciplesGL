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

enum InterpolationFilter : BYTE
{
	InterpolateNearest = 0,
	InterpolateLinear = 1,
	InterpolateHermite = 2,
	InterpolateCubic = 3
};

enum UpscalingFilter : BYTE
{
	UpscaleNone = 0,
	UpscaleXRBZ = 1,
	UpscaleScaleHQ = 2,
	UpscaleXSal = 3,
	UpscaleEagle = 4,
	UpscaleScaleNx = 5
};

struct FilterState {
	InterpolationFilter interpolation;
	UpscalingFilter upscaling;
	BYTE value;
	BYTE flags;
};

struct Resolution {
	WORD width;
	WORD height;
};

struct Size {
	DWORD width;
	DWORD height;
};

struct SizeFloat {
	FLOAT width;
	FLOAT height;
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
	Uniform texSize;
};

enum ImageType
{
	ImageBMP = 0,
	ImagePNG
};

struct ConfigItems {
	BOOL version;
	BOOL isEditor;
	BOOL windowedMode;
	DisplayMode* mode;
	Size zoomed;
	SizeFloat zoomedFloat;
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
	BOOL borderlessReal;
	BOOL wideHooked;
	BOOL wideAllowed;
	BOOL isBattle;
	BOOL isWideBattle;
	BOOL isWideZoomable;
	BOOL singleWindow;
	BOOL singleThread;
	BOOL isAiThinking;
	BOOL isWaiting;
	DOUBLE syncStep;
	POINT randPos;
	FLOAT zoomFactor;
	BOOL fastAI;

	struct {
		BOOL hooked;
		DWORD time;
		DOUBLE value;
	} msgTimeScale;

	struct {
		struct {
			LCID id;
			DWORD oem;
			DWORD ansi;
		} current;
		HMENU menus[('Z' - 'A') + 1];
		DWORD count;
		LCID* list;
	} locales;
	
	struct {
		ImageType type;
		DWORD level;
	} snapshot;

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
		BYTE scaleNx;
		BYTE xSal;
		BYTE eagle;
		BYTE scaleHQ;
		BYTE xBRz;
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
		BYTE snapshot;
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
	HMENU hMenu;
	UINT index;
	UINT childId;
};

struct Stream {
	BYTE* data;
	DWORD size;
	DWORD position;
};

struct WinMessage {
	WinMessage* prev;
	UINT id;
	CHAR* name;
};

enum SnapshotType {
	SnapshotNone = 0,
	SnapshotFile,
	SnapshotClipboard,
	SnapshotSurfaceFile,
	SnapshotSurfaceClipboard
};

enum MenuType
{
	MenuLocale,
	MenuAspect,
	MenuVSync,
	MenuInterpolate,
	MenuUpscale,
	MenuResolution,
	MenuSpeed,
	MenuBorders,
	MenuStretch,
	MenuWindowMode,
	MenuWindowType,
	MenuFastAI,
	MenuActive,
	MenuCpu,
	MenuBattle,
	MenuSnapshotType,
	MenuSnapshotLevel,
	MenuMsgTimeScale
};

struct SpritePosition {
	POINT dstPos;
	Rect srcRect;
};

struct ImageIndices {
	Size size;
	const VOID* lpIndices;
	DWORD count;
	SpritePosition indices[];
};

struct TimeScale {
	DWORD real;
	DWORD virt;
	DWORD lastReal;
	DWORD lastVirt;
	DOUBLE scale;
};