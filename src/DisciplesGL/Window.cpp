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
#include "timeapi.h"
#include "Window.h"
#include "CommCtrl.h"
#include "Windowsx.h"
#include "Shellapi.h"
#include "Resource.h"
#include "Main.h"
#include "Config.h"
#include "Hooks.h"
#include "OpenDraw.h"
#include "PngLib.h"

#define WM_REDRAW_CANVAS 0x8001

namespace Window
{
	HHOOK OldKeysHook;
	WNDPROC OldWindowProc, OldPanelProc;

	VOID __fastcall BeginDialog(DialogParams* params)
	{
		params->ddraw = Main::FindOpenDrawByWindow(params->hWnd);
		if (params->ddraw)
		{
			if (!config.windowedMode && !config.borderless.real)
			{
				params->ddraw->RenderStop();

				config.borderless.mode = TRUE;
				if (params->isGrayed)
					config.colors.current = &inactiveColors;

				params->ddraw->RenderStart();
			}
			else
			{
				if (params->isGrayed)
				{
					config.colors.current = &inactiveColors;
					params->ddraw->Redraw();
				}
			}
		}
		else
		{
			GetClipCursor(&params->clip);
			ClipCursor(NULL);
		}

		if (hActCtx && hActCtx != INVALID_HANDLE_VALUE && !ActivateActCtxC(hActCtx, &params->cookie))
			params->cookie = NULL;
	}

	VOID __fastcall EndDialog(DialogParams* params)
	{
		if (params->cookie)
			DeactivateActCtxC(0, params->cookie);

		if (params->ddraw)
		{
			if (!config.windowedMode && !config.borderless.real)
			{
				params->ddraw->RenderStop();

				config.borderless.mode = FALSE;
				if (params->isGrayed)
					config.colors.current = &config.colors.active;

				params->ddraw->RenderStart();
			}
			else
			{
				if (params->isGrayed)
				{
					config.colors.current = &config.colors.active;
					params->ddraw->Redraw();
				}
			}
		}
		else
			ClipCursor(&params->clip);

		if (params->hWnd)
			SetForegroundWindow(params->hWnd);
	}

	BYTE __fastcall CubicInterpolate(BYTE p0, BYTE p1, BYTE p2, BYTE p3, FLOAT x)
	{
		INT d = INT(p1 + 0.5 * x * (p2 - p0 + x * (2 * p0 - 5 * p1 + 4 * p2 - p3 + x * (3 * (p1 - p2) + p3 - p0))));
		if (d > 0xFF)
			d = 0xFF;
		else if (d < 0)
			d = 0;
		return LOBYTE(d);
	}

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
		case MenuLocale: {
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

		case MenuAspect: {
			CheckMenuItem(config.menu, IDM_ASPECT_RATIO, MF_BYCOMMAND | (config.image.aspect ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuVSync: {
			EnableMenuItem(config.menu, IDM_VSYNC, MF_BYCOMMAND | (config.gl.version.value && WGLSwapInterval ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			CheckMenuItem(config.menu, IDM_VSYNC, MF_BYCOMMAND | (config.gl.version.value && WGLSwapInterval && config.image.vSync ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuInterpolate: {
			CheckMenuItem(config.menu, IDM_FILT_OFF, MF_BYCOMMAND | MF_UNCHECKED);

			EnableMenuItem(config.menu, IDM_FILT_LINEAR, MF_BYCOMMAND | (config.gl.version.value ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			CheckMenuItem(config.menu, IDM_FILT_LINEAR, MF_BYCOMMAND | MF_UNCHECKED);

			EnableMenuItem(config.menu, IDM_FILT_HERMITE, MF_BYCOMMAND | (config.gl.version.value >= GL_VER_2_0 ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			CheckMenuItem(config.menu, IDM_FILT_HERMITE, MF_BYCOMMAND | MF_UNCHECKED);

			EnableMenuItem(config.menu, IDM_FILT_CUBIC, MF_BYCOMMAND | (config.gl.version.value >= GL_VER_2_0 ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			CheckMenuItem(config.menu, IDM_FILT_CUBIC, MF_BYCOMMAND | MF_UNCHECKED);

			EnableMenuItem(config.menu, IDM_FILT_LANCZOS, MF_BYCOMMAND | (config.gl.version.value >= GL_VER_2_0 ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			CheckMenuItem(config.menu, IDM_FILT_LANCZOS, MF_BYCOMMAND | MF_UNCHECKED);

			switch (config.image.interpolation)
			{
			case InterpolateLinear:
				CheckMenuItem(config.menu, (config.gl.version.value ? IDM_FILT_LINEAR : IDM_FILT_OFF), MF_BYCOMMAND | MF_CHECKED);
				break;

			case InterpolateHermite:
				CheckMenuItem(config.menu, config.gl.version.value >= GL_VER_2_0 ? IDM_FILT_HERMITE : (config.gl.version.value ? IDM_FILT_LINEAR : IDM_FILT_OFF), MF_BYCOMMAND | MF_CHECKED);
				break;

			case InterpolateCubic:
				CheckMenuItem(config.menu, config.gl.version.value >= GL_VER_2_0 ? IDM_FILT_CUBIC : (config.gl.version.value ? IDM_FILT_LINEAR : IDM_FILT_OFF), MF_BYCOMMAND | MF_CHECKED);
				break;

			case InterpolateLanczos:
				CheckMenuItem(config.menu, config.gl.version.value >= GL_VER_2_0 ? IDM_FILT_LANCZOS : (config.gl.version.value ? IDM_FILT_LINEAR : IDM_FILT_OFF), MF_BYCOMMAND | MF_CHECKED);
				break;

			default:
				CheckMenuItem(config.menu, IDM_FILT_OFF, MF_BYCOMMAND | MF_CHECKED);
				break;
			}
		}
		break;

		case MenuUpscale: {
			CheckMenuItem(config.menu, IDM_FILT_NONE, MF_BYCOMMAND | MF_UNCHECKED);

			DWORD isFilters = config.gl.version.value >= GL_VER_3_0 ? MF_ENABLED : (MF_DISABLED | MF_GRAYED);

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

				case UpscaleXBRZ:
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

		case MenuResolution: {
			DWORD count = sizeof(resolutionsList) / sizeof(Resolution);
			const Resolution* resItem = resolutionsList;
			DWORD id = IDM_RES_640_480;
			while (count)
			{
				--count;
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

		case MenuSpeed: {
			DWORD count = IDM_SPEED_3_0 - IDM_SPEED_1_0 + 1;
			DWORD id = IDM_SPEED_1_0;
			while (count)
			{
				--count;
				CheckMenuItem(config.menu, id, MF_BYCOMMAND | (config.speed.enabled && id - IDM_SPEED_1_0 == config.speed.index || !config.speed.enabled && id == IDM_SPEED_1_0 ? MF_CHECKED : MF_UNCHECKED));
				++id;
			}
		}
		break;

		case MenuColors: {
			EnableMenuItem(config.menu, IDM_COLOR_ADJUST, MF_BYCOMMAND | (config.gl.version.value >= GL_VER_2_0 ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
		}
		break;

		case MenuBorders: {
			MenuItemData mData;
			mData.childId = IDM_BORDERS_OFF;
			if (GetMenuByChildID(&mData))
			{
				if (!config.borders.allowed)
				{
					EnableMenuItem(mData.hParent, mData.index, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
					CheckMenuItem(mData.hParent, mData.index, MF_BYPOSITION | MF_UNCHECKED);
				}
				else
				{
					EnableMenuItem(mData.hParent, mData.index, MF_BYPOSITION | MF_ENABLED);
					CheckMenuItem(mData.hParent, mData.index, MF_BYPOSITION | (config.borders.type != BordersNone ? MF_CHECKED : MF_UNCHECKED));

					UINT check = IDM_BORDERS_OFF + config.borders.type;
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

		case MenuBackground: {
			if (config.background.allowed)
			{
				EnableMenuItem(config.menu, IDM_BACKGROUND, MF_BYCOMMAND | MF_ENABLED);
				CheckMenuItem(config.menu, IDM_BACKGROUND, MF_BYCOMMAND | (config.background.enabled ? MF_CHECKED : MF_UNCHECKED));
			}
			else
			{
				EnableMenuItem(config.menu, IDM_BACKGROUND, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
				CheckMenuItem(config.menu, IDM_BACKGROUND, MF_BYCOMMAND | MF_UNCHECKED);
			}
		}
		break;

		case MenuStretch: {
			MenuItemData mData;
			mData.childId = IDM_STRETCH_OFF;
			if (GetMenuByChildID(&mData))
			{
				if (!config.zoom.allowed || !config.zoom.glallow)
				{
					EnableMenuItem(mData.hParent, mData.index, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
					CheckMenuItem(mData.hParent, mData.index, MF_BYPOSITION | MF_UNCHECKED);
				}
				else
				{
					EnableMenuItem(mData.hParent, mData.index, MF_BYPOSITION | MF_ENABLED);
					CheckMenuItem(mData.hParent, mData.index, MF_BYPOSITION | (config.zoom.enabled ? MF_CHECKED : MF_UNCHECKED));

					UINT check = IDM_STRETCH_OFF + (config.zoom.enabled ? config.zoom.value : 0);
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

		case MenuWindowMode: {
			CheckMenuItem(config.menu, IDM_RES_FULL_SCREEN, MF_BYCOMMAND | (!config.windowedMode ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuWindowType: {
			MenuItemData mData;
			mData.childId = IDM_MODE_BORDERLESS;
			if (GetMenuByChildID(&mData))
				EnableMenuItem(mData.hParent, mData.index, MF_BYPOSITION | (config.renderer != RendererGDI ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));

			if (config.renderer != RendererGDI)
			{
				CheckMenuItem(config.menu, IDM_MODE_BORDERLESS, MF_BYCOMMAND | (config.borderless.mode ? MF_CHECKED : MF_UNCHECKED));
				CheckMenuItem(config.menu, IDM_MODE_EXCLUSIVE, MF_BYCOMMAND | (!config.borderless.mode ? MF_CHECKED : MF_UNCHECKED));
			}
		}
		break;

		case MenuFastAI: {
			EnableMenuItem(config.menu, IDM_FAST_AI, MF_BYCOMMAND | (!config.isEditor ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			CheckMenuItem(config.menu, IDM_FAST_AI, MF_BYCOMMAND | (!config.isEditor && config.ai.fast ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuActive: {
			CheckMenuItem(config.menu, IDM_ALWAYS_ACTIVE, MF_BYCOMMAND | (config.alwaysActive ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuCpu: {
			EnableMenuItem(config.menu, IDM_PATCH_CPU, MF_BYCOMMAND | (config.renderer != RendererGDI && !config.singleThread ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			CheckMenuItem(config.menu, IDM_PATCH_CPU, MF_BYCOMMAND | (config.renderer != RendererGDI && !config.singleThread && config.coldCPU ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuBattle: {
			EnableMenuItem(config.menu, IDM_MODE_WIDEBATTLE, MF_BYCOMMAND | (config.wide.hooked ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			CheckMenuItem(config.menu, IDM_MODE_WIDEBATTLE, MF_BYCOMMAND | (config.wide.hooked && config.wide.allowed ? MF_CHECKED : MF_UNCHECKED));
		}
		break;

		case MenuSnapshot: {
			EnableMenuItem(config.menu, IDM_SCR_PNG, MF_BYCOMMAND | (pnglib_create_write_struct ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));

			BOOL enabled = config.snapshot.type == ImagePNG && pnglib_create_write_struct;
			CheckMenuItem(config.menu, IDM_SCR_BMP, MF_BYCOMMAND | (!enabled ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(config.menu, IDM_SCR_PNG, MF_BYCOMMAND | (enabled ? MF_CHECKED : MF_UNCHECKED));

			MenuItemData mData;
			mData.childId = IDM_SCR_1;
			if (GetMenuByChildID(&mData))
				EnableMenuItem(mData.hParent, mData.index, MF_BYPOSITION | (enabled ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));

			if (enabled)
			{
				DWORD count = IDM_SCR_9 - IDM_SCR_1 + 1;
				DWORD id = IDM_SCR_1;
				while (count)
				{
					--count;
					CheckMenuItem(config.menu, id, MF_BYCOMMAND | (pnglib_create_write_struct && id - IDM_SCR_PNG == config.snapshot.level ? MF_CHECKED : MF_UNCHECKED));
					++id;
				}
			}
		}
		break;

		case MenuMsgTimeScale: {
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

		case MenuRenderer: {
			EnableMenuItem(config.menu, IDM_REND_GL2, MF_BYCOMMAND | (config.gl.version.real >= GL_VER_2_0 ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			EnableMenuItem(config.menu, IDM_REND_GL3, MF_BYCOMMAND | (config.gl.version.real >= GL_VER_3_0 ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));

			CheckMenuItem(config.menu, IDM_REND_AUTO, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(config.menu, IDM_REND_GL1, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(config.menu, IDM_REND_GL2, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(config.menu, IDM_REND_GL3, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(config.menu, IDM_REND_GDI, MF_BYCOMMAND | MF_UNCHECKED);

			switch (config.renderer)
			{
			case RendererOpenGL1:
				CheckMenuItem(config.menu, IDM_REND_GL1, MF_BYCOMMAND | MF_CHECKED);
				break;

			case RendererOpenGL2:
				CheckMenuItem(config.menu, IDM_REND_GL2, MF_BYCOMMAND | MF_CHECKED);
				break;

			case RendererOpenGL3:
				CheckMenuItem(config.menu, IDM_REND_GL3, MF_BYCOMMAND | MF_CHECKED);
				break;

			case RendererGDI:
				CheckMenuItem(config.menu, IDM_REND_GDI, MF_BYCOMMAND | MF_CHECKED);
				break;

			default:
				CheckMenuItem(config.menu, IDM_REND_AUTO, MF_BYCOMMAND | MF_CHECKED);
				break;
			}
		}

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
		CheckMenu(MenuColors);
		CheckMenu(MenuBorders);
		CheckMenu(MenuBackground);
		CheckMenu(MenuStretch);
		CheckMenu(MenuWindowMode);
		CheckMenu(MenuWindowType);
		CheckMenu(MenuFastAI);
		CheckMenu(MenuActive);
		CheckMenu(MenuCpu);
		CheckMenu(MenuBattle);
		CheckMenu(MenuSnapshot);
		CheckMenu(MenuMsgTimeScale);
	}

	VOID __fastcall FilterChanged(HWND hWnd, const CHAR* name, INT value)
	{
		Config::Set(CONFIG_WRAPPER, name, value);

		OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
		if (ddraw)
		{
			ddraw->LoadFilterState();
			ddraw->Redraw();
		}
	}

	VOID __fastcall InterpolationChanged(HWND hWnd, InterpolationFilter filter)
	{
		config.image.interpolation = config.gl.version.value >= GL_VER_2_0 || filter < InterpolateHermite ? filter : InterpolateLinear;

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
			case InterpolateLanczos:
				id = IDS_TEXT_FILT_LANCZOS;
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

		FilterChanged(hWnd, "Interpolation", (INT)config.image.interpolation);
		CheckMenu(MenuInterpolate);
	}

	VOID __fastcall UpscalingChanged(HWND hWnd, UpscalingFilter filter)
	{
		config.image.upscaling = config.gl.version.value >= GL_VER_3_0 ? filter : UpscaleNone;

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
			case UpscaleXBRZ:
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

		FilterChanged(hWnd, "Upscaling", (INT)config.image.upscaling);
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
		UpscalingChanged(hWnd, UpscaleXBRZ);
	}

	VOID __fastcall SelectRenderer(HWND hWnd, RendererType renderer)
	{
		if (renderer == config.renderer)
			return;

		if (renderer == RendererGDI || config.renderer == RendererGDI)
		{
			Config::Set(CONFIG_WRAPPER, "Renderer", *(INT*)&renderer);
			Main::ShowInfo(IDS_INFO_RESTART);
		}
		else
		{
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
				ddraw->RenderStop();

			{
				config.renderer = renderer;
				Config::Set(CONFIG_WRAPPER, "Renderer", *(INT*)&renderer);

				{
					CHAR str1[32];
					LoadString(hDllModule, IDS_TEXT_RENDERER, str1, sizeof(str1));

					DWORD id;
					switch (config.renderer)
					{
					case RendererOpenGL1:
						id = IDS_REND_GL1;
						break;

					case RendererOpenGL2:
						id = IDS_REND_GL2;
						break;

					case RendererOpenGL3:
						id = IDS_REND_GL3;
						break;

					case RendererGDI:
						id = IDS_REND_GDI;
						break;

					default:
						id = IDS_REND_AUTO;
						break;
					}

					CHAR str2[32];
					LoadString(hDllModule, id, str2, sizeof(str2));

					CHAR text[64];
					StrPrint(text, "%s: %s", str1, str2);
					Hooks::PrintText(text);
				}

				CheckMenu(MenuRenderer);
			}

			if (ddraw)
				ddraw->RenderStart();
		}
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
					ddraw->Redraw();

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
		case WM_INITDIALOG: {
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

			if (lParam == IDD_ABOUT_APPLICATION)
			{
				StrPrint(path, "<A HREF=\"%s\">%s</A>", "http://www.strategyfirst.com", "http://www.strategyfirst.com");
				SetDlgItemText(hDlg, IDC_LNK_WEB, path);
			}
			else
				SetDlgItemText(hDlg, IDC_LNK_WEB, "http://www.strategyfirst.com");

			break;
		}

		case WM_NOTIFY: {
			if (((NMHDR*)lParam)->code == NM_CLICK)
			{
				switch (wParam)
				{
				case IDC_LNK_WEB:
					SHELLEXECUTEINFOW shExecInfo;
					MemoryZero(&shExecInfo, sizeof(SHELLEXECUTEINFOW));
					shExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
					shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
					shExecInfo.lpFile = ((NMLINK*)lParam)->item.szUrl;
					shExecInfo.nShow = SW_SHOW;
					ShellExecuteExW(&shExecInfo);
					break;

				default:
					break;
				}
			}

			break;
		}

		case WM_COMMAND: {
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
		case WM_INITDIALOG: {
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

			if (GetDlgItemText(hDlg, IDC_COPYRIGHT, temp, sizeof(temp)))
			{
				StrPrint(path, temp, 2020, "Verok");
				SetDlgItemText(hDlg, IDC_COPYRIGHT, path);
			}

			if (lParam == IDD_ABOUT_WRAPPER)
			{
				StrPrint(path, "<A HREF=\"mailto:%s\">%s</A>", "verokster@gmail.com", "verokster@gmail.com");
				SetDlgItemText(hDlg, IDC_LNK_EMAIL, path);

				StrPrint(path, "<A HREF=\"%s\">%s</A>", "https://verokster.blogspot.com/2019/03/disciples-i-ii-gl-wrapper-patch.html", "https://verokster.blogspot.com");
				SetDlgItemText(hDlg, IDC_LNK_WEB, path);

				StrPrint(path, "<A HREF=\"%s\">%s</A>", "https://www.patreon.com/join/verok", "https://www.patreon.com/join/verok");
				SetDlgItemText(hDlg, IDC_LNK_PATRON, path);
			}
			else
			{
				SetDlgItemText(hDlg, IDC_LNK_EMAIL, "verokster@gmail.com");
				SetDlgItemText(hDlg, IDC_LNK_WEB, "https://verokster.blogspot.com");
				SetDlgItemText(hDlg, IDC_LNK_PATRON, "https://www.patreon.com/join/verok");
			}

			break;
		}

		case WM_NOTIFY: {
			if (((NMHDR*)lParam)->code == NM_CLICK)
			{
				switch (wParam)
				{
				case IDC_LNK_EMAIL:
				case IDC_LNK_WEB:
				case IDC_LNK_PATRON:
					SHELLEXECUTEINFOW shExecInfo;
					MemoryZero(&shExecInfo, sizeof(SHELLEXECUTEINFOW));
					shExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
					shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
					shExecInfo.lpFile = ((NMLINK*)lParam)->item.szUrl;
					shExecInfo.nShow = SW_SHOW;
					ShellExecuteExW(&shExecInfo);
					break;

				default:
					break;
				}
			}

			break;
		}

		case WM_COMMAND: {
			if (wParam == IDC_BTN_OK)
				EndDialog(hDlg, TRUE);
			break;
		}

		default:
			break;
		}

		return DefWindowProc(hDlg, uMsg, wParam, lParam);
	}

	LRESULT __stdcall ColorAdjustmentProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_INITDIALOG: {
			SendDlgItemMessage(hDlg, IDC_TRK_HUE, TBM_SETRANGE, FALSE, MAKELPARAM(0, 360));
			SendDlgItemMessage(hDlg, IDC_TRK_SAT, TBM_SETRANGE, FALSE, MAKELPARAM(0, 1000));
			SendDlgItemMessage(hDlg, IDC_TRK_IN_LEFT, TBM_SETRANGE, FALSE, MAKELPARAM(0, 255));
			SendDlgItemMessage(hDlg, IDC_TRK_IN_RIGHT, TBM_SETRANGE, FALSE, MAKELPARAM(0, 255));
			SendDlgItemMessage(hDlg, IDC_TRK_GAMMA, TBM_SETRANGE, FALSE, MAKELPARAM(0, 1000));
			SendDlgItemMessage(hDlg, IDC_TRK_OUT_LEFT, TBM_SETRANGE, FALSE, MAKELPARAM(0, 255));
			SendDlgItemMessage(hDlg, IDC_TRK_OUT_RIGHT, TBM_SETRANGE, FALSE, MAKELPARAM(0, 255));

			SendDlgItemMessage(hDlg, IDC_RAD_RGB, BM_SETCHECK, BST_CHECKED, NULL);

			SendDlgItemMessage(hDlg, IDC_TRK_HUE, TBM_SETPOS, TRUE, DWORD(config.colors.active.hueShift * 360.0f));
			SendDlgItemMessage(hDlg, IDC_TRK_SAT, TBM_SETPOS, TRUE, DWORD(config.colors.active.saturation * 1000.0f));
			SendDlgItemMessage(hDlg, IDC_TRK_IN_LEFT, TBM_SETPOS, TRUE, DWORD(config.colors.active.input.left.rgb * 255.0f));
			SendDlgItemMessage(hDlg, IDC_TRK_IN_RIGHT, TBM_SETPOS, TRUE, DWORD(config.colors.active.input.right.rgb * 255.0f));
			SendDlgItemMessage(hDlg, IDC_TRK_GAMMA, TBM_SETPOS, TRUE, DWORD(config.colors.active.gamma.rgb * 1000.0f));
			SendDlgItemMessage(hDlg, IDC_TRK_OUT_LEFT, TBM_SETPOS, TRUE, DWORD(config.colors.active.output.left.rgb * 255.0f));
			SendDlgItemMessage(hDlg, IDC_TRK_OUT_RIGHT, TBM_SETPOS, TRUE, DWORD(config.colors.active.output.right.rgb * 255.0f));

			CHAR text[16];
			FLOAT val = 360.0f * config.colors.active.hueShift - 180.0f;
			StrPrint(text, val ? "%+0.f" : "%0.f", val);
			SendDlgItemMessage(hDlg, IDC_LBL_HUE, WM_SETTEXT, NULL, (WPARAM)text);

			StrPrint(text, "%.2f", MathPower(2.0f * config.colors.active.saturation, 1.5849625007211561f));
			SendDlgItemMessage(hDlg, IDC_LBL_SAT, WM_SETTEXT, NULL, (WPARAM)text);

			StrPrint(text, "%0.f", 255.0f * config.colors.active.input.left.rgb);
			SendDlgItemMessage(hDlg, IDC_LBL_IN_LEFT, WM_SETTEXT, NULL, (WPARAM)text);

			StrPrint(text, "%0.f", 255.0f * config.colors.active.input.right.rgb);
			SendDlgItemMessage(hDlg, IDC_LBL_IN_RIGHT, WM_SETTEXT, NULL, (WPARAM)text);

			StrPrint(text, "%.2f", MathPower(2.0f * config.colors.active.gamma.rgb, 3.32f));
			SendDlgItemMessage(hDlg, IDC_LBL_GAMMA, WM_SETTEXT, NULL, (WPARAM)text);

			StrPrint(text, "%0.f", 255.0f * config.colors.active.output.left.rgb);
			SendDlgItemMessage(hDlg, IDC_LBL_OUT_LEFT, WM_SETTEXT, NULL, (WPARAM)text);

			StrPrint(text, "%0.f", 255.0f * config.colors.active.output.right.rgb);
			SendDlgItemMessage(hDlg, IDC_LBL_OUT_RIGHT, WM_SETTEXT, NULL, (WPARAM)text);

			LevelsData* levelsData = (LevelsData*)MemoryAlloc(sizeof(LevelsData));
			if (levelsData)
			{
				SetWindowLong(hDlg, GWLP_USERDATA, (LONG)levelsData);
				MemoryZero(levelsData, sizeof(LevelsData));
				levelsData->delta = 0.7f;
				levelsData->values = config.colors.active;

				OpenDraw* ddraw = Main::FindOpenDrawByWindow(GetParent(hDlg));
				if (ddraw && ddraw->attachedSurface && ddraw->attachedSurface->indexBuffer)
				{
					OpenDrawSurface* surface = ddraw->attachedSurface;
					StateBufferAligned* buffer = (StateBufferAligned*)surface->indexBuffer;

					Size size;
					if (buffer->borders != BordersNone)
					{
						BOOL wide = config.battle.active && config.battle.wide;

						if (wide)
							size = { WIDE_WIDTH, WIDE_HEIGHT };
						else
							size = { *(DWORD*)&GAME_WIDTH, *(DWORD*)&GAME_HEIGHT };
					}
					else
						size = *(Size*)&surface->mode;

					DWORD left = (surface->mode.width - size.width) >> 1;
					DWORD top = (surface->mode.height - size.height) >> 1;
					DWORD pitch = surface->mode.width;

					LevelColors levels[256];
					MemoryZero(levels, sizeof(levels));

					if (config.bpp32Hooked)
					{
						DWORD* data = (DWORD*)surface->indexBuffer->data + top * pitch + left;
						pitch -= size.width;

						DWORD height = size.height;
						do
						{
							DWORD width = size.width;
							do
							{
								BYTE* b = (BYTE*)data++;
								++levels[*b++].red;
								++levels[*b++].green;
								++levels[*b].blue;
							} while (--width);

							data += pitch;
						} while (--height);
					}
					else
					{
						WORD* data = (WORD*)surface->indexBuffer->data + top * pitch + left;
						pitch -= size.width;

						DWORD height = size.height;
						do
						{
							DWORD width = size.width;
							do
							{
								WORD p = *data++;
								DWORD px = ((p & 0xF800) >> 8) | ((p & 0x07E0) << 5) | ((p & 0x001F) << 19);

								BYTE* b = (BYTE*)&px;
								++levels[*b++].red;
								++levels[*b++].green;
								++levels[*b].blue;
							} while (--width);

							data += pitch;
						} while (--height);
					}

					levelsData->hDc = CreateCompatibleDC(NULL);
					if (levelsData->hDc)
					{
						HWND hImg = GetDlgItem(hDlg, IDC_CANVAS);
						RECT rc;
						GetClientRect(hImg, &rc);

						BITMAPINFO bmi;
						MemoryZero(&bmi, sizeof(BITMAPINFO));
						bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
						bmi.bmiHeader.biWidth = rc.right;
						bmi.bmiHeader.biHeight = 100;
						bmi.bmiHeader.biPlanes = 1;
						bmi.bmiHeader.biBitCount = 32;
						bmi.bmiHeader.biXPelsPerMeter = 1;
						bmi.bmiHeader.biYPelsPerMeter = 1;

						levelsData->hBmp = CreateDIBSection(levelsData->hDc, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, (VOID**)&levelsData->data, NULL, 0);
						if (levelsData->hBmp)
						{
							SelectObject(levelsData->hDc, levelsData->hBmp);

							DWORD total = size.width * size.height;

							LevelColors* src = levels;
							LevelColorsFloat* dst = levelsData->colors;
							DWORD count = 256;
							do
							{
								dst->red = (FLOAT)src->red / total;
								dst->green = (FLOAT)src->green / total;
								dst->blue = (FLOAT)src->blue / total;

								++src;
								++dst;
							} while (--count);
						}
					}
				}
			}

			EnumChildWindows(hDlg, EnumChildProc, NULL);
			UpdateWindow(hDlg);
			break;
		}

		case WM_NOTIFY: {
			if (((NMHDR*)lParam)->code == NM_CUSTOMDRAW)
			{
				switch (wParam)
				{
				case IDC_TRK_HUE:
				case IDC_TRK_SAT:
				case IDC_TRK_IN_LEFT:
				case IDC_TRK_IN_RIGHT:
				case IDC_TRK_GAMMA:
				case IDC_TRK_OUT_LEFT:
				case IDC_TRK_OUT_RIGHT: {
					NMCUSTOMDRAW* lpDraw = (NMCUSTOMDRAW*)lParam;

					switch (lpDraw->dwDrawStage)
					{
					case CDDS_PREPAINT: {
						SetWindowLong(hDlg, DWL_MSGRESULT, CDRF_NOTIFYITEMDRAW);
						return CDRF_NOTIFYITEMDRAW;
					}

					case CDDS_ITEMPREPAINT: {

						switch (lpDraw->dwItemSpec)
						{
						case TBCD_THUMB: {
							lpDraw->rc.left += 1;
							lpDraw->rc.top += 2;
							lpDraw->rc.right -= 1;
							lpDraw->rc.bottom -= 2;
							break;
						}

						default:
							break;
						}

						return CDRF_DODEFAULT;
					}

					default:
						break;
					}

					break;
				}

				default:
					break;
				}
			}

			break;
		}

		case WM_REDRAW_CANVAS: {
			HWND hImg = GetDlgItem(hDlg, IDC_CANVAS);

			RECT rc;
			GetWindowRect(hImg, &rc);

			POINT pt = *(POINT*)&rc;
			ScreenToClient(hDlg, &pt);

			rc = { pt.x, pt.y, pt.x + (rc.right - rc.left), pt.y + (rc.bottom - rc.top) };
			InvalidateRect(hDlg, &rc, FALSE);
			return NULL;
		}

		case WM_DRAWITEM: {
			if (wParam == IDC_CANVAS)
			{
				DRAWITEMSTRUCT* paint = (DRAWITEMSTRUCT*)lParam;

				LevelsData* levelsData = (LevelsData*)GetWindowLong(hDlg, GWLP_USERDATA);
				if (levelsData)
				{
					LevelColorsFloat prep[260];
					{
						FLOAT h = 2.0f * config.colors.active.hueShift - 1.0f;
						FLOAT s = (FLOAT)MathPower(2.0f * config.colors.active.saturation, 1.5849625007211561f);

						FLOAT vsu = s * (FLOAT)MathCosinus(h * M_PI);
						FLOAT vsw = s * (FLOAT)MathSinus(h * M_PI);

						LevelColorsFloat mat[3] = {
							{ 0.299f + 0.701f * vsu + 0.168f * vsw,
								0.587f - 0.587f * vsu + 0.330f * vsw,
								0.114f - 0.114f * vsu - 0.497f * vsw },
							{ 0.299f - 0.299f * vsu - 0.328f * vsw,
								0.587f + 0.413f * vsu + 0.035f * vsw,
								0.114f - 0.114f * vsu + 0.292f * vsw },
							{ 0.299f - 0.300f * vsu + 1.25f * vsw,
								0.587f - 0.588f * vsu - 1.05f * vsw,
								0.114f + 0.886f * vsu - 0.203f * vsw }
						};

						MemoryZero(prep, sizeof(prep));

						struct {
							Levels input;
							Levels gamma;
							Levels output;
						} levels;

						for (DWORD i = 0; i < 4; ++i)
						{
							levels.input.chanel[i] = config.colors.active.input.right.chanel[i] - config.colors.active.input.left.chanel[i];
							levels.gamma.chanel[i] = 1.0f / (FLOAT)MathPower(2.0f * config.colors.active.gamma.chanel[i], 3.32f);
							levels.output.chanel[i] = config.colors.active.output.right.chanel[i] - config.colors.active.output.left.chanel[i];
						}

						LevelColorsFloat* src = levelsData->colors;
						for (DWORD i = 0; i < 256; ++i, ++src)
						{
							FLOAT dx = (FLOAT)i / 255.0f;

							LevelColorsFloat* mt = mat;
							for (DWORD j = 0; j < 3; ++j, ++mt)
							{
								FLOAT k = dx;
								for (DWORD s = 2, idx = j + 1; s; --s, idx = 0)
								{
									k = (k - config.colors.active.input.left.chanel[idx]) / levels.input.chanel[idx];
									k = min(1.0f, max(0.0f, k));
									k = (FLOAT)MathPower(k, levels.gamma.chanel[idx]);
									k = k * levels.output.chanel[idx] + config.colors.active.output.left.chanel[idx];
									k = min(1.0f, max(0.0f, k));
								}

								prep[(DWORD)(k * 255.0f) + 2].chanel[j] += mt->red * src->red + mt->green * src->green + mt->blue * src->blue;
							}
						}
					}

					prep[1] = prep[2];
					prep[258] = prep[257];

					LevelColorsFloat floats[259];
					for (DWORD y = 4; y; --y)
					{
						{
							LevelColorsFloat* src = prep;
							LevelColorsFloat* dst = floats + 1;
							DWORD count = 257;
							do
							{
								for (DWORD i = 0; i < 3; ++i)
									dst->chanel[i] = FLOAT((src[0].chanel[i] + src[3].chanel[i]) * (0.125 / 6.0) + (src[1].chanel[i] + src[2].chanel[i]) * (2.875 / 6.0));

								++src;
								++dst;
							} while (--count);
						}
						floats[0] = floats[1];
						floats[258] = floats[257];

						{
							LevelColorsFloat* src = floats;
							LevelColorsFloat* dst = prep + 2;
							DWORD count = 256;
							do
							{
								for (DWORD i = 0; i < 3; ++i)
									dst->chanel[i] = FLOAT((src[0].chanel[i] + src[3].chanel[i]) * (0.125 / 6.0) + (src[1].chanel[i] + src[2].chanel[i]) * (2.875 / 6.0));

								++src;
								++dst;
							} while (--count);
						}
						prep[1] = prep[2];
						prep[258] = prep[257];
					}

					{
						LevelColorsFloat* src = prep;
						LevelColorsFloat* dst = floats + 1;
						DWORD count = 257;
						do
						{
							for (DWORD i = 0; i < 3; ++i)
								dst->chanel[i] = min(1.0f, max(0.0f, FLOAT((src[0].chanel[i] + src[3].chanel[i]) * (0.125 / 6.0) + (src[1].chanel[i] + src[2].chanel[i]) * (2.875 / 6.0))));

							++src;
							++dst;
						} while (--count);
					}

					FLOAT max = 0.0;
					{
						FLOAT* data = (FLOAT*)(floats + 1);
						DWORD count = 257 * 3;
						do
						{
							*data = (FLOAT)MathPower(*data, levelsData->delta);
							max += *data++;
						} while (--count);
					}

					if (max > 0.0)
					{
						LevelColors levels[259];
						MemoryZero(levels, sizeof(levels));

						{
							max /= FLOAT(256 / 4 * 3);

							FLOAT* src = (FLOAT*)(floats + 1);
							DWORD* dst = (DWORD*)(levels + 1);
							DWORD count = 257 * 3;
							do
								*dst++ = DWORD(*src++ / max * 100.0f);
							while (--count);

							levels[0] = levels[1];
							levels[258] = levels[257];
						}

						{
							MemoryZero(levelsData->bmpData, sizeof(levelsData->bmpData));

							INT index;
							struct {
								DWORD line;
								DWORD back;
							} light, dark;

							if (SendDlgItemMessage(hDlg, IDC_RAD_RED, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
							{
								index = 0;
								dark = { 0xA0, 0x30 };
								light = { 0xFF, 0xC0 };
							}
							else if (SendDlgItemMessage(hDlg, IDC_RAD_GREEN, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
							{
								index = 1;
								dark = { 0xA0, 0x30 };
								light = { 0xFF, 0xC0 };
							}
							else if (SendDlgItemMessage(hDlg, IDC_RAD_BLUE, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
							{
								index = 2;
								dark = { 0xA0, 0x30 };
								light = { 0xFF, 0xC0 };
							}
							else
							{
								index = -1;
								dark = { 0xD0, 0x50 };
							}

							DWORD* dst = levelsData->bmpData + 1;
							for (DWORD i = 0; i < 100; ++i)
							{
								LevelColors* src = levels + 1;
								DWORD count = 257;
								do
								{
									LevelColors* neighbor = src - 1;
									DWORD iter = 2;
									do
									{
										DWORD pix = 0;
										for (DWORD j = 0, x = 16; j < 3; ++j, x -= 8)
										{
											if (j != index)
											{
												if (i < src->chanel[j])
												{
													DWORD min = neighbor->chanel[j] + 1;
													if (min > src->chanel[j])
														min = src->chanel[j];

													pix |= (i + 1 >= min ? dark.line : dark.back) << x;
												}
												else
													pix |= 0x20 << x;
											}
										}

										if (index + 1)
										{
											DWORD shift = (2 - index) * 8;
											if (i < src->chanel[index])
											{
												DWORD min = neighbor->chanel[index] + 1;
												if (min > src->chanel[index])
													min = src->chanel[index];

												if (i + 1 >= min)
													pix = light.line << shift;
												else
													pix |= light.back << shift;
											}
											else
												pix |= 0x20 << shift;
										}

										*dst++ = pix;
										neighbor = src + 1;
									} while (--iter);

									++src;
								} while (--count);

								dst += 2;
							}

							HWND hImg = GetDlgItem(hDlg, IDC_CANVAS);

							RECT rc;
							GetClientRect(hImg, &rc);

							for (LONG i = 0; i < rc.right; ++i)
							{
								FLOAT pos = (FLOAT)i / rc.right * 514.0f;
								DWORD index = DWORD(pos);
								pos -= (FLOAT)index;

								DWORD* dest = levelsData->data + i;
								for (DWORD j = 0; j < 100; ++j)
								{
									BYTE* src = (BYTE*)&levelsData->bmpData[j * 516 + index];
									BYTE* dst = (BYTE*)dest;

									for (DWORD j = 0; j < 3; ++j, ++src)
										dst[j] = CubicInterpolate(src[0], src[4], src[8], src[12], pos);

									dest += rc.right;
								}
							}

							if (levelsData->hBmp)
								BitBlt(paint->hDC, 0, 0, rc.right, 100, levelsData->hDc, 0, 0, SRCCOPY);
						}

						return TRUE;
					}
				}

				FillRect(paint->hDC, &paint->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
				return TRUE;
			}

			break;
		}

		case WM_COMMAND: {
			switch (wParam)
			{
			case IDC_BTN_OK: {
				LevelsData* levelsData = (LevelsData*)GetWindowLong(hDlg, GWLP_USERDATA);
				if (levelsData)
					levelsData->values = config.colors.active;

				Config::Set(CONFIG_COLORS, "HueSat", MAKELONG(DWORD(config.colors.active.hueShift * 1000.0f), DWORD(config.colors.active.saturation * 1000.0f)));
				Config::Set(CONFIG_COLORS, "RgbInput", MAKELONG(DWORD(config.colors.active.input.left.rgb * 1000.0f), DWORD(config.colors.active.input.right.rgb * 1000.0f)));
				Config::Set(CONFIG_COLORS, "RedInput", MAKELONG(DWORD(config.colors.active.input.left.red * 1000.0f), DWORD(config.colors.active.input.right.red * 1000.0f)));
				Config::Set(CONFIG_COLORS, "GreenInput", MAKELONG(DWORD(config.colors.active.input.left.green * 1000.0f), DWORD(config.colors.active.input.right.green * 1000.0f)));
				Config::Set(CONFIG_COLORS, "BlueInput", MAKELONG(DWORD(config.colors.active.input.left.blue * 1000.0f), DWORD(config.colors.active.input.right.blue * 1000.0f)));
				Config::Set(CONFIG_COLORS, "RgbGamma", DWORD(config.colors.active.gamma.rgb * 1000.0f));
				Config::Set(CONFIG_COLORS, "RedGamma", DWORD(config.colors.active.gamma.red * 1000.0f));
				Config::Set(CONFIG_COLORS, "GreenGamma", DWORD(config.colors.active.gamma.green * 1000.0f));
				Config::Set(CONFIG_COLORS, "BlueGamma", DWORD(config.colors.active.gamma.blue * 1000.0f));
				Config::Set(CONFIG_COLORS, "RgbOutput", MAKELONG(DWORD(config.colors.active.output.left.rgb * 1000.0f), DWORD(config.colors.active.output.right.rgb * 1000.0f)));
				Config::Set(CONFIG_COLORS, "RedOutput", MAKELONG(DWORD(config.colors.active.output.left.red * 1000.0f), DWORD(config.colors.active.output.right.red * 1000.0f)));
				Config::Set(CONFIG_COLORS, "GreenOutput", MAKELONG(DWORD(config.colors.active.output.left.green * 1000.0f), DWORD(config.colors.active.output.right.green * 1000.0f)));
				Config::Set(CONFIG_COLORS, "BlueOutput", MAKELONG(DWORD(config.colors.active.output.left.blue * 1000.0f), DWORD(config.colors.active.output.right.blue * 1000.0f)));
			}

			case IDC_BTN_CANCEL: {
				EndDialog(hDlg, TRUE);
				return NULL;
			}

			case IDC_BTN_RESET: {
				config.colors.active.hueShift = 0.5f;
				config.colors.active.saturation = 0.5f;

				for (DWORD i = 0; i < 4; ++i)
				{
					config.colors.active.input.left.chanel[i] = 0.0f;
					config.colors.active.input.right.chanel[i] = 1.0f;
					config.colors.active.gamma.chanel[i] = 0.5f;
					config.colors.active.output.left.chanel[i] = 0.0f;
					config.colors.active.output.right.chanel[i] = 1.0f;
				}

				SendDlgItemMessage(hDlg, IDC_RAD_BLUE, BM_SETCHECK, BST_UNCHECKED, NULL);
				SendDlgItemMessage(hDlg, IDC_RAD_GREEN, BM_SETCHECK, BST_UNCHECKED, NULL);
				SendDlgItemMessage(hDlg, IDC_RAD_RED, BM_SETCHECK, BST_UNCHECKED, NULL);
				SendDlgItemMessage(hDlg, IDC_RAD_RGB, BM_SETCHECK, BST_CHECKED, NULL);

				SendDlgItemMessage(hDlg, IDC_TRK_HUE, TBM_SETPOS, TRUE, 180);
				SendDlgItemMessage(hDlg, IDC_TRK_SAT, TBM_SETPOS, TRUE, 500);
				SendDlgItemMessage(hDlg, IDC_TRK_IN_LEFT, TBM_SETPOS, TRUE, 0);
				SendDlgItemMessage(hDlg, IDC_TRK_IN_RIGHT, TBM_SETPOS, TRUE, 255);
				SendDlgItemMessage(hDlg, IDC_TRK_GAMMA, TBM_SETPOS, TRUE, 500);
				SendDlgItemMessage(hDlg, IDC_TRK_OUT_LEFT, TBM_SETPOS, TRUE, 0);
				SendDlgItemMessage(hDlg, IDC_TRK_OUT_RIGHT, TBM_SETPOS, TRUE, 255);

				SendDlgItemMessage(hDlg, IDC_LBL_HUE, WM_SETTEXT, NULL, (WPARAM) "0");
				SendDlgItemMessage(hDlg, IDC_LBL_SAT, WM_SETTEXT, NULL, (WPARAM) "1.00");
				SendDlgItemMessage(hDlg, IDC_LBL_IN_LEFT, WM_SETTEXT, NULL, (WPARAM) "0");
				SendDlgItemMessage(hDlg, IDC_LBL_IN_RIGHT, WM_SETTEXT, NULL, (WPARAM) "255");
				SendDlgItemMessage(hDlg, IDC_LBL_GAMMA, WM_SETTEXT, NULL, (WPARAM) "1.00");
				SendDlgItemMessage(hDlg, IDC_LBL_OUT_LEFT, WM_SETTEXT, NULL, (WPARAM) "0");
				SendDlgItemMessage(hDlg, IDC_LBL_OUT_RIGHT, WM_SETTEXT, NULL, (WPARAM) "255");

				SendMessage(hDlg, WM_REDRAW_CANVAS, NULL, NULL);
				OpenDraw* ddraw = Main::FindOpenDrawByWindow(GetParent(hDlg));
				if (ddraw)
					ddraw->Redraw();

				return NULL;
			}

			case IDC_BTN_AUTO: {
				config.colors.active.hueShift = 0.5f;
				config.colors.active.saturation = 0.5f;

				for (DWORD i = 0; i < 4; ++i)
				{
					config.colors.active.input.left.chanel[i] = 0.0f;
					config.colors.active.input.right.chanel[i] = 1.0f;
					config.colors.active.gamma.chanel[i] = 0.5f;
					config.colors.active.output.left.chanel[i] = 0.0f;
					config.colors.active.output.right.chanel[i] = 1.0f;
				}

				SendDlgItemMessage(hDlg, IDC_RAD_BLUE, BM_SETCHECK, BST_UNCHECKED, NULL);
				SendDlgItemMessage(hDlg, IDC_RAD_GREEN, BM_SETCHECK, BST_UNCHECKED, NULL);
				SendDlgItemMessage(hDlg, IDC_RAD_RED, BM_SETCHECK, BST_UNCHECKED, NULL);
				SendDlgItemMessage(hDlg, IDC_RAD_RGB, BM_SETCHECK, BST_CHECKED, NULL);

				SendDlgItemMessage(hDlg, IDC_TRK_HUE, TBM_SETPOS, TRUE, 180);
				SendDlgItemMessage(hDlg, IDC_TRK_SAT, TBM_SETPOS, TRUE, 500);
				SendDlgItemMessage(hDlg, IDC_TRK_IN_LEFT, TBM_SETPOS, TRUE, 0);
				SendDlgItemMessage(hDlg, IDC_TRK_IN_RIGHT, TBM_SETPOS, TRUE, 255);
				SendDlgItemMessage(hDlg, IDC_TRK_GAMMA, TBM_SETPOS, TRUE, 500);
				SendDlgItemMessage(hDlg, IDC_TRK_OUT_LEFT, TBM_SETPOS, TRUE, 0);
				SendDlgItemMessage(hDlg, IDC_TRK_OUT_RIGHT, TBM_SETPOS, TRUE, 255);

				SendDlgItemMessage(hDlg, IDC_LBL_HUE, WM_SETTEXT, NULL, (WPARAM) "0");
				SendDlgItemMessage(hDlg, IDC_LBL_SAT, WM_SETTEXT, NULL, (WPARAM) "1.00");
				SendDlgItemMessage(hDlg, IDC_LBL_IN_LEFT, WM_SETTEXT, NULL, (WPARAM) "0");
				SendDlgItemMessage(hDlg, IDC_LBL_IN_RIGHT, WM_SETTEXT, NULL, (WPARAM) "255");
				SendDlgItemMessage(hDlg, IDC_LBL_GAMMA, WM_SETTEXT, NULL, (WPARAM) "1.00");
				SendDlgItemMessage(hDlg, IDC_LBL_OUT_LEFT, WM_SETTEXT, NULL, (WPARAM) "0");
				SendDlgItemMessage(hDlg, IDC_LBL_OUT_RIGHT, WM_SETTEXT, NULL, (WPARAM) "255");

				LevelsData* levelsData = (LevelsData*)GetWindowLong(hDlg, GWLP_USERDATA);
				if (levelsData)
				{
					{
						LevelColorsFloat found = { 0.0, 0.0, 0.0 };
						BOOL success[3] = { FALSE, FALSE, FALSE };
						DWORD exit = 0;
						LevelColorsFloat* data = levelsData->colors;

						for (DWORD i = 0; i < 256; ++i, ++data)
						{
							for (DWORD j = 0; j < 3; ++j)
							{
								if (!success[j])
								{
									found.chanel[j] += data->chanel[j];
									if (found.chanel[j] > 0.007f)
									{
										success[j] = TRUE;
										++exit;
										config.colors.active.input.left.chanel[j + 1] = (FLOAT)i / 255.0f;
									}
								}
							}

							if (exit == 3)
								break;
						};
					}

					{
						LevelColorsFloat found = { 0.0, 0.0, 0.0 };
						BOOL success[3] = { FALSE, FALSE, FALSE };
						DWORD exit = 0;
						LevelColorsFloat* data = &levelsData->colors[255];

						for (DWORD i = 255; i >= 0; --i, --data)
						{
							for (DWORD j = 0; j < 3; ++j)
							{
								if (!success[j])
								{
									found.chanel[j] += data->chanel[j];
									if (found.chanel[j] > 0.007f)
									{
										success[j] = TRUE;
										++exit;
										config.colors.active.input.right.chanel[j + 1] = (FLOAT)i / 255.0f;
									}
								}
							}

							if (exit == 3)
								break;
						};
					}
				}

				SendMessage(hDlg, WM_REDRAW_CANVAS, NULL, NULL);
				OpenDraw* ddraw = Main::FindOpenDrawByWindow(GetParent(hDlg));
				if (ddraw)
					ddraw->Redraw();

				return NULL;
			}

			case IDC_RAD_RGB:
			case IDC_RAD_RED:
			case IDC_RAD_GREEN:
			case IDC_RAD_BLUE: {
				DWORD index = (DWORD)wParam - IDC_RAD_RGB;

				SendDlgItemMessage(hDlg, IDC_TRK_IN_LEFT, TBM_SETPOS, TRUE, DWORD(config.colors.active.input.left.chanel[index] * 255.0f));
				SendDlgItemMessage(hDlg, IDC_TRK_IN_RIGHT, TBM_SETPOS, TRUE, DWORD(config.colors.active.input.right.chanel[index] * 255.0f));
				SendDlgItemMessage(hDlg, IDC_TRK_GAMMA, TBM_SETPOS, TRUE, DWORD(config.colors.active.gamma.chanel[index] * 1000.0f));
				SendDlgItemMessage(hDlg, IDC_TRK_OUT_LEFT, TBM_SETPOS, TRUE, DWORD(config.colors.active.output.left.chanel[index] * 255.0f));
				SendDlgItemMessage(hDlg, IDC_TRK_OUT_RIGHT, TBM_SETPOS, TRUE, DWORD(config.colors.active.output.right.chanel[index] * 255.0f));

				CHAR text[16];

				StrPrint(text, "%0.f", 255.0f * config.colors.active.input.left.chanel[index]);
				SendDlgItemMessage(hDlg, IDC_LBL_IN_LEFT, WM_SETTEXT, NULL, (WPARAM)text);

				StrPrint(text, "%0.f", 255.0f * config.colors.active.input.right.chanel[index]);
				SendDlgItemMessage(hDlg, IDC_LBL_IN_RIGHT, WM_SETTEXT, NULL, (WPARAM)text);

				StrPrint(text, "%.2f", MathPower(2.0f * config.colors.active.gamma.chanel[index], 3.32f));
				SendDlgItemMessage(hDlg, IDC_LBL_GAMMA, WM_SETTEXT, NULL, (WPARAM)text);

				StrPrint(text, "%0.f", 255.0f * config.colors.active.output.left.chanel[index]);
				SendDlgItemMessage(hDlg, IDC_LBL_OUT_LEFT, WM_SETTEXT, NULL, (WPARAM)text);

				StrPrint(text, "%0.f", 255.0f * config.colors.active.output.right.chanel[index]);
				SendDlgItemMessage(hDlg, IDC_LBL_OUT_RIGHT, WM_SETTEXT, NULL, (WPARAM)text);

				SendMessage(hDlg, WM_REDRAW_CANVAS, NULL, NULL);
				return NULL;
			}

			default:
				break;
			}

			break;
		}

		case WM_MOUSEWHEEL: {
			HWND hImg = GetDlgItem(hDlg, IDC_CANVAS);

			RECT rc;
			GetClientRect(hImg, &rc);

			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			ScreenToClient(hImg, &pt);
			if (PtInRect(&rc, pt))
			{
				LevelsData* levelsData = (LevelsData*)GetWindowLong(hDlg, GWLP_USERDATA);
				if (levelsData)
				{
					FLOAT dlt = levelsData->delta + 0.025f * GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
					if (dlt > 0.0f && dlt < 1.0f)
					{
						levelsData->delta = dlt;
						SendMessage(hDlg, WM_REDRAW_CANVAS, NULL, NULL);
					}
				}
			}

			break;
		}

		case WM_HSCROLL: {
			DWORD value = LOWORD(wParam) == SB_THUMBTRACK || LOWORD(wParam) == SB_THUMBPOSITION ? HIWORD(wParam) : SendMessage((HWND)lParam, TBM_GETPOS, NULL, NULL);
			CHAR text[16];
			INT id = GetDlgCtrlID((HWND)lParam);
			switch (id)
			{
			case IDC_TRK_HUE: {
				config.colors.active.hueShift = (FLOAT)value / 360.0f;

				DWORD val = value - 180;
				StrPrint(text, val ? "%+d" : "%d", val);
				SendDlgItemMessage(hDlg, IDC_LBL_HUE, WM_SETTEXT, NULL, (WPARAM)text);
				break;
			}
			case IDC_TRK_SAT: {
				config.colors.active.saturation = 0.001f * value;

				StrPrint(text, "%.2f", MathPower(0.002f * value, 1.5849625007211561f));
				SendDlgItemMessage(hDlg, IDC_LBL_SAT, WM_SETTEXT, NULL, (WPARAM)text);
				break;
			}
			case IDC_TRK_IN_LEFT: {
				DWORD idx;
				if (SendDlgItemMessage(hDlg, IDC_RAD_RED, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 1;
				else if (SendDlgItemMessage(hDlg, IDC_RAD_GREEN, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 2;
				else if (SendDlgItemMessage(hDlg, IDC_RAD_BLUE, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 3;
				else
					idx = 0;

				DWORD comp = SendDlgItemMessage(hDlg, IDC_TRK_IN_RIGHT, TBM_GETPOS, NULL, NULL);
				if (value > comp)
				{
					value = comp;
					SendDlgItemMessage(hDlg, IDC_TRK_IN_LEFT, TBM_SETPOS, TRUE, value);
				}

				StrPrint(text, "%d", value);
				SendDlgItemMessage(hDlg, IDC_LBL_IN_LEFT, WM_SETTEXT, NULL, (WPARAM)text);

				config.colors.active.input.left.chanel[idx] = (FLOAT)value / 255.0f;
				break;
			}
			case IDC_TRK_IN_RIGHT: {
				DWORD idx;
				if (SendDlgItemMessage(hDlg, IDC_RAD_RED, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 1;
				else if (SendDlgItemMessage(hDlg, IDC_RAD_GREEN, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 2;
				else if (SendDlgItemMessage(hDlg, IDC_RAD_BLUE, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 3;
				else
					idx = 0;

				DWORD comp = SendDlgItemMessage(hDlg, IDC_TRK_IN_LEFT, TBM_GETPOS, NULL, NULL);
				if (value < comp)
				{
					value = comp;
					SendDlgItemMessage(hDlg, IDC_TRK_IN_RIGHT, TBM_SETPOS, TRUE, value);
				}

				StrPrint(text, "%d", value);
				SendDlgItemMessage(hDlg, IDC_LBL_IN_RIGHT, WM_SETTEXT, NULL, (WPARAM)text);

				config.colors.active.input.right.chanel[idx] = (FLOAT)value / 255.0f;
				break;
			}
			case IDC_TRK_GAMMA: {
				DWORD idx;
				if (SendDlgItemMessage(hDlg, IDC_RAD_RED, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 1;
				else if (SendDlgItemMessage(hDlg, IDC_RAD_GREEN, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 2;
				else if (SendDlgItemMessage(hDlg, IDC_RAD_BLUE, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 3;
				else
					idx = 0;

				config.colors.active.gamma.chanel[idx] = 0.001f * value;

				StrPrint(text, "%.2f", MathPower(0.002f * value, 3.32f));
				SendDlgItemMessage(hDlg, IDC_LBL_GAMMA, WM_SETTEXT, NULL, (WPARAM)text);
				break;
			}
			case IDC_TRK_OUT_LEFT: {
				DWORD idx;
				if (SendDlgItemMessage(hDlg, IDC_RAD_RED, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 1;
				else if (SendDlgItemMessage(hDlg, IDC_RAD_GREEN, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 2;
				else if (SendDlgItemMessage(hDlg, IDC_RAD_BLUE, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 3;
				else
					idx = 0;

				StrPrint(text, "%d", value);
				SendDlgItemMessage(hDlg, IDC_LBL_OUT_LEFT, WM_SETTEXT, NULL, (WPARAM)text);

				config.colors.active.output.left.chanel[idx] = (FLOAT)value / 255.0f;
				break;
			}
			case IDC_TRK_OUT_RIGHT: {
				DWORD idx;
				if (SendDlgItemMessage(hDlg, IDC_RAD_RED, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 1;
				else if (SendDlgItemMessage(hDlg, IDC_RAD_GREEN, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 2;
				else if (SendDlgItemMessage(hDlg, IDC_RAD_BLUE, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					idx = 3;
				else
					idx = 0;

				StrPrint(text, "%d", value);
				SendDlgItemMessage(hDlg, IDC_LBL_OUT_RIGHT, WM_SETTEXT, NULL, (WPARAM)text);

				config.colors.active.output.right.chanel[idx] = (FLOAT)value / 255.0f;
				break;
			}
			default:
				break;
			}

			SendMessage(hDlg, WM_REDRAW_CANVAS, NULL, NULL);
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(GetParent(hDlg));
			if (ddraw)
				ddraw->Redraw();

			return NULL;
		}

		case WM_CLOSE: {
			EndDialog(hDlg, TRUE);
			break;
		}

		case WM_DESTROY: {
			LevelsData* levelsData = (LevelsData*)GetWindowLong(hDlg, GWLP_USERDATA);
			if (levelsData)
			{
				SetWindowLong(hDlg, GWLP_USERDATA, NULL);
				if (levelsData->hDc)
				{
					DeleteDC(levelsData->hDc);
					if (levelsData->hBmp)
						DeleteObject(levelsData->hBmp);
				}

				config.colors.active = levelsData->values;
				MemoryFree(levelsData);
			}
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
		case WM_PAINT: {
			PAINTSTRUCT paint;
			HDC hDc = BeginPaint(hWnd, &paint);
			if (hDc)
			{
				OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
				if (ddraw)
					ddraw->Redraw();

				EndPaint(hWnd, &paint);
			}

			return NULL;
		}

		case WM_ERASEBKGND:
			return TRUE;

		case WM_WINDOWPOSCHANGED: {
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
			{
				RECT rect;
				GetClientRect(hWnd, &rect);
				ddraw->viewport.width = rect.right;
				ddraw->viewport.height = rect.bottom - ddraw->viewport.offset;
				ddraw->viewport.refresh = TRUE;

				if (config.renderer != RendererGDI)
					ddraw->Redraw();
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_MOVE: {
			if (config.renderer != RendererGDI)
			{
				OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
				if (ddraw)
					ddraw->Redraw();
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_SIZE: {
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
			{
				if (ddraw->hDraw && ddraw->hDraw != hWnd)
					SetWindowPos(ddraw->hDraw, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOREPOSITION | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);

				ddraw->viewport.width = LOWORD(lParam);
				ddraw->viewport.height = HIWORD(lParam) - ddraw->viewport.offset;
				ddraw->viewport.refresh = TRUE;

				if (config.renderer != RendererGDI)
					ddraw->Redraw();
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_GETMINMAXINFO: {
			DWORD style = GetWindowLong(hWnd, GWL_STYLE);

			RECT min = { 0, 0, 240, 240 * LONG(config.mode->height / config.mode->width) };
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

		case WM_DESTROY: {
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
			{
				ddraw->RenderStop();
				ddraw->SetCooperativeLevel(NULL, NULL);
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_ACTIVATEAPP: {
			if (!config.alwaysActive)
				config.colors.current = (BOOL)wParam ? &config.colors.active : &inactiveColors;

			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
			{
				ddraw->viewport.refresh = TRUE;
				if (!config.alwaysActive)
					ddraw->Redraw();

				if (ddraw->renderer && !config.windowedMode && config.renderer != RendererGDI && !config.borderless.real)
				{
					ddraw->RenderStop();
					config.borderless.mode = !(BOOL)wParam;
					ddraw->RenderStart();
				}
			}

			return !config.alwaysActive || (BOOL)wParam ? CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam) : NULL;
		}

		case WM_SETCURSOR: {
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
		case WM_KEYDOWN: {
			if (!(HIWORD(lParam) & KF_ALTDOWN))
			{
				if (wParam == VK_OEM_PLUS || wParam == VK_OEM_MINUS || wParam == VK_ADD || wParam == VK_SUBTRACT)
				{
					DWORD index = config.speed.enabled ? config.speed.index : 0;
					DWORD oldIndex = index;
					if (wParam == VK_ADD || wParam == VK_OEM_PLUS)
						++index;
					else if (index)
						--index;

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

						Hooks::SetGameSpeed();
						CheckMenu(MenuSpeed);
					}
				}
				else if (config.keys.imageFilter && config.keys.imageFilter + VK_F1 - 1 == wParam)
				{
					switch (config.image.interpolation)
					{
					case InterpolateNearest:
						InterpolationChanged(hWnd, InterpolateLinear);
						break;

					case InterpolateLinear:
						InterpolationChanged(hWnd, config.gl.version.value >= GL_VER_2_0 ? InterpolateHermite : InterpolateNearest);
						break;

					case InterpolateHermite:
						InterpolationChanged(hWnd, config.gl.version.value >= GL_VER_2_0 ? InterpolateCubic : InterpolateNearest);
						break;

					case InterpolateCubic:
						InterpolationChanged(hWnd, config.gl.version.value >= GL_VER_2_0 ? InterpolateLanczos : InterpolateNearest);
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
				else if (config.keys.zoomImage && config.keys.zoomImage + VK_F1 - 1 == wParam)
				{
					return WindowProc(hWnd, WM_COMMAND, IDM_STRETCH_OFF + (!config.zoom.enabled ? config.zoom.value : 0), NULL);
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

					Hooks::SetGameSpeed();
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
		case WM_MBUTTONUP: {
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
			{
				POINT p = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				ddraw->ScaleMouse(&p);
				lParam = MAKELONG(p.x, p.y);
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_COMMAND: {
			switch (wParam)
			{
			case IDM_FILE_RESET: {
				if (Main::ShowWarn(IDS_WARN_RESET))
				{
					Config::Set(CONFIG_WRAPPER, "ReInit", TRUE);
					Main::ShowInfo(IDS_INFO_RESET);
				}

				return NULL;
			}

			case IDM_FILE_QUIT: {
				SendMessage(hWnd, WM_CLOSE, NULL, NULL);
				return NULL;
			}

			case IDM_HELP_ABOUT_APPLICATION:
			case IDM_HELP_ABOUT_WRAPPER: {

				DialogParams params = { hWnd, TRUE, NULL };
				BeginDialog(&params);
				{
					LPARAM id;
					DLGPROC proc;
					if (wParam == IDM_HELP_ABOUT_APPLICATION)
					{
						id = params.cookie ? IDD_ABOUT_APPLICATION : IDD_ABOUT_APPLICATION_OLD;
						proc = (DLGPROC)AboutApplicationProc;
					}
					else
					{
						id = params.cookie ? IDD_ABOUT_WRAPPER : IDD_ABOUT_WRAPPER_OLD;
						proc = (DLGPROC)AboutWrapperProc;
					}
					DialogBoxParam(hDllModule, MAKEINTRESOURCE(id), hWnd, proc, id);
				}
				EndDialog(&params);
				return NULL;
			}

			case IDM_RES_FULL_SCREEN: {
				OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
				if (ddraw && ddraw->renderer)
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

			case IDM_ASPECT_RATIO: {
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
					ddraw->Redraw();
				}

				CheckMenu(MenuAspect);

				return NULL;
			}

			case IDM_VSYNC: {
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
					ddraw->Redraw();

				CheckMenu(MenuVSync);

				return NULL;
			}

			case IDM_FILT_OFF: {
				InterpolationChanged(hWnd, InterpolateNearest);
				return NULL;
			}

			case IDM_FILT_LINEAR: {
				InterpolationChanged(hWnd, InterpolateLinear);
				return NULL;
			}

			case IDM_FILT_HERMITE: {
				InterpolationChanged(hWnd, InterpolateHermite);
				return NULL;
			}

			case IDM_FILT_CUBIC: {
				InterpolationChanged(hWnd, InterpolateCubic);
				return NULL;
			}

			case IDM_FILT_LANCZOS: {
				InterpolationChanged(hWnd, InterpolateLanczos);
				return NULL;
			}

			case IDM_FILT_NONE: {
				UpscalingChanged(hWnd, UpscaleNone);
				return NULL;
			}

			case IDM_FILT_SCALENX_2X: {
				SelectScaleNxMode(hWnd, 2);
				return NULL;
			}

			case IDM_FILT_SCALENX_3X: {
				SelectScaleNxMode(hWnd, 3);
				return NULL;
			}

			case IDM_FILT_XSAL_2X: {
				SelectXSalMode(hWnd, 2);
				return NULL;
			}

			case IDM_FILT_EAGLE_2X: {
				SelectEagleMode(hWnd, 2);
				return NULL;
			}

			case IDM_FILT_SCALEHQ_2X: {
				SelectScaleHQMode(hWnd, 2);
				return NULL;
			}

			case IDM_FILT_SCALEHQ_4X: {
				SelectScaleHQMode(hWnd, 4);
				return NULL;
			}

			case IDM_FILT_XRBZ_2X: {
				SelectXBRZMode(hWnd, 2);
				return NULL;
			}

			case IDM_FILT_XRBZ_3X: {
				SelectXBRZMode(hWnd, 3);
				return NULL;
			}

			case IDM_FILT_XRBZ_4X: {
				SelectXBRZMode(hWnd, 4);
				return NULL;
			}

			case IDM_FILT_XRBZ_5X: {
				SelectXBRZMode(hWnd, 5);
				return NULL;
			}

			case IDM_FILT_XRBZ_6X: {
				SelectXBRZMode(hWnd, 6);
				return NULL;
			}

			case IDM_BACKGROUND: {
				if (config.background.allowed)
				{
					config.background.enabled = !config.background.enabled;
					Config::Set(CONFIG_WRAPPER, "Background", config.background.enabled);
					Config::Set(CONFIG_DISCIPLE, "ShowInterfBorder", config.background.enabled);

					{
						CHAR str1[32];
						LoadString(hDllModule, IDS_BACKGROUND, str1, sizeof(str1));

						CHAR str2[32];
						LoadString(hDllModule, config.background.enabled ? IDS_ON : IDS_OFF, str2, sizeof(str2));

						CHAR text[64];
						StrPrint(text, "%s: %s", str1, str2);
						Hooks::PrintText(text);
					}

					OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
					if (ddraw)
						ddraw->Redraw();

					CheckMenu(MenuBackground);
				}

				return NULL;
			}

			case IDM_COLOR_ADJUST: {
				DialogParams params = { hWnd, FALSE, NULL };
				BeginDialog(&params);
				{
					DialogBox(hDllModule, MAKEINTRESOURCE(IDD_COLOR_ADJUSTMENT), hWnd, (DLGPROC)ColorAdjustmentProc);
				}
				EndDialog(&params);
				return NULL;
			}

			case IDM_MODE_BORDERLESS: {
				config.borderless.real = config.borderless.mode = TRUE;
				Config::Set(CONFIG_WRAPPER, "BorderlessMode", config.borderless.mode);
				CheckMenu(MenuWindowType);
				return NULL;
			}

			case IDM_MODE_EXCLUSIVE: {
				config.borderless.real = config.borderless.mode = FALSE;
				Config::Set(CONFIG_WRAPPER, "BorderlessMode", config.borderless.mode);
				CheckMenu(MenuWindowType);
				return NULL;
			}

			case IDM_FAST_AI: {
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

			case IDM_ALWAYS_ACTIVE: {
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

			case IDM_PATCH_CPU: {
				config.coldCPU = !config.coldCPU;
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

				CheckMenu(MenuCpu);
				return NULL;
			}

			case IDM_MODE_WIDEBATTLE: {
				config.wide.allowed = !config.wide.allowed;
				Config::Set(CONFIG_WRAPPER, "WideBattle", config.wide.allowed);

				Main::ShowInfo(IDS_INFO_NEXT_BATTLE);

				CheckMenu(MenuBattle);
				return NULL;
			}

			case IDM_SCR_BMP:
			case IDM_SCR_PNG: {
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

					CheckMenu(MenuSnapshot);
				}

				return NULL;
			}

			case IDM_REND_AUTO: {
				SelectRenderer(hWnd, RendererAuto);
				return NULL;
			}

			case IDM_REND_GL1: {
				SelectRenderer(hWnd, RendererOpenGL1);
				return NULL;
			}

			case IDM_REND_GL2: {
				SelectRenderer(hWnd, RendererOpenGL2);
				return NULL;
			}

			case IDM_REND_GL3: {
				SelectRenderer(hWnd, RendererOpenGL3);
				return NULL;
			}

			case IDM_REND_GDI: {
				SelectRenderer(hWnd, RendererGDI);
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
					if (config.zoom.allowed)
					{
						DWORD value = wParam - IDM_STRETCH_OFF;

						if (value)
						{
							config.zoom.enabled = TRUE;

							config.zoom.value = value;
							Config::Set(CONFIG_WRAPPER, "ZoomFactor", *(INT*)&config.zoom.value);

							Config::CalcZoomed();
						}
						else
							config.zoom.enabled = FALSE;

						if (!config.version)
							Config::Set(CONFIG_DISCIPLE, "EnableZoom", config.zoom.enabled);
						Config::Set(CONFIG_WRAPPER, "EnableZoom", config.zoom.enabled);

						{
							CHAR str1[32];
							LoadString(hDllModule, IDS_STRETCH, str1, sizeof(str1));

							CHAR text[64];
							if (!config.zoom.enabled)
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
							ddraw->Redraw();

						CheckMenu(MenuStretch);
					}

					return NULL;
				}
				else if (wParam >= IDM_BORDERS_OFF && wParam <= IDM_BORDERS_ALTERNATIVE)
				{
					if (config.borders.allowed)
					{
						config.borders.type = BordersType(wParam - IDM_BORDERS_OFF);
						Config::Set(CONFIG_WRAPPER, "Borders", (INT)config.borders.type);

						{
							CHAR str1[32];
							LoadString(hDllModule, IDS_BORDERS, str1, sizeof(str1));

							CHAR str2[32];
							switch (config.borders.type)
							{
							case BordersClassic:
								LoadString(hDllModule, IDS_BORDERS_CLASSIC, str2, sizeof(str2));
								break;

							case BordersAlternative:
								LoadString(hDllModule, IDS_BORDERS_ALTERNATIVE, str2, sizeof(str2));
								break;

							default:
								LoadString(hDllModule, IDS_OFF, str2, sizeof(str2));
								break;
							}

							CHAR text[64];
							StrPrint(text, "%s: %s", str1, str2);

							Hooks::PrintText(text);
						}

						OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
						if (ddraw)
							ddraw->Redraw();

						CheckMenu(MenuBorders);
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

						Hooks::SetGameSpeed();
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

						CheckMenu(MenuSnapshot);
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

		case WM_DISPLAYCHANGE: {
			DEVMODE devMode;
			MemoryZero(&devMode, sizeof(DEVMODE));
			devMode.dmSize = sizeof(DEVMODE);
			config.syncStep = 1000.0 / (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode) && (devMode.dmFields & DM_DISPLAYFREQUENCY) ? devMode.dmDisplayFrequency : 60);

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		default:
			if (uMsg == config.msgSnapshot)
			{
				CHAR str[32];
				LoadString(hDllModule, IDS_SCR_FILE, str, sizeof(str));

				CHAR text[MAX_PATH + 36];
				StrPrint(text, "%s: %s", str, snapshotName);
				Hooks::PrintText(text);

				return NULL;
			}
			else if (uMsg == config.msgMenu)
			{
				CheckMenu(MenuRenderer);
				CheckMenu(MenuAspect);
				CheckMenu(MenuVSync);
				CheckMenu(MenuInterpolate);
				CheckMenu(MenuUpscale);
				CheckMenu(MenuStretch);
				CheckMenu(MenuWindowType);
				CheckMenu(MenuColors);

				return NULL;
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}
	}

	LRESULT __stdcall PanelProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
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