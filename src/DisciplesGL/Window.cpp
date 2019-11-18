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
#include "timeapi.h"
#include "Window.h"
#include "CommCtrl.h"
#include "Windowsx.h"
#include "Shellapi.h"
#include "Resource.h"
#include "Main.h"
#include "Config.h"
#include "Hooks.h"
#include "FpsCounter.h"
#include "OpenDraw.h"
#include "PngLib.h"

namespace Window
{
	HHOOK OldKeysHook;
	WNDPROC OldWindowProc, OldPanelProc;

	BOOL __fastcall GetMenuByChildID(HMENU hParent, MenuItemData* mData, INT index)
	{
		HMENU hMenu = GetSubMenu(hParent, index);

		INT count = GetMenuItemCount(hMenu);
		for (INT i = 0; i < count; ++i)
		{
			UINT id = GetMenuItemID(hMenu, i);
			if (id == mData->childId)
			{
				mData->hParent = hParent;
				mData->hMenu = hMenu;
				mData->index = index;

				return TRUE;
			}
			else if (GetMenuByChildID(hMenu, mData, i))
				return TRUE;
		}

		return FALSE;
	}

	BOOL __fastcall GetMenuByChildID(MenuItemData* mData)
	{
		INT count = GetMenuItemCount(config.menu);
		for (INT i = 0; i < count; ++i)
			if (GetMenuByChildID(config.menu, mData, i))
				return TRUE;

		MemoryZero(mData, sizeof(MenuItemData));
		return FALSE;
	}

	VOID __fastcall CheckEnablePopup(MenuItemData* mData, DWORD flags)
	{
		if (GetMenuByChildID(mData))
		{
			EnableMenuItem(mData->hParent, mData->index, MF_BYPOSITION | flags);
			CheckMenuItem(mData->hParent, mData->index, MF_BYPOSITION | MF_UNCHECKED);

			UINT count = (UINT)GetMenuItemCount(mData->hMenu);
			for (UINT i = 0; i < count; ++i)
			{
				EnableMenuItem(mData->hMenu, i, MF_BYPOSITION | flags);
				CheckMenuItem(mData->hMenu, i, MF_BYPOSITION | MF_UNCHECKED);
			}
		}
	}

	VOID __fastcall CheckMenu(MenuType type)
	{
		switch (type)
		{
		case MenuLocale:
		{
			CheckMenuItem(config.menu, IDM_LOCALE_DEFAULT, MF_BYCOMMAND | (!config.locales.current.id ? MF_CHECKED : MF_UNCHECKED));

			MenuItemData mData;
			mData.childId = IDM_LOCALE_DEFAULT;
			if (GetMenuByChildID(&mData))
			{
				HMENU* hPopup = config.locales.menus;
				DWORD count = sizeof(config.locales.menus) / sizeof(HMENU);
				DWORD idx = 2;
				do
				{
					if (*hPopup)
					{
						BOOL checked = FALSE;
						INT count = GetMenuItemCount(*hPopup);
						for (INT i = 0; i < count; ++i)
						{
							UINT id = GetMenuItemID(*hPopup, i);
							LCID locale = config.locales.list[id - IDM_LOCALE_DEFAULT];

							BOOL check = locale == config.locales.current.id;
							CheckMenuItem(config.menu, id, MF_BYCOMMAND | (check ? MF_CHECKED : MF_UNCHECKED));

							if (check)
								checked = TRUE;
						}

						CheckMenuItem(mData.hMenu, idx, MF_BYPOSITION | (checked ? MF_CHECKED : MF_UNCHECKED));

						++idx;
					}

					++hPopup;
				} while (--count);
			}
		}
		break;

		case MenuAspect:
		{
			EnableMenuItem(config.menu, IDM_ASPECT_RATIO, MF_BYCOMMAND | (glVersion ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			CheckMenuItem(config.menu, IDM_ASPECT_RATIO, MF_BYCOMMAND | (glVersion && config.image.aspect ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuVSync:
		{
			EnableMenuItem(config.menu, IDM_VSYNC, MF_BYCOMMAND | (glVersion && WGLSwapInterval ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			CheckMenuItem(config.menu, IDM_VSYNC, MF_BYCOMMAND | (glVersion && WGLSwapInterval && config.image.vSync ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuInterpolate:
		{
			CheckMenuItem(config.menu, IDM_FILT_OFF, MF_BYCOMMAND | MF_UNCHECKED);

			EnableMenuItem(config.menu, IDM_FILT_LINEAR, MF_BYCOMMAND | (glVersion ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			CheckMenuItem(config.menu, IDM_FILT_LINEAR, MF_BYCOMMAND | MF_UNCHECKED);

			DWORD isFilters = glVersion >= GL_VER_2_0 ? MF_ENABLED : (MF_DISABLED | MF_GRAYED);

			EnableMenuItem(config.menu, IDM_FILT_HERMITE, MF_BYCOMMAND | isFilters);
			CheckMenuItem(config.menu, IDM_FILT_HERMITE, MF_BYCOMMAND | MF_UNCHECKED);

			EnableMenuItem(config.menu, IDM_FILT_CUBIC, MF_BYCOMMAND | isFilters);
			CheckMenuItem(config.menu, IDM_FILT_CUBIC, MF_BYCOMMAND | MF_UNCHECKED);

			switch (config.image.interpolation)
			{
			case InterpolateLinear:
				CheckMenuItem(config.menu, IDM_FILT_LINEAR, MF_BYCOMMAND | MF_CHECKED);
				break;

			case InterpolateHermite:
				CheckMenuItem(config.menu, isFilters == MF_ENABLED ? IDM_FILT_HERMITE : IDM_FILT_LINEAR, MF_BYCOMMAND | MF_CHECKED);
				break;

			case InterpolateCubic:
				CheckMenuItem(config.menu, isFilters == MF_ENABLED ? IDM_FILT_CUBIC : IDM_FILT_LINEAR, MF_BYCOMMAND | MF_CHECKED);
				break;

			default:
				CheckMenuItem(config.menu, IDM_FILT_OFF, MF_BYCOMMAND | MF_CHECKED);
				break;
			}
		}
		break;

		case MenuUpscale:
		{
			CheckMenuItem(config.menu, IDM_FILT_NONE, MF_BYCOMMAND | MF_UNCHECKED);

			DWORD isFilters = glVersion >= GL_VER_3_0 ? MF_ENABLED : (MF_DISABLED | MF_GRAYED);

			MenuItemData mScaleNx, mEagle, mXSal, mScaleHQ, mXBRZ;

			// ScaleNx
			mScaleNx.childId = IDM_FILT_SCALENX_2X;
			CheckEnablePopup(&mScaleNx, isFilters);

			// Eagle
			mEagle.childId = IDM_FILT_EAGLE_2X;
			CheckEnablePopup(&mEagle, isFilters);

			// XSal
			mXSal.childId = IDM_FILT_XSAL_2X;
			CheckEnablePopup(&mXSal, isFilters);

			// ScaleHQ
			mScaleHQ.childId = IDM_FILT_SCALEHQ_2X;
			CheckEnablePopup(&mScaleHQ, isFilters);

			// xBRz
			mXBRZ.childId = IDM_FILT_XRBZ_2X;
			CheckEnablePopup(&mXBRZ, isFilters);

			if (config.image.upscaling != UpscaleNone && isFilters == MF_ENABLED)
			{
				switch (config.image.upscaling)
				{
				case UpscaleScaleNx:
					if (mScaleNx.hParent)
						CheckMenuItem(mScaleNx.hParent, mScaleNx.index, MF_BYPOSITION | MF_CHECKED);

					switch (config.image.scaleNx)
					{
					case 3:
						CheckMenuItem(config.menu, IDM_FILT_SCALENX_3X, MF_BYCOMMAND | MF_CHECKED);
						break;

					default:
						CheckMenuItem(config.menu, IDM_FILT_SCALENX_2X, MF_BYCOMMAND | MF_CHECKED);
						break;
					}

					break;

				case UpscaleEagle:
					if (mEagle.hParent)
						CheckMenuItem(mEagle.hParent, mEagle.index, MF_BYPOSITION | MF_CHECKED);

					CheckMenuItem(config.menu, IDM_FILT_EAGLE_2X, MF_BYCOMMAND | MF_CHECKED);

					break;

				case UpscaleXSal:
					if (mXSal.hParent)
						CheckMenuItem(mXSal.hParent, mXSal.index, MF_BYPOSITION | MF_CHECKED);

					CheckMenuItem(config.menu, IDM_FILT_XSAL_2X, MF_BYCOMMAND | MF_CHECKED);

					break;

				case UpscaleScaleHQ:
					if (mScaleHQ.hParent)
						CheckMenuItem(mScaleHQ.hParent, mScaleHQ.index, MF_BYPOSITION | MF_CHECKED);

					switch (config.image.scaleHQ)
					{
					case 4:
						CheckMenuItem(config.menu, IDM_FILT_SCALEHQ_4X, MF_BYCOMMAND | MF_CHECKED);
						break;

					default:
						CheckMenuItem(config.menu, IDM_FILT_SCALEHQ_2X, MF_BYCOMMAND | MF_CHECKED);
						break;
					}

					break;

				case UpscaleXRBZ:
					if (mXBRZ.hParent)
						CheckMenuItem(mXBRZ.hParent, mXBRZ.index, MF_BYPOSITION | MF_CHECKED);

					switch (config.image.xBRz)
					{
					case 3:
						CheckMenuItem(config.menu, IDM_FILT_XRBZ_3X, MF_BYCOMMAND | MF_CHECKED);
						break;

					case 4:
						CheckMenuItem(config.menu, IDM_FILT_XRBZ_4X, MF_BYCOMMAND | MF_CHECKED);
						break;

					case 5:
						CheckMenuItem(config.menu, IDM_FILT_XRBZ_5X, MF_BYCOMMAND | MF_CHECKED);
						break;

					case 6:
						CheckMenuItem(config.menu, IDM_FILT_XRBZ_6X, MF_BYCOMMAND | MF_CHECKED);
						break;

					default:
						CheckMenuItem(config.menu, IDM_FILT_XRBZ_2X, MF_BYCOMMAND | MF_CHECKED);
						break;
					}

					break;

				default:
					break;
				}
			}
			else
				CheckMenuItem(config.menu, IDM_FILT_NONE, MF_BYCOMMAND | MF_CHECKED);
		}
		break;

		case MenuResolution:
		{
			DWORD count = sizeof(resolutionsList) / sizeof(Resolution);
			const Resolution* resItem = resolutionsList;
			DWORD id = IDM_RES_640_480;
			while (count--)
			{
				BOOL enabled;
				if (config.version)
				{
					if (!config.resHooked)
						enabled = id == IDM_RES_640_480;
					else
						enabled = resItem->width <= 2048 && resItem->height <= 1024;
				}
				else
				{
					if (!config.resHooked)
						enabled = id == IDM_RES_800_600 || id == IDM_RES_1024_768 || id == IDM_RES_1280_1024;
					else
						enabled = id != IDM_RES_640_480;
				}

				EnableMenuItem(config.menu, id, MF_BYCOMMAND | (enabled ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
				CheckMenuItem(config.menu, id, MF_BYCOMMAND | (resItem->width == config.resolution.width && resItem->height == config.resolution.height ? MF_CHECKED : MF_UNCHECKED));

				++resItem;
				++id;
			}
		}
		break;

		case MenuSpeed:
		{
			DWORD count = IDM_SPEED_3_0 - IDM_SPEED_1_0 + 1;
			DWORD id = IDM_SPEED_1_0;
			while (count--)
			{
				CheckMenuItem(config.menu, id, MF_BYCOMMAND | (config.speed.enabled && id - IDM_SPEED_1_0 == config.speed.index || !config.speed.enabled && id == IDM_SPEED_1_0 ? MF_CHECKED : MF_UNCHECKED));
				++id;
			}
		}
		break;

		case MenuBorders:
		{
			if (config.version)
			{
				EnableMenuItem(config.menu, IDM_RES_BORDERS, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
				CheckMenuItem(config.menu, IDM_RES_BORDERS, MF_BYCOMMAND | MF_UNCHECKED);
			}
			else if (!config.resHooked || !config.zoom.allowed)
			{
				EnableMenuItem(config.menu, IDM_RES_BORDERS, MF_BYCOMMAND | MF_ENABLED);
				CheckMenuItem(config.menu, IDM_RES_BORDERS, MF_BYCOMMAND | (config.border.enabled ? MF_CHECKED : MF_UNCHECKED));
			}
			else
			{
				EnableMenuItem(config.menu, IDM_RES_BORDERS, MF_BYCOMMAND | MF_ENABLED);
				CheckMenuItem(config.menu, IDM_RES_BORDERS, MF_BYCOMMAND | (config.border.enabled ? MF_CHECKED : MF_UNCHECKED));
			}
		}
		break;

		case MenuStretch:
		{
			MenuItemData mData;
			mData.childId = IDM_STRETCH_OFF;
			if (GetMenuByChildID(&mData))
			{
				if (config.version || !config.resHooked || !config.zoom.allowed)
				{
					EnableMenuItem(mData.hParent, mData.index, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
					CheckMenuItem(mData.hParent, mData.index, MF_BYPOSITION | MF_UNCHECKED);
				}
				else
				{
					EnableMenuItem(mData.hParent, mData.index, MF_BYPOSITION | MF_ENABLED);
					CheckMenuItem(mData.hParent, mData.index, MF_BYPOSITION | (config.zoom.menu ? MF_CHECKED : MF_UNCHECKED));

					UINT check = IDM_STRETCH_OFF + (config.zoom.menu ? config.zoom.value : 0);
					UINT count = (UINT)GetMenuItemCount(mData.hMenu);
					for (UINT i = 0; i < count; ++i)
					{
						UINT id = GetMenuItemID(mData.hMenu, i);
						CheckMenuItem(mData.hMenu, i, MF_BYPOSITION | (id == check ? MF_CHECKED : MF_UNCHECKED));
					}
				}
			}
		}
		break;

		case MenuWindowMode:
		{
			CheckMenuItem(config.menu, IDM_RES_FULL_SCREEN, MF_BYCOMMAND | (!config.windowedMode ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuWindowType:
		{
			CheckMenuItem(config.menu, IDM_MODE_BORDERLESS, MF_BYCOMMAND | (config.borderless.mode ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(config.menu, IDM_MODE_EXCLUSIVE, MF_BYCOMMAND | (!config.borderless.mode ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuFastAI:
		{
			CheckMenuItem(config.menu, IDM_FAST_AI, MF_BYCOMMAND | (config.ai.fast ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuActive:
		{
			CheckMenuItem(config.menu, IDM_ALWAYS_ACTIVE, MF_BYCOMMAND | (config.alwaysActive ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuCpu:
		{
			CheckMenuItem(config.menu, IDM_PATCH_CPU, MF_BYCOMMAND | (config.coldCPU ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuBattle:
		{
			EnableMenuItem(config.menu, IDM_MODE_WIDEBATTLE, MF_BYCOMMAND | (config.wide.hooked ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			CheckMenuItem(config.menu, IDM_MODE_WIDEBATTLE, MF_BYCOMMAND | (config.wide.allowed ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuSnapshotType:
		{
			EnableMenuItem(config.menu, IDM_SCR_PNG, MF_BYCOMMAND | (pnglib_create_write_struct ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			BOOL pngChecked = config.snapshot.type == ImagePNG && pnglib_create_write_struct;
			CheckMenuItem(config.menu, IDM_SCR_BMP, MF_BYCOMMAND | (!pngChecked ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(config.menu, IDM_SCR_PNG, MF_BYCOMMAND | (pngChecked ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuSnapshotLevel:
		{
			if (!pnglib_create_write_struct)
			{
				MenuItemData mData;
				mData.childId = IDM_SCR_1;
				if (GetMenuByChildID(&mData))
					EnableMenuItem(mData.hParent, mData.index, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
			}

			DWORD count = IDM_SCR_9 - IDM_SCR_1 + 1;
			DWORD id = IDM_SCR_1;
			while (count--)
			{
				CheckMenuItem(config.menu, id, MF_BYCOMMAND | (pnglib_create_write_struct && id - IDM_SCR_PNG == config.snapshot.level ? MF_CHECKED : MF_UNCHECKED));
				++id;
			}
		}
		break;

		case MenuMsgTimeScale:
		{
			MenuItemData mData;
			mData.childId = IDM_MSG_5;
			if (GetMenuByChildID(&mData))
			{
				if (config.msgTimeScale.hooked)
				{
					EnableMenuItem(mData.hParent, mData.index, MF_BYPOSITION | MF_ENABLED);

					UINT count = (UINT)GetMenuItemCount(mData.hMenu);
					for (UINT i = 0; i < count; ++i)
					{
						UINT id = GetMenuItemID(mData.hMenu, i);
						CheckMenuItem(mData.hMenu, i, MF_BYPOSITION | (id - IDM_MSG_0 == config.msgTimeScale.time ? MF_CHECKED : MF_UNCHECKED));
					}
				}
				else
					EnableMenuItem(mData.hParent, mData.index, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
			}
		}
		break;

		default:
			break;
		}
	}

	VOID __fastcall CheckMenu()
	{
		CheckMenu(MenuLocale);
		CheckMenu(MenuAspect);
		CheckMenu(MenuVSync);
		CheckMenu(MenuInterpolate);
		CheckMenu(MenuUpscale);
		CheckMenu(MenuResolution);
		CheckMenu(MenuSpeed);
		CheckMenu(MenuBorders);
		CheckMenu(MenuStretch);
		CheckMenu(MenuWindowMode);
		CheckMenu(MenuWindowType);
		CheckMenu(MenuFastAI);
		CheckMenu(MenuActive);
		CheckMenu(MenuCpu);
		CheckMenu(MenuBattle);
		CheckMenu(MenuSnapshotType);
		CheckMenu(MenuSnapshotLevel);
		CheckMenu(MenuMsgTimeScale);
	}

	VOID __fastcall FilterChanged(HWND hWnd, const CHAR* name, INT value)
	{
		Config::Set(CONFIG_WRAPPER, name, value);

		OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
		if (ddraw)
		{
			ddraw->LoadFilterState();
			SetEvent(ddraw->hDrawEvent);
		}
	}

	VOID __fastcall InterpolationChanged(HWND hWnd, InterpolationFilter filter)
	{
		config.image.interpolation = glVersion >= GL_VER_2_0 || filter < InterpolateHermite ? filter : InterpolateLinear;

		{
			CHAR format[32];
			LoadString(hDllModule, IDS_TEXT_INTERPOLATIN, format, sizeof(format));

			DWORD id;
			switch (config.image.interpolation)
			{
			case InterpolateLinear:
				id = IDS_TEXT_FILT_LINEAR;
				break;
			case InterpolateHermite:
				id = IDS_TEXT_FILT_HERMITE;
				break;
			case InterpolateCubic:
				id = IDS_TEXT_FILT_CUBIC;
				break;
			default:
				id = IDS_TEXT_FILT_OFF;
				break;
			}

			CHAR str[32];
			LoadString(hDllModule, id, str, sizeof(str));

			CHAR text[64];
			StrPrint(text, "%s: %s", format, str);
			Hooks::PrintText(text);
		}

		FilterChanged(hWnd, "Interpolation", *(INT*)&config.image.interpolation);
		CheckMenu(MenuInterpolate);
	}

	VOID __fastcall UpscalingChanged(HWND hWnd, UpscalingFilter filter)
	{
		config.image.upscaling = glVersion >= GL_VER_3_0 ? filter : UpscaleNone;

		{
			CHAR format[32];
			LoadString(hDllModule, IDS_TEXT_UPSCALING, format, sizeof(format));

			DWORD id, value;
			switch (config.image.upscaling)
			{
			case UpscaleScaleNx:
				id = IDS_TEXT_FILT_SCALENX;
				value = config.image.scaleNx;
				break;
			case UpscaleEagle:
				id = IDS_TEXT_FILT_EAGLE;
				value = config.image.eagle;
				break;
			case UpscaleXSal:
				id = IDS_TEXT_FILT_XSAL;
				value = config.image.xSal;
				break;
			case UpscaleScaleHQ:
				id = IDS_TEXT_FILT_SCALEHQ;
				value = config.image.scaleHQ;
				break;
			case UpscaleXRBZ:
				id = IDS_TEXT_FILT_XBRZ;
				value = config.image.xBRz;
				break;
			default:
				id = IDS_TEXT_FILT_NONE;
				break;
			}

			CHAR str[32];
			LoadString(hDllModule, id, str, sizeof(str));

			CHAR text[64];

			if (config.image.upscaling)
				StrPrint(text, "%s: %s x%d", format, str, value);
			else
				StrPrint(text, "%s: %s", format, str);

			Hooks::PrintText(text);
		}

		FilterChanged(hWnd, "Upscaling", *(INT*)&config.image.upscaling);
		CheckMenu(MenuUpscale);
	}

	VOID __fastcall SelectScaleNxMode(HWND hWnd, BYTE value)
	{
		config.image.scaleNx = value;
		Config::Set(CONFIG_WRAPPER, "ScaleNx", *(INT*)&config.image.scaleNx);
		UpscalingChanged(hWnd, UpscaleScaleNx);
	}

	VOID __fastcall SelectXSalMode(HWND hWnd, BYTE value)
	{
		config.image.xSal = value;
		Config::Set(CONFIG_WRAPPER, "XSal", *(INT*)&config.image.xSal);
		UpscalingChanged(hWnd, UpscaleXSal);
	}

	VOID __fastcall SelectEagleMode(HWND hWnd, BYTE value)
	{
		config.image.eagle = value;
		Config::Set(CONFIG_WRAPPER, "Eagle", *(INT*)&config.image.eagle);
		UpscalingChanged(hWnd, UpscaleEagle);
	}

	VOID __fastcall SelectScaleHQMode(HWND hWnd, BYTE value)
	{
		config.image.scaleHQ = value;
		Config::Set(CONFIG_WRAPPER, "ScaleHQ", *(INT*)&config.image.scaleHQ);
		UpscalingChanged(hWnd, UpscaleScaleHQ);
	}

	VOID __fastcall SelectXBRZMode(HWND hWnd, BYTE value)
	{
		config.image.xBRz = value;
		Config::Set(CONFIG_WRAPPER, "XBRZ", *(INT*)&config.image.xBRz);
		UpscalingChanged(hWnd, UpscaleXRBZ);
	}

	BOOL __stdcall EnumChildProc(HWND hDlg, LPARAM lParam)
	{
		if ((GetWindowLong(hDlg, GWL_STYLE) & SS_ICON) == SS_ICON)
			SendMessage(hDlg, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)config.icon);
		else
			SendMessage(hDlg, WM_SETFONT, (WPARAM)config.font, TRUE);

		return TRUE;
	}

	LRESULT __stdcall KeysHook(INT nCode, WPARAM wParam, LPARAM lParam)
	{
		if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN))
		{
			KBDLLHOOKSTRUCT* phs = (KBDLLHOOKSTRUCT*)lParam;
			if (phs->vkCode == VK_SNAPSHOT)
			{
				HWND hWnd = GetForegroundWindow();
				OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
				if (ddraw && !config.windowedMode)
				{
					ddraw->isTakeSnapshot = SnapshotClipboard;
					if (ddraw)
						SetEvent(ddraw->hDrawEvent);

					return TRUE;
				}
			}
		}

		return CallNextHookEx(OldKeysHook, nCode, wParam, lParam);
	}

	LRESULT __stdcall AboutApplicationProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_INITDIALOG:
		{
			SetWindowLong(hDlg, GWL_EXSTYLE, NULL);
			EnumChildWindows(hDlg, EnumChildProc, NULL);

			CHAR path[MAX_PATH];
			CHAR temp[100];

			GetModuleFileName(NULL, path, sizeof(path));

			DWORD hSize;
			DWORD verSize = GetFileVersionInfoSize(path, &hSize);

			if (verSize)
			{
				CHAR* verData = (CHAR*)MemoryAlloc(verSize);
				{
					if (GetFileVersionInfo(path, hSize, verSize, verData))
					{
						VOID* buffer;
						UINT size;
						if (VerQueryValue(verData, "\\", &buffer, &size) && size)
						{
							VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)buffer;

							GetDlgItemText(hDlg, IDC_VERSION, temp, sizeof(temp));
							StrPrint(path, temp, HIWORD(verInfo->dwProductVersionMS), LOWORD(verInfo->dwProductVersionMS), HIWORD(verInfo->dwProductVersionLS), LOWORD(verInfo->dwProductVersionLS));
							SetDlgItemText(hDlg, IDC_VERSION, path);
						}

						DWORD* lpTranslate;
						if (VerQueryValue(verData, "\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &size) && size)
						{
							BOOL isTitle = FALSE;
							BOOL isCopyright = FALSE;

							DWORD count = size / sizeof(UINT);
							{
								DWORD* translate = lpTranslate;
								while (count && (!isTitle || !isCopyright))
								{
									if (!isTitle && StrPrint(path, "\\StringFileInfo\\%04x%04x\\%s", LOWORD(*translate), HIWORD(*translate), "FileDescription") && VerQueryValue(verData, path, &buffer, &size) && size)
									{
										SetDlgItemText(hDlg, IDC_TITLE, (CHAR*)buffer);
										isTitle = TRUE;
									}

									if (!isCopyright && StrPrint(path, "\\StringFileInfo\\%04x%04x\\%s", LOWORD(*translate), HIWORD(*translate), "LegalCopyright") && VerQueryValue(verData, path, &buffer, &size) && size)
									{
										SetDlgItemText(hDlg, IDC_COPYRIGHT, (CHAR*)buffer);
										isCopyright = TRUE;
									}

									++translate;
									--count;
								}
							}
						}
					}
				}
				MemoryFree(verData);
			}

			break;
		}

		case WM_COMMAND:
		{
			if (wParam == IDC_BTN_OK)
				EndDialog(hDlg, TRUE);
			break;
		}

		default:
			break;
		}

		return DefWindowProc(hDlg, uMsg, wParam, lParam);
	}

	LRESULT __stdcall AboutWrapperProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_INITDIALOG:
		{
			SetWindowLong(hDlg, GWL_EXSTYLE, NULL);
			EnumChildWindows(hDlg, EnumChildProc, NULL);

			CHAR path[MAX_PATH];
			CHAR temp[100];

			GetModuleFileName(hDllModule, path, sizeof(path));

			DWORD hSize;
			DWORD verSize = GetFileVersionInfoSize(path, &hSize);

			if (verSize)
			{
				CHAR* verData = (CHAR*)MemoryAlloc(verSize);
				{
					if (GetFileVersionInfo(path, hSize, verSize, verData))
					{
						VOID* buffer;
						UINT size;
						if (VerQueryValue(verData, "\\", &buffer, &size) && size)
						{
							VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)buffer;

							GetDlgItemText(hDlg, IDC_VERSION, temp, sizeof(temp));
							StrPrint(path, temp, HIWORD(verInfo->dwProductVersionMS), LOWORD(verInfo->dwProductVersionMS), HIWORD(verInfo->dwProductVersionLS), LOWORD(verInfo->dwFileVersionLS));
							SetDlgItemText(hDlg, IDC_VERSION, path);
						}
					}
				}
				MemoryFree(verData);
			}

			if (GetDlgItemText(hDlg, IDC_LNK_EMAIL, temp, sizeof(temp)))
			{
				StrPrint(path, "<A HREF=\"mailto:%s\">%s</A>", temp, temp);
				SetDlgItemText(hDlg, IDC_LNK_EMAIL, path);
			}

			break;
		}

		case WM_NOTIFY:
		{
			if (((NMHDR*)lParam)->code == NM_CLICK && wParam == IDC_LNK_EMAIL)
			{
				SHELLEXECUTEINFOW shExecInfo;
				MemoryZero(&shExecInfo, sizeof(SHELLEXECUTEINFOW));
				shExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
				shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
				shExecInfo.lpFile = ((NMLINK*)lParam)->item.szUrl;
				shExecInfo.nShow = SW_SHOW;

				ShellExecuteExW(&shExecInfo);
			}

			break;
		}

		case WM_COMMAND:
		{
			if (wParam == IDC_BTN_OK)
				EndDialog(hDlg, TRUE);
			break;
		}

		default:
			break;
		}

		return DefWindowProc(hDlg, uMsg, wParam, lParam);
	}

	LRESULT __stdcall WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC hDc = BeginPaint(hWnd, &paint);
			if (hDc)
				EndPaint(hWnd, &paint);

			return NULL;
		}

		case WM_ERASEBKGND:
			return TRUE;

		case WM_WINDOWPOSCHANGED:
		{
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
			{
				RECT rect;
				GetClientRect(hWnd, &rect);
				ddraw->viewport.width = rect.right;
				ddraw->viewport.height = rect.bottom - ddraw->viewport.offset;
				ddraw->viewport.refresh = TRUE;
				SetEvent(ddraw->hDrawEvent);
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_MOVE:
		{
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
				SetEvent(ddraw->hDrawEvent);

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_SIZE:
		{
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
			{
				if (ddraw->hDraw && ddraw->hDraw != hWnd)
					SetWindowPos(ddraw->hDraw, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOREPOSITION | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);

				ddraw->viewport.width = LOWORD(lParam);
				ddraw->viewport.height = HIWORD(lParam) - ddraw->viewport.offset;
				ddraw->viewport.refresh = TRUE;
				SetEvent(ddraw->hDrawEvent);
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_GETMINMAXINFO:
		{
			DWORD style = GetWindowLong(hWnd, GWL_STYLE);

			RECT min = { 0, 0, MIN_WIDTH, MIN_HEIGHT };
			AdjustWindowRect(&min, style, config.windowedMode);

			RECT max = { 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
			if (!config.windowedMode && config.borderless.mode)
				max.bottom += BORDERLESS_OFFSET;
			AdjustWindowRect(&max, style, config.windowedMode);

			MINMAXINFO* mmi = (MINMAXINFO*)lParam;
			mmi->ptMinTrackSize.x = min.right - min.left;
			mmi->ptMinTrackSize.y = min.bottom - min.top;
			mmi->ptMaxTrackSize.x = max.right - max.left;
			mmi->ptMaxTrackSize.y = max.bottom - max.top;

			return NULL;
		}

		case WM_DESTROY:
		{
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
			{
				ddraw->RenderStop();
				ddraw->SetCooperativeLevel(NULL, NULL);
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_ACTIVATEAPP:
		{
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
			{
				ddraw->viewport.refresh = TRUE;

				if (!config.windowedMode && (!config.borderless.real || !config.alwaysActive))
				{
					if (!config.borderless.real)
					{
						ddraw->RenderStop();
						config.borderless.mode = !(BOOL)wParam;
						ddraw->RenderStart();
					}
					else
					{
						if (wParam)
							ddraw->RenderStart();
						else
							ddraw->RenderStop();
					}
				}
			}

			return !config.alwaysActive || (BOOL)wParam ? CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam) : NULL;
		}

		case WM_SETCURSOR:
		{
			if (LOWORD(lParam) == HTCLIENT)
			{
				OpenDraw* ddraw = Main::FindOpenDrawByWindow(GetForegroundWindow());
				if (ddraw)
				{
					if (!config.windowedMode || !config.image.aspect)
						SetCursor(NULL);
					else
					{
						POINT p;
						GetCursorPos(&p);
						ScreenToClient(hWnd, &p);

						if (p.x >= ddraw->viewport.rectangle.x && p.x < ddraw->viewport.rectangle.x + ddraw->viewport.rectangle.width && p.y >= ddraw->viewport.rectangle.y && p.y < ddraw->viewport.rectangle.y + ddraw->viewport.rectangle.height)
							SetCursor(NULL);
						else
							SetCursor(config.cursor);
					}
				}
				else
					SetCursor(config.cursor);

				return TRUE;
			}

			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			if (!(HIWORD(lParam) & KF_ALTDOWN))
			{
				if (wParam == VK_OEM_PLUS || wParam == VK_OEM_MINUS || wParam == VK_ADD || wParam == VK_SUBTRACT)
				{
					DWORD index = config.speed.enabled ? config.speed.index : 0;
					DWORD oldIndex = index;
					if (wParam == VK_ADD || wParam == VK_OEM_PLUS)
					{
						if (index != MAX_SPEED_INDEX - 1)
							++index;
					}
					else
					{
						if (index)
							--index;
					}

					if (index != oldIndex)
					{
						config.speed.index = index;
						config.speed.value = 0.1f * (index + 10);
						config.speed.enabled = TRUE;
						Config::Set(CONFIG_WRAPPER, "GameSpeed", *(INT*)&index);
						Config::Set(CONFIG_WRAPPER, "SpeedEnabled", *(INT*)&config.speed.enabled);

						{
							CHAR str[32];
							LoadString(hDllModule, IDS_SPEED, str, sizeof(str));

							CHAR text[64];
							StrPrint(text, "%s: %.1fx", str, config.speed.value);
							Hooks::PrintText(text);
						}

						CheckMenu(MenuSpeed);
					}
				}
				else if (config.keys.fpsCounter && config.keys.fpsCounter + VK_F1 - 1 == wParam)
				{
					switch (fpsState)
					{
					case FpsNormal:
						fpsState = FpsBenchmark;
						break;
					case FpsBenchmark:
						fpsState = FpsDisabled;
						break;
					default:
						fpsState = FpsNormal;
						break;
					}

					isFpsChanged = TRUE;
					OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
					if (ddraw)
						SetEvent(ddraw->hDrawEvent);

					return NULL;
				}
				else if (config.keys.imageFilter && config.keys.imageFilter + VK_F1 - 1 == wParam)
				{
					switch (config.image.interpolation)
					{
					case InterpolateNearest:
						InterpolationChanged(hWnd, InterpolateLinear);
						break;

					case InterpolateLinear:
						InterpolationChanged(hWnd, glVersion >= GL_VER_2_0 ? InterpolateHermite : InterpolateNearest);
						break;

					case InterpolateHermite:
						InterpolationChanged(hWnd, glVersion >= GL_VER_2_0 ? InterpolateCubic : InterpolateNearest);
						break;

					default:
						InterpolationChanged(hWnd, InterpolateNearest);
						break;
					}

					return NULL;
				}
				else if (config.keys.aspectRatio && config.keys.aspectRatio + VK_F1 - 1 == wParam)
				{
					WindowProc(hWnd, WM_COMMAND, IDM_ASPECT_RATIO, NULL);
					return NULL;
				}
				else if (config.keys.vSync && config.keys.vSync + VK_F1 - 1 == wParam)
				{
					return WindowProc(hWnd, WM_COMMAND, IDM_VSYNC, NULL);
					return NULL;
				}
				else if (config.keys.windowedMode && config.keys.windowedMode + VK_F1 - 1 == wParam)
				{
					return WindowProc(hWnd, WM_COMMAND, IDM_RES_FULL_SCREEN, NULL);
					return NULL;
				}
				else if (config.keys.showBorders && config.keys.showBorders + VK_F1 - 1 == wParam)
				{
					return WindowProc(hWnd, WM_COMMAND, IDM_RES_BORDERS, NULL);
					return NULL;
				}
				else if (config.keys.zoomImage && config.keys.zoomImage + VK_F1 - 1 == wParam)
				{
					return WindowProc(hWnd, WM_COMMAND, IDM_STRETCH_OFF + (!config.zoom.menu ? config.zoom.value : 0), NULL);
					return NULL;
				}
				else if (config.keys.speedToggle && config.keys.speedToggle + VK_F1 - 1 == wParam)
				{
					config.speed.enabled = !config.speed.enabled;
					Config::Set(CONFIG_WRAPPER, "SpeedEnabled", *(INT*)&config.speed.enabled);

					{
						DWORD index = config.speed.enabled ? config.speed.index : 0;
						FLOAT value = 0.1f * (index + 10);

						CHAR str[32];
						LoadString(hDllModule, IDS_SPEED, str, sizeof(str));

						CHAR text[64];
						StrPrint(text, "%s: %.1fx", str, value);
						Hooks::PrintText(text);
					}

					CheckMenu(MenuSpeed);
					return NULL;
				}
				else if (config.keys.snapshot && config.keys.snapshot + VK_F1 - 1 == wParam)
				{
					OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
					if (ddraw)
						ddraw->isTakeSnapshot = SnapshotFile;

					return NULL;
				}
				else if (!config.version && wParam == VK_F12)
					return NULL;
			}
			else
			{
				if (config.keys.snapshot && config.keys.snapshot + VK_F1 - 1 == wParam)
				{
					OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
					if (ddraw)
						ddraw->isTakeSnapshot = SnapshotSurfaceFile;

					return NULL;
				}
				else if (config.version && wParam == VK_F10)
					return NULL;
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_MOUSEMOVE:
		case WM_MOUSEWHEEL:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		{
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
			{
				POINT p = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				ddraw->ScaleMouse(&p);
				lParam = MAKELONG(p.x, p.y);
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_COMMAND:
		{
			switch (wParam)
			{
			case IDM_FILE_RESET:
			{
				if (Main::ShowWarn(IDS_WARN_RESET))
				{
					Config::Set(CONFIG_WRAPPER, "ReInit", TRUE);
					Main::ShowInfo(IDS_INFO_RESET);
				}

				return NULL;
			}

			case IDM_FILE_QUIT:
			{
				SendMessage(hWnd, WM_CLOSE, NULL, NULL);
				return NULL;
			}

			case IDM_HELP_ABOUT_APPLICATION:
			case IDM_HELP_ABOUT_WRAPPER:
			{
				ULONG_PTR cookie = NULL;
				if (hActCtx && hActCtx != INVALID_HANDLE_VALUE && !ActivateActCtxC(hActCtx, &cookie))
					cookie = NULL;

				LPCSTR id;
				DLGPROC proc;
				if (wParam == IDM_HELP_ABOUT_APPLICATION)
				{
					id = MAKEINTRESOURCE(IDD_ABOUT_APPLICATION);
					proc = (DLGPROC)AboutApplicationProc;
				}
				else
				{
					id = MAKEINTRESOURCE(cookie ? IDD_ABOUT_WRAPPER : IDD_ABOUT_WRAPPER_OLD);
					proc = (DLGPROC)AboutWrapperProc;
				}

				DialogBoxParam(hDllModule, id, hWnd, proc, NULL);

				if (cookie)
					DeactivateActCtxC(0, cookie);

				SetForegroundWindow(hWnd);
				return NULL;
			}

			case IDM_RES_FULL_SCREEN:
			{
				OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
				if (ddraw && !ddraw->isFinish)
				{
					ddraw->RenderStop();
					{
						if (!config.windowedMode)
							ddraw->SetWindowedMode();
						else
							ddraw->SetFullscreenMode();

						config.windowedMode = !config.windowedMode;
						Config::Set(CONFIG_DISCIPLE, config.version ? "InWindow" : "DisplayMode", config.windowedMode);

						if (!config.windowedMode)
						{
							CHAR str1[32];
							LoadString(hDllModule, IDS_RES_FULL_SCREEN, str1, sizeof(str1));

							CHAR str2[32];
							LoadString(hDllModule, config.borderless.real ? IDS_MODE_BORDERLESS : IDS_MODE_EXCLUSIVE, str2, sizeof(str2));

							CHAR text[64];
							StrPrint(text, "%s: %s", str1, str2);

							Hooks::PrintText(text);
						}

						CheckMenu(MenuWindowMode);
					}
					ddraw->RenderStart();
				}

				return NULL;
			}

			case IDM_ASPECT_RATIO:
			{
				config.image.aspect = !config.image.aspect;
				Config::Set(CONFIG_WRAPPER, "ImageAspect", config.image.aspect);

				{
					CHAR str1[32];
					LoadString(hDllModule, IDS_ASPECT_RATIO, str1, sizeof(str1));

					CHAR str2[32];
					LoadString(hDllModule, config.image.aspect ? IDS_ASPECT_KEEP : IDS_ASPECT_STRETCH, str2, sizeof(str2));

					CHAR text[64];
					StrPrint(text, "%s: %s", str1, str2);
					Hooks::PrintText(text);
				}

				OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
				if (ddraw)
				{
					ddraw->viewport.refresh = TRUE;
					SetEvent(ddraw->hDrawEvent);
				}

				CheckMenu(MenuAspect);

				return NULL;
			}

			case IDM_VSYNC:
			{
				config.image.vSync = !config.image.vSync;
				Config::Set(CONFIG_WRAPPER, "ImageVSync", config.image.vSync);

				{
					CHAR str1[32];
					LoadString(hDllModule, IDS_VSYNC, str1, sizeof(str1));

					CHAR str2[32];
					LoadString(hDllModule, config.image.vSync ? IDS_ON : IDS_OFF, str2, sizeof(str2));

					CHAR text[64];
					StrPrint(text, "%s: %s", str1, str2);
					Hooks::PrintText(text);
				}

				OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
				if (ddraw)
					SetEvent(ddraw->hDrawEvent);

				CheckMenu(MenuVSync);

				return NULL;
			}

			case IDM_FILT_OFF:
			{
				InterpolationChanged(hWnd, InterpolateNearest);
				return NULL;
			}

			case IDM_FILT_LINEAR:
			{
				InterpolationChanged(hWnd, InterpolateLinear);
				return NULL;
			}

			case IDM_FILT_HERMITE:
			{
				InterpolationChanged(hWnd, InterpolateHermite);
				return NULL;
			}

			case IDM_FILT_CUBIC:
			{
				InterpolationChanged(hWnd, InterpolateCubic);
				return NULL;
			}

			case IDM_FILT_NONE:
			{
				UpscalingChanged(hWnd, UpscaleNone);
				return NULL;
			}

			case IDM_FILT_SCALENX_2X:
			{
				SelectScaleNxMode(hWnd, 2);
				return NULL;
			}

			case IDM_FILT_SCALENX_3X:
			{
				SelectScaleNxMode(hWnd, 3);
				return NULL;
			}

			case IDM_FILT_XSAL_2X:
			{
				SelectXSalMode(hWnd, 2);
				return NULL;
			}

			case IDM_FILT_EAGLE_2X:
			{
				SelectEagleMode(hWnd, 2);
				return NULL;
			}

			case IDM_FILT_SCALEHQ_2X:
			{
				SelectScaleHQMode(hWnd, 2);
				return NULL;
			}

			case IDM_FILT_SCALEHQ_4X:
			{
				SelectScaleHQMode(hWnd, 4);
				return NULL;
			}

			case IDM_FILT_XRBZ_2X:
			{
				SelectXBRZMode(hWnd, 2);
				return NULL;
			}

			case IDM_FILT_XRBZ_3X:
			{
				SelectXBRZMode(hWnd, 3);
				return NULL;
			}

			case IDM_FILT_XRBZ_4X:
			{
				SelectXBRZMode(hWnd, 4);
				return NULL;
			}

			case IDM_FILT_XRBZ_5X:
			{
				SelectXBRZMode(hWnd, 5);
				return NULL;
			}

			case IDM_FILT_XRBZ_6X:
			{
				SelectXBRZMode(hWnd, 6);
				return NULL;
			}

			case IDM_RES_BORDERS:
			{
				if (!config.version)
				{
					config.border.enabled = !config.border.enabled;
					Config::Set(CONFIG_DISCIPLE, "ShowInterfBorder", config.border.enabled);

					if (config.resHooked)
					{
						{
							CHAR str1[32];
							LoadString(hDllModule, IDS_RES_BORDERS, str1, sizeof(str1));

							CHAR str2[32];
							LoadString(hDllModule, config.border.enabled ? IDS_ON : IDS_OFF, str2, sizeof(str2));

							CHAR text[64];
							StrPrint(text, "%s: %s", str1, str2);
							Hooks::PrintText(text);
						}

						OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
						if (ddraw)
							SetEvent(ddraw->hDrawEvent);
					}
					else
						Main::ShowInfo(IDS_INFO_RESTART);

					CheckMenu(MenuBorders);

					return NULL;
				}
				else
					return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
			}

			case IDM_MODE_BORDERLESS:
			{
				config.borderless.real = config.borderless.mode = TRUE;
				Config::Set(CONFIG_WRAPPER, "BorderlessMode", config.borderless.mode);
				CheckMenu(MenuWindowType);
				return NULL;
			}

			case IDM_MODE_EXCLUSIVE:
			{
				config.borderless.real = config.borderless.mode = FALSE;
				Config::Set(CONFIG_WRAPPER, "BorderlessMode", config.borderless.mode);
				CheckMenu(MenuWindowType);
				return NULL;
			}

			case IDM_FAST_AI:
			{
				if (config.ai.fast || Main::ShowWarn(IDS_WARN_FAST_AI))
				{
					config.ai.fast = !config.ai.fast;
					Config::Set(CONFIG_WRAPPER, "FastAI", config.ai.fast);

					{
						CHAR str1[32];
						LoadString(hDllModule, IDS_FAST_AI, str1, sizeof(str1));

						CHAR str2[32];
						LoadString(hDllModule, config.ai.fast ? IDS_ON : IDS_OFF, str2, sizeof(str2));

						CHAR text[64];
						StrPrint(text, "%s: %s", str1, str2);
						Hooks::PrintText(text);
					}

					CheckMenu(MenuFastAI);
				}

				return NULL;
			}

			case IDM_ALWAYS_ACTIVE:
			{
				config.alwaysActive = !config.alwaysActive;
				Config::Set(CONFIG_WRAPPER, "AlwaysActive", config.alwaysActive);

				{
					CHAR str1[32];
					LoadString(hDllModule, IDS_ALWAYS_ACTIVE, str1, sizeof(str1));

					CHAR str2[32];
					LoadString(hDllModule, config.alwaysActive ? IDS_ON : IDS_OFF, str2, sizeof(str2));

					CHAR text[64];
					StrPrint(text, "%s: %s", str1, str2);
					Hooks::PrintText(text);
				}

				CheckMenu(MenuActive);
				return NULL;
			}

			case IDM_PATCH_CPU:
			{
				config.coldCPU = !config.coldCPU;
				if (config.coldCPU)
					timeBeginPeriod(1);
				else
					timeEndPeriod(1);

				Config::Set(CONFIG_WRAPPER, "ColdCPU", config.coldCPU);

				{
					CHAR str1[32];
					LoadString(hDllModule, IDS_PATCH_CPU, str1, sizeof(str1));

					CHAR str2[32];
					LoadString(hDllModule, config.coldCPU ? IDS_ON : IDS_OFF, str2, sizeof(str2));

					CHAR text[64];
					StrPrint(text, "%s: %s", str1, str2);
					Hooks::PrintText(text);
				}

				Window::CheckMenu(MenuCpu);
				return NULL;
			}

			case IDM_MODE_WIDEBATTLE:
			{
				config.wide.allowed = !config.wide.allowed;
				Config::Set(CONFIG_WRAPPER, "WideBattle", config.wide.allowed);

				Main::ShowInfo(IDS_INFO_NEXT_BATTLE);

				Window::CheckMenu(MenuBattle);
				return NULL;
			}

			case IDM_SCR_BMP:
			case IDM_SCR_PNG:
			{
				ImageType type = wParam == IDM_SCR_PNG ? ImagePNG : ImageBMP;
				if (config.snapshot.type != type)
				{
					config.snapshot.type = type;
					Config::Set(CONFIG_WRAPPER, "ScreenshotType", *(INT*)&config.snapshot.type);

					{
						CHAR str1[32];
						LoadString(hDllModule, IDS_SCR_TYPE, str1, sizeof(str1));

						CHAR str2[32];
						LoadString(hDllModule, config.snapshot.type == ImagePNG ? IDS_SCR_PNG : IDS_SCR_BMP, str2, sizeof(str2));

						CHAR text[64];
						StrPrint(text, "%s: %s", str1, str2);
						Hooks::PrintText(text);
					}

					Window::CheckMenu(MenuSnapshotType);
				}

				return NULL;
			}

			default:
				if (wParam >= IDM_RES_640_480 && wParam < IDM_RES_640_480 + sizeof(resolutionsList) / sizeof(Resolution))
				{
					DWORD idx = wParam - IDM_RES_640_480;
					const Resolution* resItem = &resolutionsList[idx];

					if (resItem->width != config.resolution.width || resItem->height != config.resolution.height)
					{
						config.resolution = *resItem;

						if (config.resHooked)
						{
							Config::Set(CONFIG_WRAPPER, "DisplayWidth", (INT)config.resolution.width);
							Config::Set(CONFIG_WRAPPER, "DisplayHeight", (INT)config.resolution.height);
						}
						else if (!config.version)
						{
							INT mode;
							if (config.resolution.width == 1280)
								mode = 2;
							else if (config.resolution.width == 1024)
								mode = 1;
							else
								mode = 0;

							Config::Set(CONFIG_DISCIPLE, "DisplaySize", mode);
						}

						Main::ShowInfo(IDS_INFO_RESTART);
						CheckMenu(MenuResolution);
					}

					return NULL;
				}
				else if (wParam >= IDM_STRETCH_OFF && wParam <= IDM_STRETCH_OFF + 100)
				{
					if (!config.version && config.resHooked && config.zoom.allowed)
					{
						DWORD value = wParam - IDM_STRETCH_OFF;

						if (value)
						{
							config.zoom.menu = TRUE;

							config.zoom.value = value;
							Config::Set(CONFIG_WRAPPER, "ZoomFactor", *(INT*)&config.zoom.value);
							config.zoom.factor = 0.01f * config.zoom.value;

							Config::CalcZoomed();
						}
						else
							config.zoom.menu = FALSE;

						config.zoom.menu = value != 0;
						Config::Set(CONFIG_DISCIPLE, "EnableZoom", config.zoom.menu);

						if (config.mode->width > *(DWORD*)&GAME_WIDTH && config.mode->height > *(DWORD*)&GAME_HEIGHT)
							config.zoom.enabled = config.zoom.menu;

						{
							CHAR str1[32];
							LoadString(hDllModule, IDS_STRETCH, str1, sizeof(str1));

							CHAR text[64];
							if (!config.zoom.menu)
							{
								CHAR str2[32];
								LoadString(hDllModule, IDS_OFF, str2, sizeof(str2));
								StrPrint(text, "%s: %s", str1, str2);
							}
							else
								StrPrint(text, "%s: %d%%", str1, config.zoom.value);

							Hooks::PrintText(text);
						}

						OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
						if (ddraw)
							SetEvent(ddraw->hDrawEvent);

						CheckMenu(MenuStretch);
					}

					return NULL;
				}
				else if (wParam >= IDM_SPEED_1_0 && wParam <= IDM_SPEED_3_0)
				{
					DWORD index = wParam - IDM_SPEED_1_0;
					if (index != config.speed.index || !config.speed.enabled)
					{
						FLOAT value;
						if (index)
						{
							config.speed.index = index;
							config.speed.value = 0.1f * (index + 10);
							config.speed.enabled = TRUE;
							Config::Set(CONFIG_WRAPPER, "GameSpeed", *(INT*)&index);
							value = (FLOAT)config.speed.value;
						}
						else
						{
							config.speed.enabled = FALSE;
							value = 1.0f;
						}

						Config::Set(CONFIG_WRAPPER, "SpeedEnabled", *(INT*)&config.speed.enabled);

						{
							CHAR str[32];
							LoadString(hDllModule, IDS_SPEED, str, sizeof(str));

							CHAR text[64];
							StrPrint(text, "%s: %.1fx", str, value);
							Hooks::PrintText(text);
						}

						CheckMenu(MenuSpeed);
					}

					return NULL;
				}
				else if (wParam >= IDM_SCR_1 && wParam <= IDM_SCR_9)
				{
					DWORD index = wParam - IDM_SCR_1 + 1;
					if (config.snapshot.level != index)
					{
						config.snapshot.level = index;
						Config::Set(CONFIG_WRAPPER, "ScreenshotLevel", *(INT*)&config.snapshot.level);

						CheckMenu(MenuSnapshotLevel);
					}

					return NULL;
				}
				else if (wParam >= IDM_LOCALE_DEFAULT && wParam <= IDM_LOCALE_DEFAULT + config.locales.count)
				{
					LCID id = config.locales.list[wParam - IDM_LOCALE_DEFAULT];
					if (config.locales.current.id != id)
					{
						config.locales.current.id = id;
						Config::Set(CONFIG_WRAPPER, "Locale", *(INT*)&config.locales.current.id);
						Config::UpdateLocale();

						{
							CHAR text[160];
							CHAR str1[32];
							LoadString(hDllModule, IDS_LOCALE, str1, sizeof(str1));

							if (config.locales.current.id)
							{

								CHAR name[64];
								GetLocaleInfo(config.locales.current.id, LOCALE_SENGLISHLANGUAGENAME, name, sizeof(name));

								CHAR country[64];
								GetLocaleInfo(config.locales.current.id, LOCALE_SENGLISHCOUNTRYNAME, country, sizeof(country));

								StrPrint(text, "%s: %s (%s)", str1, name, country);
							}
							else
							{
								CHAR str2[32];
								LoadString(hDllModule, IDS_LOCALE_DEFAULT, str2, sizeof(str2));

								StrPrint(text, "%s: %s", str1, str2);
							}

							Hooks::PrintText(text);
						}

						CheckMenu(MenuLocale);
					}

					return NULL;
				}
				else if (wParam > IDM_MSG_0 && wParam <= IDM_MSG_30)
				{
					DWORD time = wParam - IDM_MSG_0;
					if (config.msgTimeScale.time != time)
					{
						config.msgTimeScale.time = time;
						config.msgTimeScale.value = (FLOAT)MSG_TIMEOUT / config.msgTimeScale.time;

						Config::Set(CONFIG_WRAPPER, "MessageTimeout", *(INT*)&config.msgTimeScale.time);

						{
							CHAR str1[32];
							LoadString(hDllModule, IDS_MSG, str1, sizeof(str1));

							CHAR str2[32];
							LoadString(hDllModule, IDS_MSG_SECONDS, str2, sizeof(str2));

							CHAR text[64];
							StrPrint(text, "%s: %d %s", str1, config.msgTimeScale.time, str2);
							Hooks::PrintText(text);
						}

						CheckMenu(MenuMsgTimeScale);
					}

					return NULL;
				}
				else
					return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
			}
		}

		case WM_DISPLAYCHANGE:
		{
			DEVMODE devMode;
			MemoryZero(&devMode, sizeof(DEVMODE));
			devMode.dmSize = sizeof(DEVMODE);
			config.syncStep = 1000.0 / (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode) && (devMode.dmFields & DM_DISPLAYFREQUENCY) ? devMode.dmDisplayFrequency : 60);

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		default:
			if (uMsg == WM_SNAPSHOT)
			{
				CHAR str[32];
				LoadString(hDllModule, IDS_SCR_FILE, str, sizeof(str));

				CHAR text[MAX_PATH + 36];
				StrPrint(text, "%s: %s", str, snapshotName);
				Hooks::PrintText(text);

				return NULL;
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}
	}

	LRESULT __stdcall PanelProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC hDc = BeginPaint(hWnd, &paint);
			if (hDc)
				EndPaint(hWnd, &paint);

			return NULL;
		}

		case WM_ERASEBKGND:
			return TRUE;

		case WM_NCHITTEST:
		case WM_SETCURSOR:

		case WM_MOUSEMOVE:
		case WM_MOUSEWHEEL:

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDBLCLK:

		case WM_SYSCOMMAND:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
			return WindowProc(GetParent(hWnd), uMsg, wParam, lParam);

		default:
			return CallWindowProc(OldPanelProc, hWnd, uMsg, wParam, lParam);
		}
	}

	VOID __fastcall SetCaptureKeys(BOOL state)
	{
		if (state)
		{
			if (!OldKeysHook)
				OldKeysHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeysHook, hDllModule, NULL);
		}
		else
		{
			if (OldKeysHook && UnhookWindowsHookEx(OldKeysHook))
				OldKeysHook = NULL;
		}
	}

	VOID __fastcall SetCaptureWindow(HWND hWnd)
	{
		OldWindowProc = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (LPARAM)WindowProc);
	}

	VOID __fastcall SetCapturePanel(HWND hWnd)
	{
		OldPanelProc = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (LPARAM)PanelProc);
	}
}