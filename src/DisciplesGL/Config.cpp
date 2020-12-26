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
#include "Config.h"
#include "intrin.h"
#include "Resource.h"
#include "Window.h"

LONG GAME_WIDTH;
LONG GAME_HEIGHT;

FLOAT GAME_WIDTH_FLOAT;
FLOAT GAME_HEIGHT_FLOAT;

ConfigItems config;

DisplayMode modesList[] = {
	640, 480, 8,
	800, 600, 16,
	1024, 768, 16,
	1280, 1024, 16
};

const Resolution resolutionsList[] = {
	640, 480,
	800, 600,
	960, 768,
	1024, 600,
	1024, 768,
	1152, 864,
	1280, 720,
	1280, 768,
	1280, 800,
	1280, 960,
	1280, 1024,
	1360, 768,
	1366, 768,
	1400, 1050,
	1440, 900,
	1440, 1080,
	1536, 864,
	1600, 900,
	1600, 1200,
	1680, 1050,
	1920, 1080,
	1920, 1200,
	1920, 1440,
	2048, 1536,
	2560, 1440,
	2560, 1600,
	3840, 2160,
	7680, 4320
};

Adjustment activeColors;
const Adjustment inactiveColors = {
	0.5f,
	0.0f,

	0.0f,
	0.15200001f,
	0.0700000003f,
	0.0f,

	1.0f,
	1.0f,
	1.0f,
	1.0f,

	0.5f,
	0.604000032f,
	0.539000034f,
	0.478000015f,

	0.0430000015f,
	0.0f,
	0.0f,
	0.0f,

	1.0f,
	1.0f,
	1.0f,
	1.0f
};

const Adjustment defaultColors = {
	0.5f,
	0.5f,

	0.0f,
	0.0f,
	0.0f,
	0.0f,

	1.0f,
	1.0f,
	1.0f,
	1.0f,

	0.5f,
	0.5f,
	0.5f,
	0.5f,

	0.0f,
	0.0f,
	0.0f,
	0.0f,

	1.0f,
	1.0f,
	1.0f,
	1.0f
};

namespace Config
{
	BOOL __stdcall EnumLocalesCount(LPTSTR lpLocaleString)
	{
		++config.locales.count;

		DWORD locale = strtoul(lpLocaleString, NULL, 16);

		CHAR name[64];
		GetLocaleInfo(locale, LOCALE_SENGLISHLANGUAGENAME, name, 64);

		CHAR ch = ToUpper(*name);
		HMENU* hPopup = &config.locales.menus[ch - 'A'];

		if (!*hPopup)
			*hPopup = CreatePopupMenu();

		return TRUE;
	}

	DWORD localeIndex;
	BOOL __stdcall EnumLocales(LPTSTR lpLocaleString)
	{
		DWORD locale = strtoul(lpLocaleString, NULL, 16);

		CHAR name[64];
		GetLocaleInfo(locale, LOCALE_SENGLISHLANGUAGENAME, name, 64);

		CHAR ch = ToUpper(*name);
		HMENU* hPopup = &config.locales.menus[ch - 'A'];
		if (*hPopup)
		{
			config.locales.list[++localeIndex] = locale;

			CHAR country[64];
			GetLocaleInfo(locale, LOCALE_SENGLISHCOUNTRYNAME, country, 64);

			CHAR title[256];
			StrPrint(title, "%s (%s)", name, country);

			CHAR temp[256];
			BOOL inserted = FALSE;
			INT count = GetMenuItemCount(*hPopup);
			for (INT i = 0; i < count; ++i)
			{
				if (GetMenuString(*hPopup, i, temp, sizeof(temp), MF_BYPOSITION))
				{
					if (StrCompare(title, temp) < 0)
					{
						InsertMenu(*hPopup, i, MF_STRING | MF_BYPOSITION, IDM_LOCALE_DEFAULT + localeIndex, title);
						inserted = TRUE;
						break;
					}
				}
			}

			if (!inserted)
				AppendMenu(*hPopup, MF_STRING, IDM_LOCALE_DEFAULT + localeIndex, title);
		}

		return TRUE;
	}

	BOOL __fastcall Load()
	{
		HMODULE hModule = GetModuleHandle(NULL);

		GetModuleFileName(hModule, config.file, MAX_PATH - 1);
		CHAR* p = StrLastChar(config.file, '\\');
		*p = NULL;
		StrCopy(p, "\\disciple.ini");

		DWORD base = (DWORD)hModule;
		PIMAGE_NT_HEADERS headNT = (PIMAGE_NT_HEADERS)((BYTE*)base + ((PIMAGE_DOS_HEADER)hModule)->e_lfanew);

		PIMAGE_DATA_DIRECTORY dataDir = &headNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
		if (dataDir->Size)
		{
			PIMAGE_IMPORT_DESCRIPTOR imports = (PIMAGE_IMPORT_DESCRIPTOR)(base + dataDir->VirtualAddress);
			for (DWORD idx = 0; imports->Name; ++idx, ++imports)
			{
				CHAR* libraryName = (CHAR*)(base + imports->Name);
				if (!StrCompareInsensitive(libraryName, "smackw32.dll"))
					config.version = TRUE;

				if (!StrCompareInsensitive(libraryName, "comdlg32.dll"))
					config.isEditor = TRUE;
			}
		}

		if (config.version)
		{
			GAME_WIDTH = 640;
			GAME_HEIGHT = 480;
		}
		else
		{
			GAME_WIDTH = 800;
			GAME_HEIGHT = 600;
		}

		GAME_WIDTH_FLOAT = (FLOAT)GAME_WIDTH;
		GAME_HEIGHT_FLOAT = (FLOAT)GAME_HEIGHT;

		config.isExist = !(BOOL)Config::Get(CONFIG_WRAPPER, "ReInit", TRUE);

		if (!config.isExist)
		{
			config.windowedMode = TRUE;

			if (config.version)
			{
				Config::Set(CONFIG_DISCIPLE, "DDraw", TRUE);
				Config::Set(CONFIG_DISCIPLE, "InWindow", config.windowedMode);
			}
			else
			{
				Config::Set(CONFIG_DISCIPLE, "UseD3D", FALSE);
				Config::Set(CONFIG_DISCIPLE, "DisplayMode", config.windowedMode);
			}

			{
				config.hd = TRUE;
				config.image.aspect = TRUE;
				config.image.vSync = TRUE;
				config.image.interpolation = InterpolateHermite;
				config.image.upscaling = UpscaleNone;
				config.image.scaleNx = 2;
				config.image.xSal = 2;
				config.image.eagle = 2;
				config.image.scaleHQ = 2;
				config.image.xBRz = 2;

				if (!config.version)
				{
					Config::Set(CONFIG_DISCIPLE, "ShowInterfBorder", TRUE);
					Config::Set(CONFIG_DISCIPLE, "EnableZoom", TRUE);

					config.borders.type = BordersClassic;
					config.background.enabled = TRUE;
					config.battle.mirror = TRUE;
				}

				config.zoom.enabled = TRUE;
				config.zoom.value = 100;

				config.speed.index = 5;
				config.speed.value = 0.1f * (config.speed.index + 10);
				config.speed.enabled = TRUE;

				config.snapshot.type = ImagePNG;
				config.snapshot.level = 9;
				config.locales.current.id = GetUserDefaultLCID();
				config.msgTimeScale.time = 15;
				config.msgTimeScale.value = (FLOAT)MSG_TIMEOUT / config.msgTimeScale.time;
				config.updateMode = UpdateSSE;
				config.mouseScroll.lButton = config.mouseScroll.mButton = TRUE;
				config.cloudsFactor = 1.0f;

				CHAR* buffer = (CHAR*)MemoryAlloc(8192);
				if (buffer)
				{
					CHAR* ptr = buffer;
					ptr = Add(ptr, "ReInit", FALSE, "Reset all wrapper configurations (0 - no; 1 - yes)");

					ptr = Add(ptr, "ShowBanners", config.toogle.banners, "Show in-game banners for troops (0 - no 'default'; 1 - yes)");
					ptr = Add(ptr, "ShowResources", config.toogle.resources, "Show resources panel (0 - no 'default'; 1 - yes)");
					ptr = Add(ptr, "SceneSort", *(INT*)&config.sceneSort, "Sort scenarios (0 - by title 'default'; 1 - by file name; 2 - by map size 'ascending'; 3 - by map size 'descending')");
					ptr = Add(ptr, "Renderer", *(INT*)&config.renderer, "Image renderer (0 - auto 'default'; 1 - opengl 1.1; 2 - opengl 2.0; 3 - opengl 3.0; 4 - windows gdi)");
					ptr = Add(ptr, "HD", config.hd, "Enables HD options, like 32 bpp rendering and different resolutions support (0 - no; 1 - yes 'default')");
					
					if (config.version)
					{
						ptr = Add(ptr, "DisplayWidth", GAME_WIDTH, "Image resolution width (640 - min; 2048 - max)");
						ptr = Add(ptr, "DisplayHeight", GAME_HEIGHT, "Image resolution height (480 - min; 1024 - max)");
					}
					else
					{
						ptr = Add(ptr, "DisplayWidth", GAME_WIDTH, "Image resolution width (800 - min)");
						ptr = Add(ptr, "DisplayHeight", GAME_HEIGHT, "Image resolution height (600 - min)");
					}
					
					ptr = Add(ptr, "ImageAspect", config.image.aspect, "Keep image aspect ratio (0 - no; 1 - yes 'default')");
					ptr = Add(ptr, "ImageVSync", config.image.vSync, "Enables vertical synchronization (0 - no; 1 - yes 'default')");
					ptr = Add(ptr, "Interpolation", (INT)config.image.interpolation, "Image interpolation filter (0 - none; 1 - linear; 2 - hermite 'default'; 3 - cubic; 4 - lanczos)");
					ptr = Add(ptr, "Upscaling", (INT)config.image.upscaling, "Image upscaling filter (0 - none 'default'; 1 - xbrz; 2 - scalehq; 3 - xsal; 4 - eagle; 5 - scalenx)");
					ptr = Add(ptr, "ScaleNx", config.image.scaleNx, "ScaleNx scale factor (2 'default'; 3)");
					ptr = Add(ptr, "XSal", config.image.xSal, "XSal scale factor (2 'default')");
					ptr = Add(ptr, "Eagle", config.image.eagle, "Eagle scale factor (2 'default')");
					ptr = Add(ptr, "ScaleHQ", config.image.scaleHQ, "ScaleHQ scale factor (2 'default')");
					ptr = Add(ptr, "XBRZ", config.image.xBRz, "xBRZ scale factor (2 'default'; 3; 4; 5; 6)");

					if (!config.version)
					{
						ptr = Add(ptr, "Borders", *(INT*)&config.borders.type, "Enables borders for in-game windows (0 - no; 1 - classic style 'default', 2 - alternative style)");
						ptr = Add(ptr, "Background", config.background.enabled, "Enables background for in-game windows (0 - no; 1 - yes 'default')");
					}

					ptr = Add(ptr, "EnableZoom", config.zoom.enabled, "Enables in-game windows zoom (0 - no; 1 - yes 'default')");
					ptr = Add(ptr, "ZoomFactor", *(INT*)&config.zoom.value, "Zoom factor for in-game windows (0 - 100 'default')");
					ptr = Add(ptr, "FullScreenMode", config.borderless.mode, "Full screen window mode (0 - exclusive 'default'; 1 - borderless)");
					ptr = Add(ptr, "GameSpeed", config.speed.index, "Animation speed (1 - ..., 5 - 1.5x 'deafult'");
					ptr = Add(ptr, "SpeedEnabled", config.speed.enabled, "Enables animation speed (0 - no; 1 - yes 'default')");
					ptr = Add(ptr, "AlwaysActive", config.alwaysActive, "Game window is always active (0 - no 'default'; 1 - yes)");
					ptr = Add(ptr, "ColdCPU", config.coldCPU, "Decrease CPU usage for OpenGL renderer (0 - no 'default'; 1 - yes)");

					if (!config.version)
					{
						ptr = Add(ptr, "WideBattle", config.wide.allowed, "Makes window battle wider. Depends on image aspect ratio (0 - no; 1 - yes)");
						ptr = Add(ptr, "MirrorBattle", config.battle.mirror, "Allows mirrored bacgrounds for battles (0 - no; 1 - yes 'default')");
					}

					ptr = Add(ptr, "ScreenshotType", *(INT*)&config.snapshot.type, "Screenshots type (0 - bmp; 1 - png 'default')");
					ptr = Add(ptr, "ScreenshotLevel", *(INT*)&config.snapshot.level, "Compression level for PNG screenshots (0 - 9, 5 - 'default')");
					ptr = Add(ptr, "Locale", *(INT*)&config.locales.current.id, "Locale id (by default is determined by system locale)");
					ptr = Add(ptr, "MessageTimeout", config.msgTimeScale.time, "In-game messages timeout in seconds (15 'default')");
					ptr = Add(ptr, "UpdateMode", (INT)config.updateMode, "Image comparison. Affects on rendering performance (0 - none; 1 - SSE2 'default'; 2 - classic 'default'; 3 - alternative)");
					ptr = Add(ptr, "FastAI", config.ai.fast, "Increases AI performance, but may cause an unexpected game crash (0 - no 'default'; 1 - yes)");
					ptr = Add(ptr, "MouseScroll", 3, "Allows map scrolling by pressing mouse button (0 - no; 1 - left button; 2 - middle button; 3 - both buttons 'default')");
					ptr = Add(ptr, "CloudsFactor", 1, "Defines clouds count multiplier (1 - min 'default')");
					*ptr = NULL;

					WritePrivateProfileSection(CONFIG_WRAPPER, buffer + (*(WORD*)buffer == '\r\n' ? 2 : 0), config.file);
					MemoryFree(buffer);
				}
			}

			{
				config.keys.imageFilter = 3;
				Config::Set(CONFIG_KEYS, "ImageFilter", config.keys.imageFilter);

				config.keys.windowedMode = 4;
				Config::Set(CONFIG_KEYS, "WindowedMode", config.keys.windowedMode);

				Config::Set(CONFIG_KEYS, "AspectRatio", "");
				Config::Set(CONFIG_KEYS, "VSync", "");

				if (!config.version)
					Config::Set(CONFIG_KEYS, "ZoomImage", "");

				config.keys.speedToggle = 5;
				Config::Set(CONFIG_KEYS, "SpeedToggle", config.keys.speedToggle);

				config.keys.snapshot = 12;
				Config::Set(CONFIG_KEYS, "Screenshot", config.keys.snapshot);
			}

			{
				config.colors.active.hueShift = 0.5f;
				config.colors.active.saturation = 0.5f;
				Config::Set(CONFIG_COLORS, "HueSat", 0x01F401F4);

				config.colors.active.input.left.rgb = 0.0f;
				config.colors.active.input.right.rgb = 1.0f;
				Config::Set(CONFIG_COLORS, "RgbInput", 0x03E80000);

				config.colors.active.input.left.red = 0.0f;
				config.colors.active.input.right.red = 1.0f;
				Config::Set(CONFIG_COLORS, "RedInput", 0x03E80000);

				config.colors.active.input.left.green = 0.0f;
				config.colors.active.input.right.green = 1.0f;
				Config::Set(CONFIG_COLORS, "GreenInput", 0x03E80000);

				config.colors.active.input.left.blue = 0.0f;
				config.colors.active.input.right.blue = 1.0f;
				Config::Set(CONFIG_COLORS, "BlueInput", 0x03E80000);

				config.colors.active.gamma.rgb = 0.5f;
				Config::Set(CONFIG_COLORS, "RgbGamma", 500);

				config.colors.active.gamma.red = 0.5f;
				Config::Set(CONFIG_COLORS, "RedGamma", 500);

				config.colors.active.gamma.green = 0.5f;
				Config::Set(CONFIG_COLORS, "GreenGamma", 500);

				config.colors.active.gamma.blue = 0.5f;
				Config::Set(CONFIG_COLORS, "BlueGamma", 500);

				config.colors.active.output.left.rgb = 0.0f;
				config.colors.active.output.right.rgb = 1.0f;
				Config::Set(CONFIG_COLORS, "RgbOutput", 0x03E80000);

				config.colors.active.output.left.red = 0.0f;
				config.colors.active.output.right.red = 1.0f;
				Config::Set(CONFIG_COLORS, "RedOutput", 0x03E80000);

				config.colors.active.output.left.green = 0.0f;
				config.colors.active.output.right.green = 1.0f;
				Config::Set(CONFIG_COLORS, "GreenOutput", 0x03E80000);

				config.colors.active.output.left.blue = 0.0f;
				config.colors.active.output.right.blue = 1.0f;
				Config::Set(CONFIG_COLORS, "BlueOutput", 0x03E80000);
			}
		}
		else
		{
			if (config.version && !Config::Get(CONFIG_DISCIPLE, "DDraw", TRUE) || !config.version && Config::Get(CONFIG_DISCIPLE, "UseD3D", FALSE))
				return FALSE;

			config.windowedMode = (BOOL)Config::Get(CONFIG_DISCIPLE, config.version ? "InWindow" : "DisplayMode", TRUE);

			{
				config.toogle.banners = (BOOL)Config::Get(CONFIG_WRAPPER, "ShowBanners", FALSE);
				config.toogle.resources = (BOOL)Config::Get(CONFIG_WRAPPER, "ShowResources", FALSE);

				INT value = Config::Get(CONFIG_WRAPPER, "SceneSort", SceneByTitle);
				config.sceneSort = *(SceneSort*)&value;
				if (config.sceneSort < SceneByTitle || config.sceneSort > SceneBySizeDesc)
					config.sceneSort = SceneByTitle;

				value = Config::Get(CONFIG_WRAPPER, "Renderer", RendererAuto);
				config.renderer = *(RendererType*)&value;
				if (config.renderer < RendererAuto || config.renderer > RendererGDI)
					config.renderer = RendererAuto;

				config.hd = (BOOL)Config::Get(CONFIG_WRAPPER, "HD", TRUE);

				config.image.aspect = (BOOL)Config::Get(CONFIG_WRAPPER, "ImageAspect", TRUE);
				config.image.vSync = (BOOL)Config::Get(CONFIG_WRAPPER, "ImageVSync", TRUE);

				value = Config::Get(CONFIG_WRAPPER, "Interpolation", InterpolateHermite);
				config.image.interpolation = *(InterpolationFilter*)&value;
				if (config.image.interpolation < InterpolateNearest || config.image.interpolation > InterpolateLanczos)
					config.image.interpolation = InterpolateHermite;

				value = Config::Get(CONFIG_WRAPPER, "Upscaling", UpscaleNone);
				config.image.upscaling = *(UpscalingFilter*)&value;
				if (config.image.upscaling < UpscaleNone || config.image.upscaling > UpscaleScaleNx)
					config.image.upscaling = UpscaleNone;

				config.image.scaleNx = Config::Get(CONFIG_WRAPPER, "ScaleNx", 2);
				if (config.image.scaleNx != 2 && config.image.scaleNx != 3)
					config.image.scaleNx = 2;

				config.image.xSal = Config::Get(CONFIG_WRAPPER, "XSal", 2);
				if (config.image.xSal != 2)
					config.image.xSal = 2;

				config.image.eagle = Config::Get(CONFIG_WRAPPER, "Eagle", 2);
				if (config.image.eagle != 2)
					config.image.eagle = 2;

				config.image.scaleHQ = Config::Get(CONFIG_WRAPPER, "ScaleHQ", 2);
				if (config.image.scaleHQ != 2 && config.image.scaleHQ != 4)
					config.image.scaleHQ = 2;

				config.image.xBRz = Config::Get(CONFIG_WRAPPER, "XBRZ", 2);
				if (config.image.xBRz < 2 || config.image.xBRz > 6)
					config.image.xBRz = 6;

				if (!config.version)
				{
					BOOL value = Config::Get(CONFIG_DISCIPLE, "ShowInterfBorder", TRUE);
					config.borders.type = (BordersType)Config::Get(CONFIG_WRAPPER, "Borders", value ? BordersClassic : BordersNone);
					config.background.enabled = (BOOL)Config::Get(CONFIG_WRAPPER, "Background", value);

					config.zoom.enabled = (BOOL)Config::Get(CONFIG_DISCIPLE, "EnableZoom", TRUE);
				}
				else
					config.zoom.enabled = (BOOL)Config::Get(CONFIG_WRAPPER, "EnableZoom", TRUE);

				config.borderless.real = config.borderless.mode = (BOOL)Config::Get(CONFIG_WRAPPER, "FullScreenMode", FALSE);

				value = Config::Get(CONFIG_WRAPPER, "GameSpeed", 5);
				if (value < 1)
					value = 5;
				config.speed.index = *(DWORD*)&value;
				config.speed.value = 0.1f * (config.speed.index + 10);

				config.speed.enabled = (BOOL)Config::Get(CONFIG_WRAPPER, "SpeedEnabled", TRUE);

				config.alwaysActive = (BOOL)Config::Get(CONFIG_WRAPPER, "AlwaysActive", FALSE);
				config.coldCPU = (BOOL)Config::Get(CONFIG_WRAPPER, "ColdCPU", FALSE);

				if (!config.version)
				{
					config.wide.allowed = (BOOL)Config::Get(CONFIG_WRAPPER, "WideBattle", FALSE);
					config.battle.mirror = (BOOL)Config::Get(CONFIG_WRAPPER, "MirrorBattle", TRUE);
				}

				value = Config::Get(CONFIG_WRAPPER, "ScreenshotType", ImagePNG);
				config.snapshot.type = *(ImageType*)&value;
				if (config.snapshot.type < ImageBMP || config.snapshot.type > ImagePNG)
					config.snapshot.type = ImagePNG;

				value = Config::Get(CONFIG_WRAPPER, "ScreenshotLevel", 9);
				config.snapshot.level = *(DWORD*)&value;
				if (config.snapshot.level < 1 || config.snapshot.level > 9)
					config.snapshot.level = 9;

				value = Config::Get(CONFIG_WRAPPER, "ZoomFactor", 100);
				config.zoom.value = *(DWORD*)&value;
				if (!config.zoom.value || config.zoom.value > 100)
					config.zoom.value = 100;

				config.locales.current.id = (LCID)Config::Get(CONFIG_WRAPPER, "Locale", (INT)GetUserDefaultLCID());

				value = Config::Get(CONFIG_WRAPPER, "MessageTimeout", 15);
				if (value < 1)
					value = 1;

				config.msgTimeScale.time = *(DWORD*)&value;
				config.msgTimeScale.value = (FLOAT)MSG_TIMEOUT / config.msgTimeScale.time;

				config.updateMode = (UpdateMode)Config::Get(CONFIG_WRAPPER, "UpdateMode", (INT)UpdateSSE);
				if (config.updateMode < UpdateNone || config.updateMode > UpdateASM)
					config.updateMode = UpdateSSE;

				if (!config.isEditor)
					config.ai.fast = (BOOL)Config::Get(CONFIG_WRAPPER, "FastAI", FALSE);

				value = Config::Get(CONFIG_WRAPPER, "MouseScroll", 3);
				if (value < 0 || value > 3)
					value = 3;

				config.mouseScroll.lButton = value & 1;
				config.mouseScroll.mButton = value & 2;

				value = Config::Get(CONFIG_WRAPPER, "CloudsFactor", 1);
				if (value < 1)
					value = 1;
				config.cloudsFactor = (FLOAT)value;
			}

			{
				// F1 - reserved for "About"
				CHAR buffer[20];
				if (Config::Get(CONFIG_KEYS, "ImageFilter", "", buffer, sizeof(buffer)))
				{
					INT value = Config::Get(CONFIG_KEYS, "ImageFilter", 0);
					config.keys.imageFilter = LOBYTE(value);
					if (config.keys.imageFilter > 1 && config.keys.imageFilter > 24)
						config.keys.imageFilter = 0;
				}

				if (Config::Get(CONFIG_KEYS, "WindowedMode", "", buffer, sizeof(buffer)))
				{
					INT value = Config::Get(CONFIG_KEYS, "WindowedMode", 0);
					config.keys.windowedMode = LOBYTE(value);
					if (config.keys.windowedMode > 1 && config.keys.windowedMode > 24)
						config.keys.windowedMode = 0;
				}

				if (Config::Get(CONFIG_KEYS, "AspectRatio", "", buffer, sizeof(buffer)))
				{
					INT value = Config::Get(CONFIG_KEYS, "AspectRatio", 0);
					config.keys.aspectRatio = LOBYTE(value);
					if (config.keys.aspectRatio > 1 && config.keys.aspectRatio > 24)
						config.keys.aspectRatio = 0;
				}

				if (Config::Get(CONFIG_KEYS, "VSync", "", buffer, sizeof(buffer)))
				{
					INT value = Config::Get(CONFIG_KEYS, "VSync", 0);
					config.keys.vSync = LOBYTE(value);
					if (config.keys.vSync > 1 && config.keys.vSync > 24)
						config.keys.vSync = 0;
				}

				if (!config.version)
				{
					if (Config::Get(CONFIG_KEYS, "ZoomImage", "", buffer, sizeof(buffer)))
					{
						INT value = Config::Get(CONFIG_KEYS, "ZoomImage", 0);
						config.keys.zoomImage = LOBYTE(value);
						if (config.keys.zoomImage > 1 && config.keys.zoomImage > 24)
							config.keys.zoomImage = 0;
					}
				}

				if (Config::Get(CONFIG_KEYS, "SpeedToggle", "", buffer, sizeof(buffer)))
				{
					INT value = Config::Get(CONFIG_KEYS, "SpeedToggle", 0);
					config.keys.speedToggle = LOBYTE(value);
					if (config.keys.speedToggle > 1 && config.keys.speedToggle > 24)
						config.keys.speedToggle = 0;
				}

				if (Config::Get(CONFIG_KEYS, "Screenshot", "", buffer, sizeof(buffer)))
				{
					INT value = Config::Get(CONFIG_KEYS, "Screenshot", 0);
					config.keys.snapshot = LOBYTE(value);
					if (config.keys.snapshot > 1 && config.keys.snapshot > 24)
						config.keys.snapshot = 0;
				}
			}

			{
				INT value = Config::Get(CONFIG_COLORS, "HueSat", 0x01F401F4);
				config.colors.active.hueShift = 0.001f * min(1000, max(0, LOWORD(value)));
				config.colors.active.saturation = 0.001f * min(1000, max(0, HIWORD(value)));

				value = Config::Get(CONFIG_COLORS, "RgbInput", 0x03E80000);
				if (LOWORD(value) < HIWORD(value))
				{
					config.colors.active.input.left.rgb = 0.001f * min(1000, max(0, LOWORD(value)));
					config.colors.active.input.right.rgb = 0.001f * min(1000, max(0, HIWORD(value)));
				}
				else
				{
					config.colors.active.input.left.rgb = 0.0f;
					config.colors.active.input.right.rgb = 1.0f;
				}

				value = Config::Get(CONFIG_COLORS, "RedInput", 0x03E80000);
				if (LOWORD(value) < HIWORD(value))
				{
					config.colors.active.input.left.red = 0.001f * min(1000, max(0, LOWORD(value)));
					config.colors.active.input.right.red = 0.001f * min(1000, max(0, HIWORD(value)));
				}
				else
				{
					config.colors.active.input.left.red = 0.0f;
					config.colors.active.input.right.red = 1.0f;
				}

				value = Config::Get(CONFIG_COLORS, "GreenInput", 0x03E80000);
				if (LOWORD(value) < HIWORD(value))
				{
					config.colors.active.input.left.green = 0.001f * min(1000, max(0, LOWORD(value)));
					config.colors.active.input.right.green = 0.001f * min(1000, max(0, HIWORD(value)));
				}
				else
				{
					config.colors.active.input.left.green = 0.0f;
					config.colors.active.input.right.green = 1.0f;
				}

				value = Config::Get(CONFIG_COLORS, "BlueInput", 0x03E80000);
				if (LOWORD(value) < HIWORD(value))
				{
					config.colors.active.input.left.blue = 0.001f * min(1000, max(0, LOWORD(value)));
					config.colors.active.input.right.blue = 0.001f * min(1000, max(0, HIWORD(value)));
				}
				else
				{
					config.colors.active.input.left.blue = 0.0f;
					config.colors.active.input.right.blue = 1.0f;
				}

				config.colors.active.gamma.rgb = 0.001f * min(1000, max(0, Config::Get(CONFIG_COLORS, "RgbGamma", 500)));
				config.colors.active.gamma.red = 0.001f * min(1000, max(0, Config::Get(CONFIG_COLORS, "RedGamma", 500)));
				config.colors.active.gamma.green = 0.001f * min(1000, max(0, Config::Get(CONFIG_COLORS, "GreenGamma", 500)));
				config.colors.active.gamma.blue = 0.001f * min(1000, max(0, Config::Get(CONFIG_COLORS, "BlueGamma", 500)));

				value = Config::Get(CONFIG_COLORS, "RgbOutput", 0x03E80000);
				if (LOWORD(value) < HIWORD(value))
				{
					config.colors.active.output.left.rgb = 0.001f * min(1000, max(0, LOWORD(value)));
					config.colors.active.output.right.rgb = 0.001f * min(1000, max(0, HIWORD(value)));
				}
				else
				{
					config.colors.active.output.left.rgb = 0.0f;
					config.colors.active.output.right.rgb = 1.0f;
				}

				value = Config::Get(CONFIG_COLORS, "RedOutput", 0x03E80000);
				if (LOWORD(value) < HIWORD(value))
				{
					config.colors.active.output.left.red = 0.001f * min(1000, max(0, LOWORD(value)));
					config.colors.active.output.right.red = 0.001f * min(1000, max(0, HIWORD(value)));
				}
				else
				{
					config.colors.active.output.left.red = 0.0f;
					config.colors.active.output.right.red = 1.0f;
				}

				value = Config::Get(CONFIG_COLORS, "GreenOutput", 0x03E80000);
				if (LOWORD(value) < HIWORD(value))
				{
					config.colors.active.output.left.green = 0.001f * min(1000, max(0, LOWORD(value)));
					config.colors.active.output.right.green = 0.001f * min(1000, max(0, HIWORD(value)));
				}
				else
				{
					config.colors.active.output.left.green = 0.0f;
					config.colors.active.output.right.green = 1.0f;
				}

				value = Config::Get(CONFIG_COLORS, "BlueOutput", 0x03E80000);
				if (LOWORD(value) < HIWORD(value))
				{
					config.colors.active.output.left.blue = 0.001f * min(1000, max(0, LOWORD(value)));
					config.colors.active.output.right.blue = 0.001f * min(1000, max(0, HIWORD(value)));
				}
				else
				{
					config.colors.active.output.left.blue = 0.0f;
					config.colors.active.output.right.blue = 1.0f;
				}
			}
		}

		config.colors.current = &config.colors.active;

		config.menu = LoadMenu(hDllModule, MAKEINTRESOURCE(IDR_MENU));
		config.font = (HFONT)CreateFont(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
			OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE, TEXT("MS Shell Dlg"));
		config.msgSnapshot = RegisterWindowMessage(WM_SNAPSHOT);
		config.msgMenu = RegisterWindowMessage(WM_CHECK_MENU);

		if (config.version)
			config.mode = modesList;
		else
		{
			Config::Set(CONFIG_DISCIPLE, "ForceD3DPow2", FALSE);

			DWORD modeIdx = Config::Get(CONFIG_DISCIPLE, "DisplaySize", 0);
			if (modeIdx > 2)
				modeIdx = 0;
			config.mode = &modesList[modeIdx + 1];
		}

		config.resolution.width = LOWORD(config.mode->width);
		config.resolution.height = LOWORD(config.mode->height);

		Config::CalcZoomed();

		if (config.renderer != RendererGDI)
		{
			HMODULE hLibrary = LoadLibrary("NTDLL.dll");
			if (hLibrary)
			{
				if (GetProcAddress(hLibrary, "wine_get_version"))
					config.singleWindow = TRUE;
				FreeLibrary(hLibrary);
			}
		}
		else
			config.singleWindow = TRUE;

		if (config.updateMode == UpdateSSE)
		{
			INT cpuinfo[4];
			__cpuid(cpuinfo, 1);
			BOOL isSSE2 = cpuinfo[3] & (1 << 26) || FALSE;
			if (!isSSE2)
				config.updateMode = UpdateCPP;
		}

		config.singleThread = TRUE;
		DWORD processMask, systemMask;
		HANDLE process = GetCurrentProcess();
		if (GetProcessAffinityMask(process, &processMask, &systemMask))
		{
			if (processMask != systemMask && SetProcessAffinityMask(process, systemMask))
				GetProcessAffinityMask(process, &processMask, &systemMask);

			BOOL isSingle = FALSE;
			DWORD count = sizeof(DWORD) << 3;
			do
			{
				if (processMask & 1)
				{
					if (isSingle)
					{
						config.singleThread = FALSE;
						break;
					}
					else
						isSingle = TRUE;
				}

				processMask >>= 1;
			} while (--count);
		}

		if (config.renderer == RendererGDI || config.singleThread)
		{
			config.coldCPU = FALSE;
			config.borderless.real = FALSE;
			config.borderless.mode = FALSE;
		}

		DEVMODE devMode;
		MemoryZero(&devMode, sizeof(DEVMODE));
		devMode.dmSize = sizeof(DEVMODE);
		config.syncStep = 1000.0 / (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode) && (devMode.dmFields & DM_DISPLAYFREQUENCY) ? devMode.dmDisplayFrequency : 60);

		CHAR buffer[256];
		MENUITEMINFO info;
		MemoryZero(&info, sizeof(MENUITEMINFO));
		info.cbSize = sizeof(MENUITEMINFO);
		info.fMask = MIIM_TYPE;
		info.fType = MFT_STRING;
		info.dwTypeData = buffer;

		if (EnumSystemLocales(EnumLocalesCount, LCID_INSTALLED))
		{
			MenuItemData mData;
			mData.childId = IDM_LOCALE_DEFAULT;
			if (Window::GetMenuByChildID(&mData))
			{
				CHAR title[4];
				*(DWORD*)title = NULL;

				HMENU* hMenu = config.locales.menus;
				for (DWORD i = 0; i < sizeof(config.locales.menus) / sizeof(HMENU); ++i, ++hMenu)
				{
					if (*hMenu)
					{
						*title = 'A' + LOBYTE(i);
						AppendMenu(mData.hMenu, MF_STRING | MF_POPUP, (UINT_PTR)*hMenu, title);
					}
				}
			}
		}
		else
			config.locales.count = 0;

		config.locales.list = (LCID*)MemoryAlloc((config.locales.count + 1) * sizeof(LCID));
		config.locales.list[0] = 0;

		if (config.locales.count)
			EnumSystemLocales(EnumLocales, LCID_INSTALLED);

		UpdateLocale();

		if (config.keys.windowedMode && (info.cch = sizeof(buffer), GetMenuItemInfo(config.menu, IDM_RES_FULL_SCREEN, FALSE, &info)))
		{
			StrPrint(buffer, "%sF%d", buffer, config.keys.windowedMode);
			SetMenuItemInfo(config.menu, IDM_RES_FULL_SCREEN, FALSE, &info);
		}

		if (config.keys.aspectRatio && (info.cch = sizeof(buffer), GetMenuItemInfo(config.menu, IDM_ASPECT_RATIO, FALSE, &info)))
		{
			StrPrint(buffer, "%sF%d", buffer, config.keys.aspectRatio);
			SetMenuItemInfo(config.menu, IDM_ASPECT_RATIO, FALSE, &info);
		}

		if (config.keys.vSync && (info.cch = sizeof(buffer), GetMenuItemInfo(config.menu, IDM_VSYNC, FALSE, &info)))
		{
			StrPrint(buffer, "%sF%d", buffer, config.keys.vSync);
			SetMenuItemInfo(config.menu, IDM_VSYNC, FALSE, &info);
		}

		if (config.keys.zoomImage)
		{
			MenuItemData mData;
			mData.childId = IDM_STRETCH_OFF;
			if (Window::GetMenuByChildID(&mData) && (info.cch = sizeof(buffer), GetMenuItemInfo(mData.hParent, mData.index, TRUE, &info)))
			{
				StrPrint(buffer, "%sF%d", buffer, config.keys.zoomImage);
				SetMenuItemInfo(mData.hParent, mData.index, TRUE, &info);
			}
		}

		MenuItemData mData;
		if (config.keys.imageFilter)
		{
			mData.childId = IDM_FILT_OFF;
			if (Window::GetMenuByChildID(&mData) && (info.cch = sizeof(buffer), GetMenuItemInfo(mData.hParent, mData.index, TRUE, &info)))
			{
				StrPrint(buffer, "%sF%d", buffer, config.keys.imageFilter);
				SetMenuItemInfo(mData.hParent, mData.index, TRUE, &info);
			}
		}

		if (config.keys.speedToggle)
		{
			mData.childId = IDM_SPEED_1_0;
			if (Window::GetMenuByChildID(&mData) && (info.cch = sizeof(buffer), GetMenuItemInfo(mData.hParent, mData.index, TRUE, &info)))
			{
				StrPrint(buffer, "%sF%d", buffer, config.keys.speedToggle);
				SetMenuItemInfo(mData.hParent, mData.index, TRUE, &info);
			}
		}

		return TRUE;
	}

	INT __fastcall Get(const CHAR* app, const CHAR* key, INT default)
	{
		return GetPrivateProfileInt(app, key, (INT) default, config.file);
	}

	DWORD __fastcall Get(const CHAR* app, const CHAR* key, CHAR* default, CHAR* returnString, DWORD nSize)
	{
		return GetPrivateProfileString(app, key, default, returnString, nSize, config.file);
	}

	BOOL __fastcall Set(const CHAR* app, const CHAR* key, INT value)
	{
		CHAR res[20];
		StrPrint(res, "%d", value);
		return WritePrivateProfileString(app, key, res, config.file);
	}

	BOOL __fastcall Set(const CHAR* app, const CHAR* key, CHAR* value)
	{
		return WritePrivateProfileString(app, key, value, config.file);
	}

	CHAR* __fastcall Add(CHAR* ptr, const CHAR* key, INT value, const CHAR* comments)
	{
		if (comments)
		{
			*(DWORD*)ptr = '\r\n; ';
			ptr += 4;
			StrCopy(ptr, comments);
			ptr += StrLength(comments) + 1;
		}

		{
			CHAR dst[64];
			StrPrint(dst, "%s=%d", key, value);
			StrCopy(ptr, dst);
			ptr += StrLength(dst) + 1;
		}

		return ptr;
	}

	BOOL __fastcall IsZoomed()
	{
		return config.zoom.glallow && config.zoom.enabled && config.borders.active && (!config.battle.active || !config.battle.wide || config.battle.zoomable);
	}

	VOID __fastcall CalcZoomed(Size* dst, Size* src, DWORD scale)
	{
		FLOAT k = (FLOAT)src->width / src->height;
		if (k >= 4.0f / 3.0f)
		{
			FLOAT width = GAME_HEIGHT_FLOAT * k;
			dst->width = src->width - DWORD(((FLOAT)src->width - width) * scale * 0.01);
			dst->height = src->height - DWORD((src->height - GAME_HEIGHT) * scale * 0.01);
		}
		else
		{
			FLOAT height = GAME_WIDTH_FLOAT / k;
			dst->width = src->width - DWORD((src->width - GAME_WIDTH) * scale * 0.01);
			dst->height = src->height - DWORD(((FLOAT)src->height - height) * scale * 0.01);
		}
	}

	VOID __fastcall CalcZoomed(SizeFloat* dst, SizeFloat* src, DWORD scale)
	{
		FLOAT k = src->width / src->height;
		if (k >= 4.0f / 3.0f)
		{
			FLOAT width = GAME_HEIGHT_FLOAT * k;
			dst->width = src->width - FLOAT((src->width - width) * scale * 0.01);
			dst->height = src->height - FLOAT((src->height - GAME_HEIGHT) * scale * 0.01);
		}
		else
		{
			FLOAT height = GAME_WIDTH_FLOAT / k;
			dst->width = src->width - FLOAT((src->width - GAME_WIDTH) * scale * 0.01);
			dst->height = src->height - FLOAT((src->height - height) * scale * 0.01);
		}
	}

	VOID __fastcall CalcZoomed()
	{
		CalcZoomed(&config.zoom.size, (Size*)config.mode, config.zoom.value);

		SizeFloat size = { (FLOAT)config.mode->width, (FLOAT)config.mode->height };
		CalcZoomed(&config.zoom.sizeFloat, &size, config.zoom.value);
	}

	VOID __fastcall UpdateLocale()
	{
		if (config.locales.current.id)
		{
			CHAR cp[16];
			GetLocaleInfo(config.locales.current.id, LOCALE_IDEFAULTCODEPAGE, cp, sizeof(cp));
			config.locales.current.oem = StrToInt(cp);

			GetLocaleInfo(config.locales.current.id, LOCALE_IDEFAULTANSICODEPAGE, cp, sizeof(cp));
			config.locales.current.ansi = StrToInt(cp);

			CHAR name[64];
			GetLocaleInfo(config.locales.current.id, LOCALE_SENGLISHLANGUAGENAME, name, sizeof(name));

			CHAR country[64];
			GetLocaleInfo(config.locales.current.id, LOCALE_SENGLISHCOUNTRYNAME, country, sizeof(country));

			CHAR crtName[256];
			StrPrint(crtName, "%s_%s.%d", name, country, config.locales.current.ansi);

			SetLocale(LC_CTYPE, crtName);
		}
		else
			SetLocale(LC_CTYPE, "C");
	}
}