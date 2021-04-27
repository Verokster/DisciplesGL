#include "stdafx.h"
#include "Hooks.h"
#include "Config.h"
#include "Hooker.h"

namespace Hooks
{
	DWORD sub_RegisterClass;
	ATOM __stdcall RegisterClassHook(WNDCLASSA* lpWndClass)
	{
		if (!strcmp(lpWndClass->lpszClassName, "MQ_UIManager"))
			config.icon = lpWndClass->hIcon;

		return ((ATOM(__stdcall*)(WNDCLASSA*))sub_RegisterClass)(lpWndClass);
	}

	VOID Load()
	{
		HOOKER hooker = CreateHooker(GetModuleHandle(NULL));
		if (hooker)
		{
			PatchImportByName(hooker, "RegisterClassA", RegisterClassHook, &sub_RegisterClass);
			ReleaseHooker(hooker);
		}
	}
}