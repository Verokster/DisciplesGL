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
#include "windows.h"
#include "GLib.h"

struct Rect {
	LONG x;
	LONG y;
	LONG width;
	LONG height;
};

struct VecSize {
	LONG width;
	LONG height;
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
	LONG width;
	LONG height;
	LONG offset;
	Rect rectangle;
	POINTFLOAT viewFactor;
	POINTFLOAT clipFactor;
};

enum RendererType
{
	RendererAuto = 0,
	RendererOpenGL1 = 1,
	RendererOpenGL2 = 2,
	RendererOpenGL3 = 3,
	RendererGDI = 4
};

enum InterpolationFilter : BYTE
{
	InterpolateNearest = 0,
	InterpolateLinear = 1,
	InterpolateHermite = 2,
	InterpolateCubic = 3,
	InterpolateLanczos = 4
};

enum UpscalingFilter : BYTE
{
	UpscaleNone = 0,
	UpscaleXBRZ = 1,
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

union Levels
{
	struct {
		FLOAT rgb;
		FLOAT red;
		FLOAT green;
		FLOAT blue;
	};
	FLOAT chanel[4];
};

struct Adjustment {
	FLOAT hueShift;
	FLOAT saturation;
	struct {
		Levels left;
		Levels right;
	} input;
	Levels gamma;
	struct {
		Levels left;
		Levels right;
	} output;
};

enum ImageType
{
	ImageBMP = 0,
	ImagePNG
};

struct Level {
	FLOAT r;
	FLOAT g;
	FLOAT b;
};

enum BordersType
{
	BordersNone = 0,
	BordersClassic = 1,
	BordersAlternative = 2
};

enum UpdateMode
{
	UpdateNone = 0,
	UpdateCPP = 1,
	UpdateASM = 2
};

enum SceneSort
{
	SceneByTitle = 0,
	SceneByFile = 1,
	SceneBySizeAsc = 2,
	SceneBySizeDesc = 3
};

struct ConfigItems {
	BOOL version;
	BOOL isEditor;
	BOOL windowedMode;
	DisplayMode* mode;
	Resolution resolution;

	HCURSOR cursor;
	HMENU menu;
	HICON icon;
	HFONT font;
	UINT msgSnapshot;
	UINT msgMenu;
	RendererType renderer;
	BOOL hd;
	BOOL bpp32Hooked;
	BOOL resHooked;
	BOOL alwaysActive;
	BOOL coldCPU;
	BOOL singleWindow;
	BOOL singleThread;
	DWORD* package;
	DOUBLE syncStep;
	POINT randPos;

	BOOL drawEnabled;
	UpdateMode updateMode;

	SceneSort sceneSort;
	
	struct {
		BOOL fast;
		union
		{
			DWORD thinking;
			struct {
				BYTE timeout;
				BYTE turn;
			};
		};
		BOOL waiting;
	} ai;

	struct {
		struct {
			DWORD real;
			DWORD value;
		} version;
		struct {
			BOOL clampToEdge;
		} caps;
	} gl;

	struct {
		BOOL active;
		BOOL wide;
		BOOL zoomable;
		BOOL mirror;
	} battle;

	struct {
		BOOL allowed;
		BOOL glallow;
		BOOL enabled;
		DWORD value;
		Size size;
		SizeFloat sizeFloat;
	} zoom;

	struct {
		BOOL allowed;
		BordersType type;
		BOOL active;
	} borders;

	struct {
		BOOL allowed;
		BOOL enabled;
	} background;

	struct {
		BOOL real;
		BOOL mode;
	} borderless;

	struct {
		BOOL hooked;
		BOOL allowed;
	} wide;

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
		BOOL hooked;
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
		BYTE imageFilter;
		BYTE windowedMode;
		BYTE aspectRatio;
		BYTE vSync;
		BYTE zoomImage;
		BYTE speedToggle;
		BYTE snapshot;
	} keys;

	struct {
		const Adjustment* current;
		Adjustment active;
	} colors;

	struct {
		BOOL banners;
		BOOL resources;
	} toogle;

	struct {
		BOOL lButton;
		BOOL mButton;
	} mouseScroll;

	BOOL isExist;
	CHAR file[MAX_PATH];
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

enum SnapshotType
{
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
	MenuColors,
	MenuBorders,
	MenuBackground,
	MenuStretch,
	MenuWindowMode,
	MenuWindowType,
	MenuFastAI,
	MenuActive,
	MenuCpu,
	MenuBattle,
	MenuSnapshot,
	MenuMsgTimeScale,
	MenuRenderer
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

struct GdiObject {
	HBITMAP hBmp;
	VOID* data;
};

union LevelColors
{
	struct {
		DWORD red;
		DWORD green;
		DWORD blue;
	};
	DWORD chanel[3];
};

union LevelColorsFloat
{
	struct {
		FLOAT red;
		FLOAT green;
		FLOAT blue;
	};
	FLOAT chanel[3];
};

struct LevelsData {
	HDC hDc;
	HBITMAP hBmp;
	DWORD* data;
	LevelColorsFloat* colors;
	FLOAT delta;
	Adjustment values;
};

struct StringObject {
	DWORD length;
	DWORD size;
	CHAR* text;
	DWORD unknown;
};

struct SceneObject {
	StringObject fileName; // +0
	StringObject author; // +16
	BYTE isNotCuston; // +32
	BYTE unknown_0[3]; // +33
	DWORD unknown_1; // +36
	StringObject descriptio; // +40
	StringObject title; // +45
	DWORD size; // +72
	DWORD unknown_2[3]; // +76
	DWORD isSaveGame; // +88
	DWORD unknown_3[2]; // +92
	DWORD races; // +100
	DWORD unknown_4[6]; // +104
};