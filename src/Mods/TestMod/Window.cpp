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
#include "Window.h"
#include "CommCtrl.h"
#include "Windowsx.h"
#include "Shellapi.h"
#include "Resource.h"
#include "Main.h"
#include "Config.h"

namespace Window
{
	WNDPROC OldWindowProc;

	VOID CheckMenu(HMENU hMenu, MenuType type)
	{
		if (!hMenu)
			return;

		switch (type)
		{
		case MenuEnabled:
			CheckMenuItem(hMenu, IDM_ENABLED, MF_BYCOMMAND | (config.state.enabled ? MF_CHECKED : MF_UNCHECKED));
			EnableMenuItem(hMenu, IDM_RESET, MF_BYCOMMAND | (config.state.enabled ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			break;
		case MenuPaused:
			CheckMenuItem(hMenu, IDM_PAUSED, MF_BYCOMMAND | (config.state.paused ? MF_CHECKED : MF_UNCHECKED));
			break;
		case MenuReset:
			EnableMenuItem(hMenu, IDM_RESET, MF_BYCOMMAND | (config.state.enabled ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
			break;
		case MenuDisplay:
			CheckMenuItem(hMenu, IDM_TOP_LEFT, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(hMenu, IDM_TOP_RIGHT, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(hMenu, IDM_BOTTOM_LEFT, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(hMenu, IDM_BOTTOM_RIGHT, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(hMenu, IDM_TOP_LEFT + (config.displayCorner - DisplayTopLeft), MF_BYCOMMAND | MF_CHECKED);
			break;
		default:
			break;
		}
	}

	VOID CheckMenu(HWND hWnd, MenuType type)
	{
		CheckMenu(GetMenu(hWnd), type);
	}

	VOID CheckMenu(HMENU hMenu)
	{
		CheckMenu(hMenu, MenuEnabled);
		CheckMenu(hMenu, MenuPaused);
		CheckMenu(hMenu, MenuReset);
		CheckMenu(hMenu, MenuDisplay);
	}

	VOID CheckMenu(HWND hWnd)
	{
		CheckMenu(GetMenu(hWnd));
	}

	BOOL __stdcall EnumChildProc(HWND hDlg, LPARAM lParam)
	{
		if ((GetWindowLong(hDlg, GWL_STYLE) & SS_ICON) == SS_ICON)
			SendMessage(hDlg, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)config.icon);
		else
			SendMessage(hDlg, WM_SETFONT, (WPARAM)config.font, TRUE);

		return TRUE;
	}

	LRESULT __stdcall AboutProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_INITDIALOG: {
			SetWindowLong(hDlg, GWL_EXSTYLE, NULL);
			EnumChildWindows(hDlg, EnumChildProc, NULL);

			CHAR path[MAX_PATH];
			CHAR temp[100];

			GetModuleFileName(config.hModule, path, sizeof(path));

			DWORD hSize;
			DWORD verSize = GetFileVersionInfoSize(path, &hSize);

			if (verSize)
			{
				CHAR* verData = (CHAR*)malloc(verSize);
				{
					if (GetFileVersionInfo(path, hSize, verSize, verData))
					{
						VOID* buffer;
						UINT size;
						if (VerQueryValue(verData, "\\", &buffer, &size) && size)
						{
							VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)buffer;

							GetDlgItemText(hDlg, IDC_VERSION, temp, sizeof(temp));
							sprintf(path, temp, HIWORD(verInfo->dwProductVersionMS), LOWORD(verInfo->dwProductVersionMS), HIWORD(verInfo->dwProductVersionLS), LOWORD(verInfo->dwFileVersionLS));
							SetDlgItemText(hDlg, IDC_VERSION, path);
						}
					}
				}
				free(verData);
			}

			if (GetDlgItemText(hDlg, IDC_COPYRIGHT, temp, sizeof(temp)))
			{
				sprintf(path, temp, 2021, "Verok");
				SetDlgItemText(hDlg, IDC_COPYRIGHT, path);
			}

			sprintf(path, "<A HREF=\"mailto:%s\">%s</A>", "verokster@gmail.com", "verokster@gmail.com");
			SetDlgItemText(hDlg, IDC_LNK_EMAIL, path);

			break;
		}

		case WM_NOTIFY: {
			if (((NMHDR*)lParam)->code == NM_CLICK)
			{
				switch (wParam)
				{
				case IDC_LNK_EMAIL: {
					SHELLEXECUTEINFOW shExecInfo = {};
					shExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
					shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
					shExecInfo.lpFile = ((NMLINK*)lParam)->item.szUrl;
					shExecInfo.nShow = SW_SHOW;
					ShellExecuteExW(&shExecInfo);
					break;
				}

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

	LRESULT __stdcall WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN: {
			if (HIWORD(lParam) & KF_ALTDOWN)
			{
				switch (wParam)
				{
				case 'P':
					WindowProc(hWnd, WM_COMMAND, IDM_PAUSED, NULL);
					break;

				case 'R':
					WindowProc(hWnd, WM_COMMAND, IDM_RESET, NULL);
					break;

				default:
					break;
				}
			}

			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}

		case WM_COMMAND: {
			switch (wParam)
			{
			case IDM_ENABLED:
				config.state.enabled = !config.state.enabled;
				config.tick.begin = config.tick.end = config.state.enabled ? GetTickCount() : 0;
				Config::Set(CONFIG_MOD, "Enabled", config.state.enabled);
				Window::CheckMenu(hWnd, MenuEnabled);
				Window::CheckMenu(hWnd, MenuReset);
				return NULL;

			case IDM_PAUSED:
				config.state.paused = !config.state.paused;
				if (!config.state.enabled)
					config.tick.end = 0;
				else
				{
					DWORD tick = GetTickCount();
					if (!config.state.paused)
						config.tick.begin = tick - (config.tick.end - config.tick.begin);
					config.tick.end = tick;
				}
				
				Window::CheckMenu(hWnd, MenuPaused);
				return NULL;

			case IDM_RESET:
				config.tick.begin = config.tick.end = config.state.enabled ? GetTickCount() : 0;
				return NULL;

			case IDM_TOP_LEFT:
			case IDM_TOP_RIGHT:
			case IDM_BOTTOM_LEFT:
			case IDM_BOTTOM_RIGHT:
				config.displayCorner = DisplayCorner(DisplayTopLeft + wParam - IDM_TOP_LEFT);
				Config::Set(CONFIG_MOD, "DisplayCorner", config.displayCorner);
				Window::CheckMenu(hWnd, MenuDisplay);
				return NULL;

			case IDM_ABOUT:
				DialogBoxParam(config.hModule, MAKEINTRESOURCE(IDD_ABOUT), hWnd, (DLGPROC)AboutProc, NULL);
				return NULL;

			default:
				return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
			}
		}

		default:
			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}
	}

	VOID SetCaptureWindow(HWND hWnd)
	{
		OldWindowProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WindowProc);
	}
}