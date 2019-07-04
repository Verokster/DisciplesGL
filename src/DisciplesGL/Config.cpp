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

#include "stdafx.h"
#include "Config.h"
#include "Resource.h"

ConfigItems config;

DisplayMode modesList[] = {
	640, 480, 8,
	GAME_WIDTH, GAME_HEIGHT, 16,
	1024, 768, 16,
	1280, 1024, 16
};

const Resolution resolutionsList[] = {
	640, 480,
	GAME_WIDTH, GAME_HEIGHT,
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

const BYTE speedList[] = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 };

namespace Config
{
	BOOL __fastcall GetMenuByChildID(HMENU hParent, INT index, MenuItemData* mData)
	{
		HMENU hMenu = GetSubMenu(hParent, index);

		INT count = GetMenuItemCount(hMenu);
		for (INT i = 0; i < count; ++i)
		{
			UINT id = GetMenuItemID(hMenu, i);
			if (id == mData->childId)
			{
				mData->hParent = hParent;
				mData->index = index;

				return TRUE;
			}
			else if (GetMenuByChildID(hMenu, i, mData))
				return TRUE;
		}

		return FALSE;
	}

	BOOL __fastcall GetMenuByChildID(HMENU hMenu, MenuItemData* mData)
	{
		INT count = GetMenuItemCount(hMenu);
		for (INT i = 0; i < count; ++i)
		{
			if (GetMenuByChildID(hMenu, i, mData))
				return TRUE;
		}

		return FALSE;
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
				{
					config.version = TRUE;
					break;
				}
			}
		}

		config.isExist = !(BOOL)Config::Get(CONFIG_WRAPPER, "ReInit", TRUE);
		Config::Set(CONFIG_WRAPPER, "ReInit", FALSE);

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

			if (!config.version)
			{
				config.hd = 1;
				Config::Set(CONFIG_WRAPPER, "HD", config.hd);
			}

			config.image.aspect = TRUE;
			Config::Set(CONFIG_WRAPPER, "ImageAspect", config.image.aspect);

			config.image.vSync = TRUE;
			Config::Set(CONFIG_WRAPPER, "ImageVSync", config.image.vSync);

			config.image.filter = FilterCubic;
			Config::Set(CONFIG_WRAPPER, "ImageFilter", *(INT*)&config.image.filter);

			config.image.scaleNx.value = 2;
			Config::Set(CONFIG_WRAPPER, "ImageScaleNx", *(INT*)&config.image.scaleNx);

			config.image.xSal.value = 2;
			Config::Set(CONFIG_WRAPPER, "ImageXSal", *(INT*)&config.image.xSal);

			config.image.eagle.value = 2;
			Config::Set(CONFIG_WRAPPER, "ImageEagle", *(INT*)&config.image.eagle);

			config.image.scaleHQ.value = 2;
			Config::Set(CONFIG_WRAPPER, "ImageScaleHQ", *(INT*)&config.image.scaleHQ);

			config.image.xBRz.value = 2;
			Config::Set(CONFIG_WRAPPER, "ImageXBRZ", *(INT*)&config.image.xBRz);

			if (!config.version)
			{
				config.showBackBorder = TRUE;
				Config::Set(CONFIG_DISCIPLE, "ShowInterfBorder", config.showBackBorder);
			}

			config.keys.fpsCounter = 2;
			Config::Set(CONFIG_KEYS, "FpsCounter", config.keys.fpsCounter);

			config.keys.imageFilter = 3;
			Config::Set(CONFIG_KEYS, "ImageFilter", config.keys.imageFilter);

			config.keys.windowedMode = 4;
			Config::Set(CONFIG_KEYS, "WindowedMode", config.keys.windowedMode);

			Config::Set(CONFIG_KEYS, "AspectRatio", "");
			Config::Set(CONFIG_KEYS, "VSync", "");

			if (!config.version)
			{
				Config::Set(CONFIG_KEYS, "ShowBorders", "");
				Config::Set(CONFIG_KEYS, "ZoomImage", "");
			}

			config.keys.speedToggle = 5;
			Config::Set(CONFIG_KEYS, "SpeedToggle", config.keys.speedToggle);

			Config::Set(CONFIG_WRAPPER, "BorderlessMode", config.borderlessMode);

			config.speed.index = 5;
			config.speed.value = 0.1f * speedList[config.speed.index];
			Config::Set(CONFIG_WRAPPER, "GameSpeed", config.speed.index);
			Config::Set(CONFIG_WRAPPER, "SpeedEnabled", config.speed.enabled);

			Config::Set(CONFIG_WRAPPER, "AlwaysActive", config.alwaysActive);
			Config::Set(CONFIG_WRAPPER, "ColdCPU", config.coldCPU);
		}
		else
		{
			if (config.version && !Config::Get(CONFIG_DISCIPLE, "DDraw", TRUE) || !config.version && Config::Get(CONFIG_DISCIPLE, "UseD3D", FALSE))
				return FALSE;

			config.windowedMode = (BOOL)Config::Get(CONFIG_DISCIPLE, config.version ? "InWindow" : "DisplayMode", TRUE);

			if (!config.version)
				config.hd = Config::Get(CONFIG_WRAPPER, "HD", TRUE);

			config.image.aspect = (BOOL)Config::Get(CONFIG_WRAPPER, "ImageAspect", TRUE);
			config.image.vSync = (BOOL)Config::Get(CONFIG_WRAPPER, "ImageVSync", TRUE);

			INT value = Config::Get(CONFIG_WRAPPER, "ImageFilter", FilterCubic);
			config.image.filter = *(ImageFilter*)&value;
			if (config.image.filter < FilterNearest || config.image.filter > FilterScaleNx)
				config.image.filter = FilterCubic;

			value = Config::Get(CONFIG_WRAPPER, "ImageScaleNx", 2);
			config.image.scaleNx = *(FilterType*)&value;
			if (config.image.scaleNx.value != 2 && config.image.scaleNx.value != 3)
				config.image.scaleNx.value = 2;
			if (config.image.scaleNx.type & 0xFE)
				config.image.scaleNx.type = 0;

			value = Config::Get(CONFIG_WRAPPER, "ImageXSal", 2);
			config.image.xSal = *(FilterType*)&value;
			if (config.image.xSal.value != 2)
				config.image.xSal.value = 2;
			if (config.image.xSal.type & 0xFE)
				config.image.xSal.type = 0;

			value = Config::Get(CONFIG_WRAPPER, "ImageEagle", 2);
			config.image.eagle = *(FilterType*)&value;
			if (config.image.eagle.value != 2)
				config.image.eagle.value = 2;
			if (config.image.eagle.type & 0xFE)
				config.image.eagle.type = 0;

			value = Config::Get(CONFIG_WRAPPER, "ImageScaleHQ", 2);
			config.image.scaleHQ = *(FilterType*)&value;
			if (config.image.scaleHQ.value != 2 && config.image.scaleHQ.value != 4)
				config.image.scaleHQ.value = 2;
			if (config.image.scaleHQ.type & 0xFE)
				config.image.scaleHQ.type = 0;

			value = Config::Get(CONFIG_WRAPPER, "ImageXBRZ", 2);
			config.image.xBRz = *(FilterType*)&value;
			if (config.image.xBRz.value < 2 || config.image.xBRz.value > 6)
				config.image.xBRz.value = 6;
			if (config.image.xBRz.type & 0xFE)
				config.image.xBRz.type = 0;

			if (!config.version)
				config.showBackBorder = Config::Get(CONFIG_DISCIPLE, "ShowInterfBorder", TRUE);

			// F1 - reserved for "About"
			CHAR buffer[20];
			if (Config::Get(CONFIG_KEYS, "FpsCounter", "", buffer, sizeof(buffer)))
			{
				value = Config::Get(CONFIG_KEYS, "FpsCounter", 0);
				config.keys.fpsCounter = LOBYTE(value);
				if (config.keys.fpsCounter > 1 && config.keys.fpsCounter > 24)
					config.keys.fpsCounter = 0;
			}

			if (Config::Get(CONFIG_KEYS, "ImageFilter", "", buffer, sizeof(buffer)))
			{
				value = Config::Get(CONFIG_KEYS, "ImageFilter", 0);
				config.keys.imageFilter = LOBYTE(value);
				if (config.keys.imageFilter > 1 && config.keys.imageFilter > 24)
					config.keys.imageFilter = 0;
			}

			if (Config::Get(CONFIG_KEYS, "WindowedMode", "", buffer, sizeof(buffer)))
			{
				value = Config::Get(CONFIG_KEYS, "WindowedMode", 0);
				config.keys.windowedMode = LOBYTE(value);
				if (config.keys.windowedMode > 1 && config.keys.windowedMode > 24)
					config.keys.windowedMode = 0;
			}

			if (Config::Get(CONFIG_KEYS, "AspectRatio", "", buffer, sizeof(buffer)))
			{
				value = Config::Get(CONFIG_KEYS, "AspectRatio", 0);
				config.keys.aspectRatio = LOBYTE(value);
				if (config.keys.aspectRatio > 1 && config.keys.aspectRatio > 24)
					config.keys.aspectRatio = 0;
			}

			if (Config::Get(CONFIG_KEYS, "VSync", "", buffer, sizeof(buffer)))
			{
				value = Config::Get(CONFIG_KEYS, "VSync", 0);
				config.keys.vSync = LOBYTE(value);
				if (config.keys.vSync > 1 && config.keys.vSync > 24)
					config.keys.vSync = 0;
			}

			if (!config.version)
			{
				if (Config::Get(CONFIG_KEYS, "ShowBorders", "", buffer, sizeof(buffer)))
				{
					value = Config::Get(CONFIG_KEYS, "ShowBorders", 0);
					config.keys.showBorders = LOBYTE(value);
					if (config.keys.showBorders > 1 && config.keys.showBorders > 24)
						config.keys.showBorders = 0;
				}

				if (Config::Get(CONFIG_KEYS, "ZoomImage", "", buffer, sizeof(buffer)))
				{
					value = Config::Get(CONFIG_KEYS, "ZoomImage", 0);
					config.keys.zoomImage = LOBYTE(value);
					if (config.keys.zoomImage > 1 && config.keys.zoomImage > 24)
						config.keys.zoomImage = 0;
				}
			}

			if (Config::Get(CONFIG_KEYS, "SpeedToggle", "", buffer, sizeof(buffer)))
			{
				value = Config::Get(CONFIG_KEYS, "SpeedToggle", 0);
				config.keys.speedToggle = LOBYTE(value);
				if (config.keys.speedToggle > 1 && config.keys.speedToggle > 24)
					config.keys.speedToggle = 0;
			}

			config.borderlessMode = (BOOL)Config::Get(CONFIG_WRAPPER, "BorderlessMode", FALSE);

			value = Config::Get(CONFIG_WRAPPER, "GameSpeed", 5);
			config.speed.index = *(DWORD*)&value;
			if (config.speed.index >= sizeof(speedList) / sizeof(BYTE))
				config.speed.index = 5;
			config.speed.value = 0.1f * speedList[config.speed.index];

			config.speed.enabled = (BOOL)Config::Get(CONFIG_WRAPPER, "SpeedEnabled", FALSE);

			config.alwaysActive = (BOOL)Config::Get(CONFIG_WRAPPER, "AlwaysActive", FALSE);
			config.coldCPU = (BOOL)Config::Get(CONFIG_WRAPPER, "ColdCPU", FALSE);
		}

		config.menu = LoadMenu(hDllModule, MAKEINTRESOURCE(LOBYTE(GetVersion()) > 4 ? IDR_MENU : IDR_MENU_OLD));
		config.font = (HFONT)CreateFont(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
			OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE, TEXT("MS Shell Dlg"));

		if (config.version)
			config.mode = modesList;
		else
		{
			DWORD modeIdx = Config::Get(CONFIG_DISCIPLE, "DisplaySize", 0);
			if (modeIdx > 2)
				modeIdx = 0;
			config.mode = &modesList[modeIdx + 1];
		}

		config.resolution.width = LOWORD(config.mode->width);
		config.resolution.height = LOWORD(config.mode->height);

		CHAR buffer[256];
		MENUITEMINFO info;
		MemoryZero(&info, sizeof(MENUITEMINFO));
		info.cbSize = sizeof(MENUITEMINFO);
		info.fMask = MIIM_TYPE;
		info.fType = MFT_STRING;
		info.dwTypeData = buffer;

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

		if (config.keys.showBorders && (info.cch = sizeof(buffer), GetMenuItemInfo(config.menu, IDM_RES_BORDERS, FALSE, &info)))
		{
			StrPrint(buffer, "%sF%d", buffer, config.keys.showBorders);
			SetMenuItemInfo(config.menu, IDM_RES_BORDERS, FALSE, &info);
		}

		if (config.keys.zoomImage && (info.cch = sizeof(buffer), GetMenuItemInfo(config.menu, IDM_RES_STRETCH, FALSE, &info)))
		{
			StrPrint(buffer, "%sF%d", buffer, config.keys.zoomImage);
			SetMenuItemInfo(config.menu, IDM_RES_STRETCH, FALSE, &info);
		}

		MenuItemData mData;
		if (config.keys.imageFilter)
		{
			mData.childId = IDM_FILT_OFF;
			if (GetMenuByChildID(config.menu, &mData) && (info.cch = sizeof(buffer), GetMenuItemInfo(mData.hParent, mData.index, TRUE, &info)))
			{
				StrPrint(buffer, "%sF%d", buffer, config.keys.imageFilter);
				SetMenuItemInfo(mData.hParent, mData.index, TRUE, &info);
			}
		}

		if (config.keys.speedToggle)
		{
			mData.childId = IDM_SPEED_1_0;
			if (GetMenuByChildID(config.menu, &mData) && (info.cch = sizeof(buffer), GetMenuItemInfo(mData.hParent, mData.index, TRUE, &info)))
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
}