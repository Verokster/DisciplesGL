#include "stdafx.h"
#include "IDrawSurface7.h"
#include "IDrawClipper.h"
#include "IDraw7.h"

// Inherited via IDrawSurface7
HRESULT __stdcall IDrawSurface7::AddAttachedSurface(IDrawSurface7*) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::AddOverlayDirtyRect(LPRECT) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::Blt(LPRECT, IDrawSurface7*, LPRECT, DWORD, LPDDBLTFX) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::BltBatch(LPDDBLTBATCH, DWORD, DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::BltFast(DWORD, DWORD, IDrawSurface7*, LPRECT, DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::DeleteAttachedSurface(DWORD, IDrawSurface7*) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::EnumAttachedSurfaces(LPVOID, LPDDENUMSURFACESCALLBACK7) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::EnumOverlayZOrders(DWORD, LPVOID, LPDDENUMSURFACESCALLBACK7) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::Flip(IDrawSurface7*, DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetAttachedSurface(LPDDSCAPS2, IDrawSurface7**) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetBltStatus(DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetCaps(LPDDSCAPS2) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetClipper(IDrawClipper**) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetColorKey(DWORD, LPDDCOLORKEY) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetDC(HDC*) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetFlipStatus(DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetOverlayPosition(LPLONG, LPLONG) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetPalette(IDrawPalette**) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetPixelFormat(LPDDPIXELFORMAT) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetSurfaceDesc(LPDDSURFACEDESC2) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::Initialize(LPDIRECTDRAW, LPDDSURFACEDESC2) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::IsLost() { return DD_OK; }
HRESULT __stdcall IDrawSurface7::Lock(LPRECT, LPDDSURFACEDESC2, DWORD, HANDLE) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::ReleaseDC(HDC) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::Restore() { return DD_OK; }
HRESULT __stdcall IDrawSurface7::SetClipper(IDrawClipper*) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::SetColorKey(DWORD, LPDDCOLORKEY) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::SetOverlayPosition(LONG, LONG) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::SetPalette(IDrawPalette*) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::Unlock(LPRECT) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::UpdateOverlay(LPRECT, IDrawSurface7*, LPRECT, DWORD, LPDDOVERLAYFX) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::UpdateOverlayDisplay(DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::UpdateOverlayZOrder(DWORD, IDrawSurface7*) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetDDInterface(LPVOID*) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::PageLock(DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::PageUnlock(DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::SetSurfaceDesc(LPDDSURFACEDESC2, DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::SetPrivateData(REFGUID, LPVOID, DWORD, DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetPrivateData(REFGUID, LPVOID, LPDWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::FreePrivateData(REFGUID) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetUniquenessValue(LPDWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::ChangeUniquenessValue() { return DD_OK; }
HRESULT __stdcall IDrawSurface7::SetPriority(DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetPriority(LPDWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::SetLOD(DWORD) { return DD_OK; }
HRESULT __stdcall IDrawSurface7::GetLOD(LPDWORD) { return DD_OK; }