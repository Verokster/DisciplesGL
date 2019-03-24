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

namespace Window
{
	HHOOK OldKeysHook;
	WNDPROC OldWindowProc, OldPanelProc;

	BOOL __fastcall FindMenuByChildId(HMENU hMenu, DWORD childId, HMENU* hParent, DWORD* pos)
	{
		DWORD count = GetMenuItemCount(hMenu);
		while (count--)
		{
			HMENU hSub = GetSubMenu(hMenu, count);
			if (hSub)
			{
				DWORD subCount = GetMenuItemCount(hSub);
				while (subCount--)
				{
					if (GetMenuItemID(hSub, subCount) == childId)
					{
						*hParent = hMenu;
						*pos = count;
						return TRUE;
					}

					if (FindMenuByChildId(hSub, childId, hParent, pos))
						return TRUE;
				}
			}
		}

		return FALSE;
	}

	VOID __fastcall CheckMenu(HMENU hMenu)
	{
		if (!hMenu)
			return;

		EnableMenuItem(hMenu, IDM_ASPECT_RATIO, MF_BYCOMMAND | (glVersion ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
		CheckMenuItem(hMenu, IDM_ASPECT_RATIO, MF_BYCOMMAND | (glVersion && config.image.aspect ? MF_CHECKED : MF_UNCHECKED));

		EnableMenuItem(hMenu, IDM_VSYNC, MF_BYCOMMAND | (WGLSwapInterval ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
		CheckMenuItem(hMenu, IDM_VSYNC, MF_BYCOMMAND | (WGLSwapInterval && config.image.vSync ? MF_CHECKED : MF_UNCHECKED));

		CheckMenuItem(hMenu, IDM_FILT_OFF, MF_BYCOMMAND | MF_UNCHECKED);

		EnableMenuItem(hMenu, IDM_FILT_LINEAR, MF_BYCOMMAND | (glVersion ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
		CheckMenuItem(hMenu, IDM_FILT_LINEAR, MF_BYCOMMAND | MF_UNCHECKED);

		DWORD isFilters20 = glVersion >= GL_VER_2_0 ? MF_ENABLED : (MF_DISABLED | MF_GRAYED);
		DWORD isFilters30 = glVersion >= GL_VER_3_0 ? MF_ENABLED : (MF_DISABLED | MF_GRAYED);

		EnableMenuItem(hMenu, IDM_FILT_CUBIC, MF_BYCOMMAND | isFilters20);
		CheckMenuItem(hMenu, IDM_FILT_CUBIC, MF_BYCOMMAND | MF_UNCHECKED);

		// ScaleNx
		HMENU hMenuScaleNx = NULL; DWORD posScaleNx;
		FindMenuByChildId(hMenu, IDM_FILT_SCALENX_LINEAR, &hMenuScaleNx, &posScaleNx);
		if (hMenuScaleNx)
		{
			EnableMenuItem(hMenuScaleNx, posScaleNx, MF_BYPOSITION | isFilters30);
			CheckMenuItem(hMenuScaleNx, posScaleNx, MF_BYPOSITION | MF_UNCHECKED);
		}

		EnableMenuItem(hMenu, IDM_FILT_SCALENX_LINEAR, MF_BYCOMMAND | isFilters30);
		CheckMenuItem(hMenu, IDM_FILT_SCALENX_LINEAR, MF_BYCOMMAND | MF_UNCHECKED);
		EnableMenuItem(hMenu, IDM_FILT_SCALENX_CUBIC, MF_BYCOMMAND | isFilters30);
		CheckMenuItem(hMenu, IDM_FILT_SCALENX_CUBIC, MF_BYCOMMAND | MF_UNCHECKED);
		EnableMenuItem(hMenu, IDM_FILT_SCALENX_2X, MF_BYCOMMAND | isFilters30);
		CheckMenuItem(hMenu, IDM_FILT_SCALENX_2X, MF_BYCOMMAND | MF_UNCHECKED);
		EnableMenuItem(hMenu, IDM_FILT_SCALENX_3X, MF_BYCOMMAND | isFilters30);
		CheckMenuItem(hMenu, IDM_FILT_SCALENX_3X, MF_BYCOMMAND | MF_UNCHECKED);

		// Eagle
		HMENU hMenuEagle = NULL; DWORD posEagle;
		FindMenuByChildId(hMenu, IDM_FILT_EAGLE_LINEAR, &hMenuEagle, &posEagle);
		if (hMenuEagle)
		{
			EnableMenuItem(hMenuEagle, posEagle, MF_BYPOSITION | isFilters30);
			CheckMenuItem(hMenuEagle, posEagle, MF_BYPOSITION | MF_UNCHECKED);
		}

		EnableMenuItem(hMenu, IDM_FILT_EAGLE_LINEAR, MF_BYCOMMAND | isFilters30);
		CheckMenuItem(hMenu, IDM_FILT_EAGLE_LINEAR, MF_BYCOMMAND | MF_UNCHECKED);
		EnableMenuItem(hMenu, IDM_FILT_EAGLE_CUBIC, MF_BYCOMMAND | isFilters30);
		CheckMenuItem(hMenu, IDM_FILT_EAGLE_CUBIC, MF_BYCOMMAND | MF_UNCHECKED);
		EnableMenuItem(hMenu, IDM_FILT_EAGLE_2X, MF_BYCOMMAND | isFilters30);
		CheckMenuItem(hMenu, IDM_FILT_EAGLE_2X, MF_BYCOMMAND | MF_UNCHECKED);

		// XSal
		HMENU hMenuXSal = NULL; DWORD posXSal;
		FindMenuByChildId(hMenu, IDM_FILT_XSAL_LINEAR, &hMenuXSal, &posXSal);
		if (hMenuXSal)
		{
			EnableMenuItem(hMenuXSal, posXSal, MF_BYPOSITION | isFilters30);
			CheckMenuItem(hMenuXSal, posXSal, MF_BYPOSITION | MF_UNCHECKED);
		}

		EnableMenuItem(hMenu, IDM_FILT_XSAL_LINEAR, MF_BYCOMMAND | isFilters30);
		CheckMenuItem(hMenu, IDM_FILT_XSAL_LINEAR, MF_BYCOMMAND | MF_UNCHECKED);
		EnableMenuItem(hMenu, IDM_FILT_XSAL_CUBIC, MF_BYCOMMAND | isFilters30);
		CheckMenuItem(hMenu, IDM_FILT_XSAL_CUBIC, MF_BYCOMMAND | MF_UNCHECKED);
		EnableMenuItem(hMenu, IDM_FILT_XSAL_2X, MF_BYCOMMAND | isFilters30);
		CheckMenuItem(hMenu, IDM_FILT_XSAL_2X, MF_BYCOMMAND | MF_UNCHECKED);

		// ScaleHQ
		HMENU hMenuScaleHQ = NULL; DWORD posScaleHQ;
		FindMenuByChildId(hMenu, IDM_FILT_SCALEHQ_LINEAR, &hMenuScaleHQ, &posScaleHQ);
		if (hMenuScaleHQ)
		{
			EnableMenuItem(hMenuScaleHQ, posScaleHQ, MF_BYPOSITION | isFilters30);
			CheckMenuItem(hMenuScaleHQ, posScaleHQ, MF_BYPOSITION | MF_UNCHECKED);
		}

		EnableMenuItem(hMenu, IDM_FILT_SCALEHQ_LINEAR, MF_BYCOMMAND | isFilters30);
		CheckMenuItem(hMenu, IDM_FILT_SCALEHQ_LINEAR, MF_BYCOMMAND | MF_UNCHECKED);
		EnableMenuItem(hMenu, IDM_FILT_SCALEHQ_CUBIC, MF_BYCOMMAND | isFilters30);
		CheckMenuItem(hMenu, IDM_FILT_SCALEHQ_CUBIC, MF_BYCOMMAND | MF_UNCHECKED);
		EnableMenuItem(hMenu, IDM_FILT_SCALEHQ_2X, MF_BYCOMMAND | isFilters30);
		CheckMenuItem(hMenu, IDM_FILT_SCALEHQ_2X, MF_BYCOMMAND | MF_UNCHECKED);
		EnableMenuItem(hMenu, IDM_FILT_SCALEHQ_4X, MF_BYCOMMAND | isFilters30);
		CheckMenuItem(hMenu, IDM_FILT_SCALEHQ_4X, MF_BYCOMMAND | MF_UNCHECKED);

		// xBRz
		HMENU hMenuXBRZ = NULL; DWORD posXBRZ;
		FindMenuByChildId(hMenu, IDM_FILT_XRBZ_LINEAR, &hMenuXBRZ, &posXBRZ);
		if (hMenuXBRZ)
		{
			EnableMenuItem(hMenuXBRZ, posXBRZ, MF_BYPOSITION | isFilters30);
			CheckMenuItem(hMenuXBRZ, posXBRZ, MF_BYPOSITION | MF_UNCHECKED);
		}

		EnableMenuItem(hMenu, IDM_FILT_XRBZ_LINEAR, MF_BYCOMMAND | isFilters30);
		CheckMenuItem(hMenu, IDM_FILT_XRBZ_LINEAR, MF_BYCOMMAND | MF_UNCHECKED);
		EnableMenuItem(hMenu, IDM_FILT_XRBZ_CUBIC, MF_BYCOMMAND | isFilters30);
		CheckMenuItem(hMenu, IDM_FILT_XRBZ_CUBIC, MF_BYCOMMAND | MF_UNCHECKED);
		EnableMenuItem(hMenu, IDM_FILT_XRBZ_2X, MF_BYCOMMAND | isFilters30);
		CheckMenuItem(hMenu, IDM_FILT_XRBZ_2X, MF_BYCOMMAND | MF_UNCHECKED);
		EnableMenuItem(hMenu, IDM_FILT_XRBZ_3X, MF_BYCOMMAND | isFilters30);
		CheckMenuItem(hMenu, IDM_FILT_XRBZ_3X, MF_BYCOMMAND | MF_UNCHECKED);
		EnableMenuItem(hMenu, IDM_FILT_XRBZ_4X, MF_BYCOMMAND | isFilters30);
		CheckMenuItem(hMenu, IDM_FILT_XRBZ_4X, MF_BYCOMMAND | MF_UNCHECKED);
		EnableMenuItem(hMenu, IDM_FILT_XRBZ_5X, MF_BYCOMMAND | isFilters30);
		CheckMenuItem(hMenu, IDM_FILT_XRBZ_5X, MF_BYCOMMAND | MF_UNCHECKED);
		EnableMenuItem(hMenu, IDM_FILT_XRBZ_6X, MF_BYCOMMAND | isFilters30);
		CheckMenuItem(hMenu, IDM_FILT_XRBZ_6X, MF_BYCOMMAND | MF_UNCHECKED);

		switch (config.image.filter)
		{
		case FilterLinear:
			CheckMenuItem(hMenu, glVersion ? IDM_FILT_LINEAR : IDM_FILT_LINEAR, MF_BYCOMMAND | MF_CHECKED);
			break;

		case FilterCubic:
			CheckMenuItem(hMenu, isFilters20 == MF_ENABLED ? IDM_FILT_CUBIC : IDM_FILT_LINEAR, MF_BYCOMMAND | MF_CHECKED);
			break;

		case FilterScaleNx:
			if (isFilters30 == MF_ENABLED)
			{
				if (hMenuScaleNx)
					CheckMenuItem(hMenuScaleNx, posScaleNx, MF_BYPOSITION | MF_CHECKED);

				CheckMenuItem(hMenu, config.image.scaleNx.type ? IDM_FILT_SCALENX_CUBIC : IDM_FILT_SCALENX_LINEAR, MF_BYCOMMAND | MF_CHECKED);
				switch (config.image.scaleNx.value)
				{
				case 3:
					CheckMenuItem(hMenu, IDM_FILT_SCALENX_3X, MF_BYCOMMAND | MF_CHECKED);
					break;

				default:
					CheckMenuItem(hMenu, IDM_FILT_SCALENX_2X, MF_BYCOMMAND | MF_CHECKED);
					break;
				}
			}
			else
				CheckMenuItem(hMenu, IDM_FILT_LINEAR, MF_BYCOMMAND | MF_CHECKED);

			break;

		case FilterEagle:
			if (isFilters30 == MF_ENABLED)
			{
				if (hMenuEagle)
					CheckMenuItem(hMenuEagle, posEagle, MF_BYPOSITION | MF_CHECKED);

				CheckMenuItem(hMenu, config.image.eagle.type ? IDM_FILT_EAGLE_CUBIC : IDM_FILT_EAGLE_LINEAR, MF_BYCOMMAND | MF_CHECKED);
				CheckMenuItem(hMenu, IDM_FILT_EAGLE_2X, MF_BYCOMMAND | MF_CHECKED);
			}
			else
				CheckMenuItem(hMenu, IDM_FILT_LINEAR, MF_BYCOMMAND | MF_CHECKED);

			break;

		case FilterXSal:
			if (isFilters30 == MF_ENABLED)
			{
				if (hMenuXSal)
					CheckMenuItem(hMenuXSal, posXSal, MF_BYPOSITION | MF_CHECKED);

				CheckMenuItem(hMenu, config.image.xSal.type ? IDM_FILT_XSAL_CUBIC : IDM_FILT_XSAL_LINEAR, MF_BYCOMMAND | MF_CHECKED);
				CheckMenuItem(hMenu, IDM_FILT_XSAL_2X, MF_BYCOMMAND | MF_CHECKED);
			}
			else
				CheckMenuItem(hMenu, IDM_FILT_LINEAR, MF_BYCOMMAND | MF_CHECKED);

			break;

		case FilterScaleHQ:
			if (isFilters30 == MF_ENABLED)
			{
				if (hMenuScaleHQ)
					CheckMenuItem(hMenuScaleHQ, posScaleHQ, MF_BYPOSITION | MF_CHECKED);

				CheckMenuItem(hMenu, config.image.scaleHQ.type ? IDM_FILT_SCALEHQ_CUBIC : IDM_FILT_SCALEHQ_LINEAR, MF_BYCOMMAND | MF_CHECKED);
				switch (config.image.scaleHQ.value)
				{
				case 4:
					CheckMenuItem(hMenu, IDM_FILT_SCALEHQ_4X, MF_BYCOMMAND | MF_CHECKED);
					break;

				default:
					CheckMenuItem(hMenu, IDM_FILT_SCALEHQ_2X, MF_BYCOMMAND | MF_CHECKED);
					break;
				}
			}
			else
				CheckMenuItem(hMenu, IDM_FILT_LINEAR, MF_BYCOMMAND | MF_CHECKED);

			break;

		case FilterXRBZ:
			if (isFilters30 == MF_ENABLED)
			{
				if (hMenuXBRZ)
					CheckMenuItem(hMenuXBRZ, posXBRZ, MF_BYPOSITION | MF_CHECKED);

				CheckMenuItem(hMenu, config.image.xBRz.type ? IDM_FILT_XRBZ_CUBIC : IDM_FILT_XRBZ_LINEAR, MF_BYCOMMAND | MF_CHECKED);
				switch (config.image.xBRz.value)
				{
				case 3:
					CheckMenuItem(hMenu, IDM_FILT_XRBZ_3X, MF_BYCOMMAND | MF_CHECKED);
					break;

				case 4:
					CheckMenuItem(hMenu, IDM_FILT_XRBZ_4X, MF_BYCOMMAND | MF_CHECKED);
					break;

				case 5:
					CheckMenuItem(hMenu, IDM_FILT_XRBZ_5X, MF_BYCOMMAND | MF_CHECKED);
					break;

				case 6:
					CheckMenuItem(hMenu, IDM_FILT_XRBZ_6X, MF_BYCOMMAND | MF_CHECKED);
					break;

				default:
					CheckMenuItem(hMenu, IDM_FILT_XRBZ_2X, MF_BYCOMMAND | MF_CHECKED);
					break;
				}
			}
			else
				CheckMenuItem(hMenu, IDM_FILT_LINEAR, MF_BYCOMMAND | MF_CHECKED);

			break;

		default:
			CheckMenuItem(hMenu, IDM_FILT_OFF, MF_BYCOMMAND | MF_CHECKED);
			break;
		}
	}

	VOID __fastcall CheckMenu(HWND hWnd)
	{
		CheckMenu(GetMenu(hWnd));
	}

	VOID __fastcall FilterChanged(HWND hWnd)
	{
		Config::Set(CONFIG_WRAPPER, "ImageFilter", *(INT*)&config.image.filter);
		CheckMenu(hWnd);

		OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
		if (ddraw)
		{
			ddraw->isStateChanged = TRUE;
			SetEvent(ddraw->hDrawEvent);
		}
	}

	VOID __fastcall SelectScaleNxMode(HWND hWnd, WORD value, DWORD isMode)
	{
		if (isMode)
			config.image.scaleNx.value = value;
		else
			config.image.scaleNx.type = value;

		Config::Set(CONFIG_WRAPPER, "ImageScaleNx", *(INT*)&config.image.scaleNx);

		config.image.filter = glVersion >= GL_VER_3_0 ? FilterScaleNx : FilterLinear;
		FilterChanged(hWnd);
	}

	VOID __fastcall SelectXSalMode(HWND hWnd, WORD value, DWORD isMode)
	{
		if (isMode)
			config.image.xSal.value = value;
		else
			config.image.xSal.type = value;

		Config::Set(CONFIG_WRAPPER, "ImageXSal", *(INT*)&config.image.xSal);

		config.image.filter = glVersion >= GL_VER_3_0 ? FilterXSal : FilterLinear;
		FilterChanged(hWnd);
	}

	VOID __fastcall SelectEagleMode(HWND hWnd, WORD value, DWORD isMode)
	{
		if (isMode)
			config.image.eagle.value = value;
		else
			config.image.eagle.type = value;

		Config::Set(CONFIG_WRAPPER, "ImageEagle", *(INT*)&config.image.eagle);

		config.image.filter = glVersion >= GL_VER_3_0 ? FilterEagle : FilterLinear;
		FilterChanged(hWnd);
	}

	VOID __fastcall SelectScaleHQMode(HWND hWnd, WORD value, DWORD isMode)
	{
		if (isMode)
			config.image.scaleHQ.value = value;
		else
			config.image.scaleHQ.type = value;

		Config::Set(CONFIG_WRAPPER, "ImageScaleHQ", *(INT*)&config.image.scaleHQ);

		config.image.filter = glVersion >= GL_VER_3_0 ? FilterScaleHQ : FilterLinear;
		FilterChanged(hWnd);
	}

	VOID __fastcall SelectXBRZMode(HWND hWnd, WORD value, DWORD isMode)
	{
		if (isMode)
			config.image.xBRz.value = value;
		else
			config.image.xBRz.type = value;

		Config::Set(CONFIG_WRAPPER, "ImageXBRZ", *(INT*)&config.image.xBRz);

		config.image.filter = glVersion >= GL_VER_3_0 ? FilterXRBZ : FilterLinear;
		FilterChanged(hWnd);
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
				HWND hWnd = GetActiveWindow();
				OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
				if (ddraw && !config.windowedMode)
				{
					ddraw->isTakeSnapshot = TRUE;
					if (ddraw)
						SetEvent(ddraw->hDrawEvent);
					MessageBeep(0);
					return TRUE;
				}
			}
		}

		return CallNextHookEx(OldKeysHook, nCode, wParam, lParam);
	}

	LRESULT __stdcall AboutProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_INITDIALOG:
		{
			SetWindowLong(hDlg, GWL_EXSTYLE, NULL);
			EnumChildWindows(hDlg, EnumChildProc, NULL);

			CHAR email[50];
			GetDlgItemText(hDlg, IDC_LNK_EMAIL, email, sizeof(email) - 1);
			CHAR anchor[256];
			StrPrint(anchor, "<A HREF=\"mailto:%s\">%s</A>", email, email);
			SetDlgItemText(hDlg, IDC_LNK_EMAIL, anchor);

			break;
		}

		case WM_NOTIFY:
		{
			if (((NMHDR*)lParam)->code == NM_CLICK && wParam == IDC_LNK_EMAIL)
			{
				NMLINK* pNMLink = (NMLINK*)lParam;
				LITEM iItem = pNMLink->item;

				CHAR url[MAX_PATH];
				StrToAnsi(url, pNMLink->item.szUrl, sizeof(url) - 1);

				SHELLEXECUTEINFO shExecInfo;
				MemoryZero(&shExecInfo, sizeof(SHELLEXECUTEINFO));
				shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
				shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
				shExecInfo.lpFile = url;
				shExecInfo.nShow = SW_SHOW;

				ShellExecuteEx(&shExecInfo);
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
			{
				EndPaint(hWnd, &paint);
				return NULL;
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		/*case WM_WINDOWPOSCHANGED:
		{
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
			{
				RECT rect;
				GetClientRect(hWnd, &rect);
				ddraw->viewport.width = rect.right;
				ddraw->viewport.height = rect.bottom;
				ddraw->viewport.refresh = TRUE;
				SetEvent(ddraw->hDrawEvent);
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}*/

		case WM_MOVE:
		{
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw && ddraw->hDraw)
			{
				DWORD stye = GetWindowLong(ddraw->hDraw, GWL_STYLE);
				if (stye & WS_POPUP)
				{
					POINT point = { LOWORD(lParam), HIWORD(lParam) };
					ScreenToClient(hWnd, &point);

					RECT rect;
					rect.left = point.x - LOWORD(lParam);
					rect.top = point.y - HIWORD(lParam);
					rect.right = rect.left + 256;
					rect.bottom = rect.left + 256;

					AdjustWindowRect(&rect, stye, FALSE);
					SetWindowPos(ddraw->hDraw, NULL, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOREDRAW | SWP_NOREPOSITION | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
				}
				else
					SetWindowPos(ddraw->hDraw, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOREDRAW | SWP_NOREPOSITION | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
			
				//SetEvent(ddraw->hDrawEvent);
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_SIZE:
		{
			OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
			if (ddraw)
			{
				if (ddraw->hDraw)
					SetWindowPos(ddraw->hDraw, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOREPOSITION | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);

				ddraw->viewport.width = LOWORD(lParam);
				ddraw->viewport.height = HIWORD(lParam);
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
			AdjustWindowRect(&max, style, config.windowedMode);

			MINMAXINFO* mmi = (MINMAXINFO*)lParam;
			mmi->ptMinTrackSize.x = min.right - min.left;
			mmi->ptMinTrackSize.y = min.bottom - min.top;
			mmi->ptMaxTrackSize.x = max.right - max.left;
			mmi->ptMaxTrackSize.y = max.bottom - max.top;

			return NULL;
		}

		case WM_ACTIVATEAPP:
		{
			if (!config.windowedMode)
			{
				OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
				if (ddraw)
				{
					if ((BOOL)wParam)
						ddraw->RenderStart();
					else
						ddraw->RenderStop();
				}

				return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
			}
			else
			{
				if (!(BOOL)wParam)
					return NULL;
				return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
			}
		}

		case WM_SETCURSOR:
		{
			if (LOWORD(lParam) == HTCLIENT)
			{
				if (!config.windowedMode || !config.image.aspect)
				{
					SetCursor(NULL);
					return TRUE;
				}
				else
				{
					OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
					if (ddraw)
					{
						POINT p;
						GetCursorPos(&p);
						ScreenToClient(hWnd, &p);

						if (p.x >= ddraw->viewport.rectangle.x && p.x < ddraw->viewport.rectangle.x + ddraw->viewport.rectangle.width &&
							p.y >= ddraw->viewport.rectangle.y && p.y < ddraw->viewport.rectangle.y + ddraw->viewport.rectangle.height)
							SetCursor(NULL);
						else
							SetCursor(config.cursor);

						return TRUE;
					}
				}
			}

			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			if (!(HIWORD(lParam) & KF_ALTDOWN))
			{
				if (config.keys.fpsCounter && config.keys.fpsCounter + VK_F1 - 1 == wParam)
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
					switch (config.image.filter)
					{
					case FilterNearest:
						config.image.filter = FilterLinear;
						break;

					case FilterLinear:
						config.image.filter = glVersion >= GL_VER_2_0 ? FilterCubic : FilterNearest;
						break;

					default:
						config.image.filter = FilterNearest;
						break;
					}

					FilterChanged(hWnd);

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
			case IDM_FILE_QUIT:
			{
				SendMessage(hWnd, WM_CLOSE, NULL, NULL);
				return NULL;
			}

			case IDM_HELP_ABOUT:
			{
				INT_PTR res;
				ULONG_PTR cookie = NULL;
				if (hActCtx && hActCtx != INVALID_HANDLE_VALUE && !ActivateActCtxC(hActCtx, &cookie))
					cookie = NULL;

				res = DialogBoxParam(hDllModule, MAKEINTRESOURCE(IDD_ABOUT), hWnd, (DLGPROC)AboutProc, NULL);

				if (cookie)
					DeactivateActCtxC(0, cookie);

				SetForegroundWindow(hWnd);
				return NULL;
			}

			case IDM_RES_FULL_SCREEN:
			{
				OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
				if (ddraw)
				{
					ddraw->RenderStop();
					{
						if (!config.windowedMode)
							ddraw->SetWindowedMode();
						else
							ddraw->SetFullscreenMode();

						config.windowedMode = !config.windowedMode;
						Config::Set(CONFIG_DISCIPLE, config.version ? "InWindow" : "DisplayMode", config.windowedMode);
						CheckMenu(hWnd);
					}
					ddraw->RenderStart();
				}

				return NULL;
			}

			case IDM_ASPECT_RATIO:
			{
				config.image.aspect = !config.image.aspect;
				Config::Set(CONFIG_WRAPPER, "ImageAspect", config.image.aspect);
				CheckMenu(hWnd);

				OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
				if (ddraw)
				{
					ddraw->viewport.refresh = TRUE;
					SetEvent(ddraw->hDrawEvent);
				}

				return NULL;
			}

			case IDM_VSYNC:
			{
				config.image.vSync = !config.image.vSync;
				Config::Set(CONFIG_WRAPPER, "ImageVSync", config.image.vSync);
				CheckMenu(hWnd);

				OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
				if (ddraw)
					SetEvent(ddraw->hDrawEvent);

				return NULL;
			}

			case IDM_FILT_OFF:
			{
				config.image.filter = FilterNearest;
				FilterChanged(hWnd);
				return NULL;
			}

			case IDM_FILT_LINEAR:
			{
				config.image.filter = FilterLinear;
				FilterChanged(hWnd);
				return NULL;
			}

			case IDM_FILT_CUBIC:
			{
				config.image.filter = glVersion >= GL_VER_2_0 ? FilterCubic : FilterNearest;
				FilterChanged(hWnd);
				return NULL;
			}

			case IDM_FILT_SCALENX_LINEAR:
			{
				SelectScaleNxMode(hWnd, 0, FALSE);
				return NULL;
			}

			case IDM_FILT_SCALENX_CUBIC:
			{
				SelectScaleNxMode(hWnd, 1, FALSE);
				return NULL;
			}

			case IDM_FILT_SCALENX_2X:
			{
				SelectScaleNxMode(hWnd, 2, TRUE);
				return NULL;
			}

			case IDM_FILT_SCALENX_3X:
			{
				SelectScaleNxMode(hWnd, 3, TRUE);
				return NULL;
			}

			case IDM_FILT_XSAL_LINEAR:
			{
				SelectXSalMode(hWnd, 0, FALSE);
				return NULL;
			}

			case IDM_FILT_XSAL_CUBIC:
			{
				SelectXSalMode(hWnd, 1, FALSE);
				return NULL;
			}

			case IDM_FILT_XSAL_2X:
			{
				SelectXSalMode(hWnd, 2, TRUE);
				return NULL;
			}

			case IDM_FILT_EAGLE_LINEAR:
			{
				SelectEagleMode(hWnd, 0, FALSE);
				return NULL;
			}

			case IDM_FILT_EAGLE_CUBIC:
			{
				SelectEagleMode(hWnd, 1, FALSE);
				return NULL;
			}

			case IDM_FILT_EAGLE_2X:
			{
				SelectEagleMode(hWnd, 2, TRUE);
				return NULL;
			}

			case IDM_FILT_SCALEHQ_LINEAR:
			{
				SelectScaleHQMode(hWnd, 0, FALSE);
				return NULL;
			}

			case IDM_FILT_SCALEHQ_CUBIC:
			{
				SelectScaleHQMode(hWnd, 1, FALSE);
				return NULL;
			}

			case IDM_FILT_SCALEHQ_2X:
			{
				SelectScaleHQMode(hWnd, 2, TRUE);
				return NULL;
			}

			case IDM_FILT_SCALEHQ_4X:
			{
				SelectScaleHQMode(hWnd, 4, TRUE);
				return NULL;
			}

			case IDM_FILT_XRBZ_LINEAR:
			{
				SelectXBRZMode(hWnd, 0, FALSE);
				return NULL;
			}

			case IDM_FILT_XRBZ_CUBIC:
			{
				SelectXBRZMode(hWnd, 1, FALSE);
				return NULL;
			}

			case IDM_FILT_XRBZ_2X:
			{
				SelectXBRZMode(hWnd, 2, TRUE);
				return NULL;
			}

			case IDM_FILT_XRBZ_3X:
			{
				SelectXBRZMode(hWnd, 3, TRUE);
				return NULL;
			}

			case IDM_FILT_XRBZ_4X:
			{
				SelectXBRZMode(hWnd, 4, TRUE);
				return NULL;
			}

			case IDM_FILT_XRBZ_5X:
			{
				SelectXBRZMode(hWnd, 5, TRUE);
				return NULL;
			}

			case IDM_FILT_XRBZ_6X:
			{
				SelectXBRZMode(hWnd, 6, TRUE);
				return NULL;
			}

			default:
				return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
			}
		}

		default:
			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}
	}

	LRESULT __stdcall PanelProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
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