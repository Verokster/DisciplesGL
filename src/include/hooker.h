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
// #pragma comment(lib, "hooker.lib")
// #pragma comment(linker, "/DLL /ENTRY:HookMain@12")

enum RedirectType
{
	REDIRECT_CALL = 0xE8,
	REDIRECT_JUMP = 0xE9,
	REDIRECT_JUMP_SHORT = 0xEB
};

typedef VOID* HOOKER;

extern "C"
{
	/// <summary>
	///	Creates hooker object
	/// </summary>
	/// <param name="hModule"></param>
	/// <returns></returns>
	HOOKER __stdcall CreateHooker(HMODULE hModule);

	/// <summary>
	/// Deletes Hooker object
	/// </summary>
	/// <param name="hooker"></param>
	VOID __stdcall ReleaseHooker(HOOKER hooker);

	/// <summary>
	/// Retrives module base address offset
	/// </summary>
	/// <param name="hooker"></param>
	/// <returns></returns>
	DWORD __stdcall GetBaseOffset(HOOKER hooker);

	/// <summary>
	/// Retrives module handle
	/// </summary>
	/// <param name="hooker"></param>
	/// <returns></returns>
	HMODULE __stdcall GetHookerModule(HOOKER hooker);

	/// <summary>
	/// Reads data block
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="block"></param>
	/// <param name="size"></param>
	/// <returns></returns>
	BOOL __stdcall ReadBlock(HOOKER hooker, DWORD address, VOID* block, DWORD size);

	/// <summary>
	/// Reads byte value
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="lpValue"></param>
	/// <returns></returns>
	BOOL __stdcall ReadByte(HOOKER hooker, DWORD address, BYTE* lpValue);

	/// <summary>
	/// Reads word value
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="lpValue"></param>
	/// <returns></returns>
	BOOL __stdcall ReadWord(HOOKER hooker, DWORD address, WORD* lpValue);

	/// <summary>
	/// Reads double word value
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="lpValue"></param>
	/// <returns></returns>
	BOOL __stdcall ReadDWord(HOOKER hooker, DWORD address, DWORD* lpValue);

	/// <summary>
	/// Reads double word value
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="block"></param>
	/// <param name="size"></param>
	/// <returns></returns>
	DWORD __stdcall FindBlock(HOOKER, VOID* block, DWORD size);

	/// <summary>
	/// Write redirect to new address
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="newAddress"></param>
	/// <param name="type"></param>
	/// <param name="nopCount"></param>
	/// <returns></returns>
	BOOL __stdcall PatchRedirect(HOOKER hooker, DWORD address, DWORD newAddress, RedirectType type, DWORD nopCount = 0);

	/// <summary>
	/// Writes jump to new address
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="jumpAddress"></param>
	/// <returns></returns>
	BOOL __stdcall PatchJump(HOOKER hooker, DWORD address, DWORD jumpAddress);

	/// <summary>
	/// Writes jump to new function
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="hookAddress"></param>
	/// <param name="nopCount"></param>
	/// <returns></returns>
	BOOL __stdcall PatchHook(HOOKER hooker, DWORD address, VOID* hookAddress, DWORD nopCount = 0);

	/// <summary>
	/// Writes call to new function
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="funcAddress"></param>
	/// <param name="nopCount"></param>
	/// <returns></returns>
	BOOL __stdcall PatchCall(HOOKER hooker, DWORD address, VOID* funcAddress, DWORD nopCount = 0);

	/// <summary>
	/// Fill bytes by specified value
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="value"></param>
	/// <param name="size"></param>
	/// <returns></returns>
	BOOL __stdcall PatchSet(HOOKER hooker, DWORD address, BYTE value, DWORD size);

	/// <summary>
	/// Fills bytes by no operation (nop) instruction
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="size"></param>
	/// <returns></returns>
	BOOL __stdcall PatchNop(HOOKER hooker, DWORD address, DWORD size);

	/// <summary>
	/// Writes new hex string
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="block"></param>
	/// <returns></returns>
	BOOL __stdcall PatchHex(HOOKER hooker, DWORD address, CHAR* hex);

	/// <summary>
	/// Writes new data block
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="block"></param>
	/// <param name="size"></param>
	/// <returns></returns>
	BOOL __stdcall PatchBlock(HOOKER hooker, DWORD address, VOID* block, DWORD size);

	/// <summary>
	/// Writes new byte value
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="value"></param>
	/// <returns></returns>
	BOOL __stdcall PatchByte(HOOKER hooker, DWORD address, BYTE value);

	/// <summary>
	/// Writes new word value
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="value"></param>
	/// <returns></returns>
	BOOL __stdcall PatchWord(HOOKER hooker, DWORD address, WORD value);

	/// <summary>
	/// Writes new double word value
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="value"></param>
	/// <returns></returns>
	BOOL __stdcall PatchDWord(HOOKER hooker, DWORD address, DWORD value);

	/// <summary>
	/// Redirect relative call to new address
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="funcAddress"></param>
	/// <returns></returns>
	DWORD __stdcall RedirectCall(HOOKER hooker, DWORD address, VOID* funcAddress);

	/// <summary>
	/// Redirects module imported function and retrives old address
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="funcName"></param>
	/// <param name="funcAddress"></param>
	/// <param name="old_value"></param>
	/// <returns></returns>
	DWORD __stdcall PatchImport(HOOKER hooker, const CHAR* funcName, VOID* funcAddress, DWORD* old_value = NULL);

	/// <summary>
	/// Redirects module exported function and retrives old address
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="funcName"></param>
	/// <param name="funcAddress"></param>
	/// <param name="old_value"></param>
	/// <returns></returns>
	DWORD __stdcall PatchExport(HOOKER hooker, const CHAR* funcName, VOID* funcAddress, DWORD* old_value = NULL);

	/// <summary>
	/// Redirects module entry point and retrives old address
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="funcAddress"></param>
	/// <returns></returns>
	DWORD __stdcall PatchEntry(HOOKER hooker, VOID* funcAddress);
	
	/// <summary>
	/// Redirect all imports to other module
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="libName"></param>
	/// <param name="hLib"></param>
	/// <returns></returns>
	VOID __stdcall RedirectImports(HOOKER hooker, const CHAR* libName, HMODULE hLib);
	
	/// <summary>
	/// Map module file into memory
	/// </summary>
	/// <param name="hooker"></param>
	/// <returns></returns>
	BOOL __stdcall MapFile(HOOKER hooker);
	
	/// <summary>
	/// Unmap module file from memory
	/// </summary>
	/// <param name="hooker"></param>
	/// <returns></returns>
	VOID __stdcall UnmapFile(HOOKER hooker);
}