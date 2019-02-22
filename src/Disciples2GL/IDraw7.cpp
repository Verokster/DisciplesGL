#include "stdafx.h"
#include "IDraw7.h"
#include "Main.h"

// Inherited via IDirectDraw7
HRESULT __stdcall IDraw7::Compact() { return DD_OK; }
HRESULT __stdcall IDraw7::CreateClipper(DWORD, IDrawClipper**, IUnknown*) { return DD_OK; }
HRESULT __stdcall IDraw7::CreatePalette(DWORD, LPPALETTEENTRY, IDrawPalette**, IUnknown*) { return DD_OK; }
HRESULT __stdcall IDraw7::CreateSurface(LPDDSURFACEDESC2, IDrawSurface7**, IUnknown*) { return DD_OK; }
HRESULT __stdcall IDraw7::DuplicateSurface(IDrawSurface7*, IDrawSurface7**) { return DD_OK; }
HRESULT __stdcall IDraw7::EnumDisplayModes(DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMMODESCALLBACK2) { return DD_OK; }
HRESULT __stdcall IDraw7::EnumSurfaces(DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMSURFACESCALLBACK7) { return DD_OK; }
HRESULT __stdcall IDraw7::FlipToGDISurface() { return DD_OK; }
HRESULT __stdcall IDraw7::GetCaps(LPDDCAPS, LPDDCAPS) { return DD_OK; }
HRESULT __stdcall IDraw7::GetDisplayMode(LPDDSURFACEDESC2) { return DD_OK; }
HRESULT __stdcall IDraw7::GetFourCCCodes(LPDWORD, LPDWORD) { return DD_OK; }
HRESULT __stdcall IDraw7::GetGDISurface(IDrawSurface7**) { return DD_OK; }
HRESULT __stdcall IDraw7::GetMonitorFrequency(LPDWORD) { return DD_OK; }
HRESULT __stdcall IDraw7::GetScanLine(LPDWORD) { return DD_OK; }
HRESULT __stdcall IDraw7::GetVerticalBlankStatus(LPBOOL) { return DD_OK; }
HRESULT __stdcall IDraw7::Initialize(GUID*) { return DD_OK; }
HRESULT __stdcall IDraw7::RestoreDisplayMode() { return DD_OK; }
HRESULT __stdcall IDraw7::SetCooperativeLevel(HWND, DWORD) { return DD_OK; }
HRESULT __stdcall IDraw7::SetDisplayMode(DWORD, DWORD, DWORD, DWORD, DWORD) { return DD_OK; }
HRESULT __stdcall IDraw7::WaitForVerticalBlank(DWORD, HANDLE) { return DD_OK; }
HRESULT __stdcall IDraw7::GetAvailableVidMem(LPDDSCAPS2, LPDWORD, LPDWORD) { return DD_OK; }
HRESULT __stdcall IDraw7::GetSurfaceFromDC(HDC, IDrawSurface7**) { return DD_OK; }
HRESULT __stdcall IDraw7::RestoreAllSurfaces() { return DD_OK; }
HRESULT __stdcall IDraw7::TestCooperativeLevel() { return DD_OK; }
HRESULT __stdcall IDraw7::GetDeviceIdentifier(LPDDDEVICEIDENTIFIER2, DWORD) { return DD_OK; }
HRESULT __stdcall IDraw7::StartModeTest(LPSIZE, DWORD, DWORD) { return DD_OK; }
HRESULT __stdcall IDraw7::EvaluateMode(DWORD, DWORD*) { return DD_OK; }