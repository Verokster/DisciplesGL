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

#include "dplay.h"
#include "Allocation.h"

class IPlay4 : IDirectPlay4, public Allocation {
	LPDIRECTPLAY4 dp;
	HWND hWnd;

public:
	IPlay4(LPDIRECTPLAY4, HWND);

	HRESULT EnumSessionsDecorated(LPDPSESSIONDESC2 lpsd, DWORD dwTimeout, LPDPENUMSESSIONSCALLBACK2 lpEnumSessionsCallback2, LPVOID lpContext, DWORD dwFlags);

	// Inherited via IUnknown
	HRESULT __stdcall QueryInterface(REFIID, LPVOID*);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();

	// Inherited via IDirectPlay4
	HRESULT __stdcall AddPlayerToGroup(DPID, DPID);
	HRESULT __stdcall Close();
	HRESULT __stdcall CreateGroup(LPDPID, LPDPNAME, LPVOID, DWORD, DWORD);
	HRESULT __stdcall CreatePlayer(LPDPID, LPDPNAME, HANDLE, LPVOID, DWORD, DWORD);
	HRESULT __stdcall DeletePlayerFromGroup(DPID, DPID);
	HRESULT __stdcall DestroyGroup(DPID);
	HRESULT __stdcall DestroyPlayer(DPID);
	HRESULT __stdcall EnumGroupPlayers(DPID, LPGUID, LPDPENUMPLAYERSCALLBACK2, LPVOID, DWORD);
	HRESULT __stdcall EnumGroups(LPGUID, LPDPENUMPLAYERSCALLBACK2, LPVOID, DWORD);
	HRESULT __stdcall EnumPlayers(LPGUID, LPDPENUMPLAYERSCALLBACK2, LPVOID, DWORD);
	HRESULT __stdcall EnumSessions(LPDPSESSIONDESC2, DWORD, LPDPENUMSESSIONSCALLBACK2, LPVOID, DWORD);
	HRESULT __stdcall GetCaps(LPDPCAPS, DWORD);
	HRESULT __stdcall GetGroupData(DPID, LPVOID, LPDWORD, DWORD);
	HRESULT __stdcall GetGroupName(DPID, LPVOID, LPDWORD);
	HRESULT __stdcall GetMessageCount(DPID, LPDWORD);
	HRESULT __stdcall GetPlayerAddress(DPID, LPVOID, LPDWORD);
	HRESULT __stdcall GetPlayerCaps(DPID, LPDPCAPS, DWORD);
	HRESULT __stdcall GetPlayerData(DPID, LPVOID, LPDWORD, DWORD);
	HRESULT __stdcall GetPlayerName(DPID, LPVOID, LPDWORD);
	HRESULT __stdcall GetSessionDesc(LPVOID, LPDWORD);
	HRESULT __stdcall Initialize(LPGUID);
	HRESULT __stdcall Open(LPDPSESSIONDESC2, DWORD);
	HRESULT __stdcall Receive(LPDPID, LPDPID, DWORD, LPVOID, LPDWORD);
	HRESULT __stdcall Send(DPID, DPID, DWORD, LPVOID, DWORD);
	HRESULT __stdcall SetGroupData(DPID, LPVOID, DWORD, DWORD);
	HRESULT __stdcall SetGroupName(DPID, LPDPNAME, DWORD);
	HRESULT __stdcall SetPlayerData(DPID, LPVOID, DWORD, DWORD);
	HRESULT __stdcall SetPlayerName(DPID, LPDPNAME, DWORD);
	HRESULT __stdcall SetSessionDesc(LPDPSESSIONDESC2, DWORD);
	HRESULT __stdcall AddGroupToGroup(DPID, DPID);
	HRESULT __stdcall CreateGroupInGroup(DPID, LPDPID, LPDPNAME, LPVOID, DWORD, DWORD);
	HRESULT __stdcall DeleteGroupFromGroup(DPID, DPID);
	HRESULT __stdcall EnumConnections(LPCGUID, LPDPENUMCONNECTIONSCALLBACK, LPVOID, DWORD);
	HRESULT __stdcall EnumGroupsInGroup(DPID, LPGUID, LPDPENUMPLAYERSCALLBACK2, LPVOID, DWORD);
	HRESULT __stdcall GetGroupConnectionSettings(DWORD, DPID, LPVOID, LPDWORD);
	HRESULT __stdcall InitializeConnection(LPVOID, DWORD);
	HRESULT __stdcall SecureOpen(LPCDPSESSIONDESC2, DWORD, LPCDPSECURITYDESC, LPCDPCREDENTIALS);
	HRESULT __stdcall SendChatMessage(DPID, DPID, DWORD, LPDPCHAT);
	HRESULT __stdcall SetGroupConnectionSettings(DWORD, DPID, LPDPLCONNECTION);
	HRESULT __stdcall StartSession(DWORD, DPID);
	HRESULT __stdcall GetGroupFlags(DPID, LPDWORD);
	HRESULT __stdcall GetGroupParent(DPID, LPDPID);
	HRESULT __stdcall GetPlayerAccount(DPID, DWORD, LPVOID, LPDWORD);
	HRESULT __stdcall GetPlayerFlags(DPID, LPDWORD);
	HRESULT __stdcall GetGroupOwner(DPID, LPDPID);
	HRESULT __stdcall SetGroupOwner(DPID, DPID);
	HRESULT __stdcall SendEx(DPID, DPID, DWORD, LPVOID, DWORD, DWORD, DWORD, LPVOID, LPDWORD);
	HRESULT __stdcall GetMessageQueue(DPID, DPID, DWORD, LPDWORD, LPDWORD);
	HRESULT __stdcall CancelMessage(DWORD, DWORD);
	HRESULT __stdcall CancelPriority(DWORD, DWORD, DWORD);
};
