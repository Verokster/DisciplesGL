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
#include "IPlay4.h"
#include "Main.h"
#include "Config.h"
#include "OpenDraw.h"

IPlay4::IPlay4(LPDIRECTPLAY4 dp, HWND hWnd)
{
	this->dp = dp;
	this->hWnd = hWnd;
}

HRESULT __stdcall IPlay4::QueryInterface(REFIID riid, LPVOID* obp)
{
	return this->dp->QueryInterface(riid, obp);
}

ULONG __stdcall IPlay4::AddRef()
{
	return this->dp->AddRef();
}

ULONG __stdcall IPlay4::Release()
{
	DWORD ref = this->dp->Release();
	if (!ref)
		delete this;

	return ref;
}

HRESULT __stdcall IPlay4::AddPlayerToGroup(DPID idGroup, DPID idPlayer)
{
	return this->dp->AddPlayerToGroup(idGroup, idPlayer);
}

HRESULT __stdcall IPlay4::Close()
{
	return this->dp->Close();
}

HRESULT __stdcall IPlay4::CreateGroup(LPDPID lpidGroup, LPDPNAME lpGroupName, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags)
{
	return this->dp->CreateGroup(lpidGroup, lpGroupName, lpData, dwDataSize, dwFlags);
}

HRESULT __stdcall IPlay4::CreatePlayer(LPDPID lpidPlayer, LPDPNAME lpPlayerName, HANDLE hEvent, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags)
{
	return this->dp->CreatePlayer(lpidPlayer, lpPlayerName, hEvent, lpData, dwDataSize, dwFlags);
}

HRESULT __stdcall IPlay4::DeletePlayerFromGroup(DPID idGroup, DPID idPlayer)
{
	return this->dp->DeletePlayerFromGroup(idGroup, idPlayer);
}

HRESULT __stdcall IPlay4::DestroyGroup(DPID idGroup)
{
	return this->dp->DestroyGroup(idGroup);
}

HRESULT __stdcall IPlay4::DestroyPlayer(DPID idPlayer)
{
	return this->dp->DestroyPlayer(idPlayer);
}

HRESULT __stdcall IPlay4::EnumGroupPlayers(DPID idGroup, LPGUID lpguidInstance, LPDPENUMPLAYERSCALLBACK2 lpEnumPlayersCallback2, LPVOID lpContext, DWORD dwFlags)
{
	return this->dp->EnumGroupPlayers(idGroup, lpguidInstance, lpEnumPlayersCallback2, lpContext, dwFlags);
}

HRESULT __stdcall IPlay4::EnumGroups(LPGUID lpguidInstance, LPDPENUMPLAYERSCALLBACK2 lpEnumPlayersCallback2, LPVOID lpContext, DWORD dwFlags)
{
	return this->dp->EnumGroups(lpguidInstance, lpEnumPlayersCallback2, lpContext, dwFlags);
}

HRESULT __stdcall IPlay4::EnumPlayers(LPGUID lpguidInstance, LPDPENUMPLAYERSCALLBACK2 lpEnumPlayersCallback2, LPVOID lpContext, DWORD dwFlags)
{
	return this->dp->EnumPlayers(lpguidInstance, lpEnumPlayersCallback2, lpContext, dwFlags);
}

HRESULT IPlay4::EnumSessionsDecorated(LPDPSESSIONDESC2 lpsd, DWORD dwTimeout, LPDPENUMSESSIONSCALLBACK2 lpEnumSessionsCallback2, LPVOID lpContext, DWORD dwFlags)
{
	ULONG_PTR cookie = NULL;
	if (hActCtx && hActCtx != INVALID_HANDLE_VALUE && !ActivateActCtxC(hActCtx, &cookie))
		cookie = NULL;

	HRESULT res = this->dp->EnumSessions(lpsd, dwTimeout, lpEnumSessionsCallback2, lpContext, dwFlags);

	if (cookie)
		DeactivateActCtxC(0, cookie);

	return res;
}

HRESULT __stdcall IPlay4::EnumSessions(LPDPSESSIONDESC2 lpsd, DWORD dwTimeout, LPDPENUMSESSIONSCALLBACK2 lpEnumSessionsCallback2, LPVOID lpContext, DWORD dwFlags)
{
	OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
	if (ddraw && !config.windowedMode && !config.borderlessReal)
	{
		ddraw->RenderStop();
		config.borderlessMode = TRUE;
		ddraw->RenderStart();

		HRESULT res = this->EnumSessionsDecorated(lpsd, dwTimeout, lpEnumSessionsCallback2, lpContext, dwFlags);

		ddraw->RenderStop();
		config.borderlessMode = FALSE;
		ddraw->RenderStart();

		return res;
	}
	else
	{
		RECT rc;
		GetClipCursor(&rc);

		ClipCursor(NULL);
		HRESULT res = this->EnumSessionsDecorated(lpsd, dwTimeout, lpEnumSessionsCallback2, lpContext, dwFlags);
		
		ClipCursor(&rc);

		return res;
	}
}

HRESULT __stdcall IPlay4::GetCaps(LPDPCAPS lpDPCaps, DWORD dwFlags)
{
	return this->dp->GetCaps(lpDPCaps, dwFlags);
}

HRESULT __stdcall IPlay4::GetGroupData(DPID idGroup, LPVOID lpData, LPDWORD lpdwDataSize, DWORD dwFlags)
{
	return this->dp->GetGroupData(idGroup, lpData, lpdwDataSize, dwFlags);
}

HRESULT __stdcall IPlay4::GetGroupName(DPID idGroup, LPVOID lpData, LPDWORD lpdwDataSize)
{
	return this->dp->GetGroupName(idGroup, lpData, lpdwDataSize);
}

HRESULT __stdcall IPlay4::GetMessageCount(DPID idPlayer, LPDWORD lpdwCount)
{
	return this->dp->GetMessageCount(idPlayer, lpdwCount);
}

HRESULT __stdcall IPlay4::GetPlayerAddress(DPID idPlayer, LPVOID lpData, LPDWORD lpdwDataSize)
{
	return this->dp->GetPlayerAddress(idPlayer, lpData, lpdwDataSize);
}

HRESULT __stdcall IPlay4::GetPlayerCaps(DPID idPlayer, LPDPCAPS lpPlayerCaps, DWORD dwFlags)
{
	return this->dp->GetPlayerCaps(idPlayer, lpPlayerCaps, dwFlags);
}

HRESULT __stdcall IPlay4::GetPlayerData(DPID idPlayer, LPVOID lpData, LPDWORD lpdwDataSize, DWORD dwFlags)
{
	return this->dp->GetPlayerData(idPlayer, lpData, lpdwDataSize, dwFlags);
}

HRESULT __stdcall IPlay4::GetPlayerName(DPID idPlayer, LPVOID lpData, LPDWORD lpdwDataSize)
{
	return this->dp->GetPlayerName(idPlayer, lpData, lpdwDataSize);
}

HRESULT __stdcall IPlay4::GetSessionDesc(LPVOID lpData, LPDWORD lpdwDataSize)
{
	return this->dp->GetSessionDesc(lpData, lpdwDataSize);
}

HRESULT __stdcall IPlay4::Initialize(LPGUID lpGUID)
{
	return this->dp->Initialize(lpGUID);
}

HRESULT __stdcall IPlay4::Open(LPDPSESSIONDESC2 lpsd, DWORD dwFlags)
{
	return this->dp->Open(lpsd, dwFlags);
}

HRESULT __stdcall IPlay4::Receive(LPDPID lpidFrom, LPDPID lpidTo, DWORD dwFlags, LPVOID lpData, LPDWORD lpdwDataSize)
{
	return this->dp->Receive(lpidFrom, lpidTo, dwFlags, lpData, lpdwDataSize);
}

HRESULT __stdcall IPlay4::Send(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize)
{
	return this->dp->Send(idFrom, idTo, dwFlags, lpData, dwDataSize);
}

HRESULT __stdcall IPlay4::SetGroupData(DPID idGroup, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags)
{
	return this->dp->SetGroupData(idGroup, lpData, dwDataSize, dwFlags);
}

HRESULT __stdcall IPlay4::SetGroupName(DPID idGroup, LPDPNAME lpGroupName, DWORD dwFlags)
{
	return this->dp->SetGroupName(idGroup, lpGroupName, dwFlags);
}

HRESULT __stdcall IPlay4::SetPlayerData(DPID idPlayer, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags)
{
	return this->dp->SetPlayerData(idPlayer, lpData, dwDataSize, dwFlags);
}

HRESULT __stdcall IPlay4::SetPlayerName(DPID idPlayer, LPDPNAME lpPlayerName, DWORD dwFlags)
{
	return this->dp->SetPlayerName(idPlayer, lpPlayerName, dwFlags);
}

HRESULT __stdcall IPlay4::SetSessionDesc(LPDPSESSIONDESC2 lpSessDesc, DWORD dwFlags)
{
	return this->dp->SetSessionDesc(lpSessDesc, dwFlags);
}

HRESULT __stdcall IPlay4::AddGroupToGroup(DPID idParentGroup, DPID idGroup)
{
	return this->dp->AddGroupToGroup(idParentGroup, idGroup);
}

HRESULT __stdcall IPlay4::CreateGroupInGroup(DPID idParentGroup, LPDPID lpidGroup, LPDPNAME lpGroupName, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags)
{
	return this->dp->CreateGroupInGroup(idParentGroup, lpidGroup, lpGroupName, lpData, dwDataSize, dwFlags);
}

HRESULT __stdcall IPlay4::DeleteGroupFromGroup(DPID idParentGroup, DPID idGroup)
{
	return this->dp->DeleteGroupFromGroup(idParentGroup, idGroup);
}

HRESULT __stdcall IPlay4::EnumConnections(LPCGUID lpguidApplication, LPDPENUMCONNECTIONSCALLBACK lpEnumCallback, LPVOID lpContext, DWORD dwFlags)
{
	return this->dp->EnumConnections(lpguidApplication, lpEnumCallback, lpContext, dwFlags);
}

HRESULT __stdcall IPlay4::EnumGroupsInGroup(DPID idGroup, LPGUID lpguidInstance, LPDPENUMPLAYERSCALLBACK2 lpEnumCallback, LPVOID lpContext, DWORD dwFlags)
{
	return this->dp->EnumGroupsInGroup(idGroup, lpguidInstance, lpEnumCallback, lpContext, dwFlags);
}

HRESULT __stdcall IPlay4::GetGroupConnectionSettings(DWORD dwFlags, DPID idGroup, LPVOID lpData, LPDWORD lpdwDataSize)
{
	return this->dp->GetGroupConnectionSettings(dwFlags, idGroup, lpData, lpdwDataSize);
}

HRESULT __stdcall IPlay4::InitializeConnection(LPVOID lpConnection, DWORD dwFlags)
{
	return this->dp->InitializeConnection(lpConnection, dwFlags);
}

HRESULT __stdcall IPlay4::SecureOpen(LPCDPSESSIONDESC2 lpsd, DWORD dwFlags, LPCDPSECURITYDESC lpSecurity, LPCDPCREDENTIALS lpCredentials)
{
	return this->dp->SecureOpen(lpsd, dwFlags, lpSecurity, lpCredentials);
}

HRESULT __stdcall IPlay4::SendChatMessage(DPID idFrom, DPID idTo, DWORD dwFlags, LPDPCHAT lpChatMessage)
{
	return this->dp->SendChatMessage(idFrom, idTo, dwFlags, lpChatMessage);
}

HRESULT __stdcall IPlay4::SetGroupConnectionSettings(DWORD dwFlags, DPID idGroup, LPDPLCONNECTION lpConnection)
{
	return this->dp->SetGroupConnectionSettings(dwFlags, idGroup, lpConnection);
}

HRESULT __stdcall IPlay4::StartSession(DWORD dwFlags, DPID idGroup)
{
	return this->dp->StartSession(dwFlags, idGroup);
}

HRESULT __stdcall IPlay4::GetGroupFlags(DPID idGroup, LPDWORD lpdwFlags)
{
	return this->dp->GetGroupFlags(idGroup, lpdwFlags);
}

HRESULT __stdcall IPlay4::GetGroupParent(DPID idGroup, LPDPID lpidParent)
{
	return this->dp->GetGroupParent(idGroup, lpidParent);
}

HRESULT __stdcall IPlay4::GetPlayerAccount(DPID idPlayer, DWORD dwFlags, LPVOID lpData, LPDWORD lpdwDataSize)
{
	return this->dp->GetPlayerAccount(idPlayer, dwFlags, lpData, lpdwDataSize);
}

HRESULT __stdcall IPlay4::GetPlayerFlags(DPID idPlayer, LPDWORD lpdwFlags)
{
	return this->dp->GetPlayerFlags(idPlayer, lpdwFlags);
}

HRESULT __stdcall IPlay4::GetGroupOwner(DPID idGroup, LPDPID lpidOwner)
{
	return this->dp->GetGroupOwner(idGroup, lpidOwner);
}

HRESULT __stdcall IPlay4::SetGroupOwner(DPID idGroup, DPID idOwner)
{
	return this->dp->SetGroupOwner(idGroup, idOwner);
}

HRESULT __stdcall IPlay4::SendEx(DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize, DWORD dwPriority, DWORD dwTimeout, LPVOID lpContext, LPDWORD lpdwMsgID)
{
	return this->dp->SendEx(idFrom, idTo, dwFlags, lpData, dwDataSize, dwPriority, dwTimeout, lpContext, lpdwMsgID);
}

HRESULT __stdcall IPlay4::GetMessageQueue(DPID idFrom, DPID idTo, DWORD dwFlags, LPDWORD lpdwNumMsgs, LPDWORD lpdwNumBytes)
{
	return this->dp->GetMessageQueue(idFrom, idTo, dwFlags, lpdwNumMsgs, lpdwNumBytes);
}

HRESULT __stdcall IPlay4::CancelMessage(DWORD dwMsgID, DWORD dwFlags)
{
	return this->dp->CancelMessage(dwMsgID, dwFlags);
}

HRESULT __stdcall IPlay4::CancelPriority(DWORD dwMinPriority, DWORD dwMaxPriority, DWORD dwFlags)
{
	return this->dp->CancelPriority(dwMinPriority, dwMaxPriority, dwFlags);
}