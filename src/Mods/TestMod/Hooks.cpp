#include "stdafx.h"
#include "Hooks.h"
#include "Config.h"
#include "Hooker.h"

namespace Hooks
{
	DWORD RegisterClassOld;
	ATOM __stdcall RegisterClassHook(WNDCLASSA* lpWndClass)
	{
		if (!strcmp(lpWndClass->lpszClassName, "MQ_UIManager"))
			config.icon = lpWndClass->hIcon;

		return ((ATOM(__stdcall*)(WNDCLASSA*))RegisterClassOld)(lpWndClass);
	}

	VOID Load()
	{
		HOOKER hooker = CreateHooker(GetModuleHandle(NULL));
		{
			PatchImportByName(hooker, "RegisterClassA", RegisterClassHook, &RegisterClassOld);
		}
		ReleaseHooker(hooker);
	}
}