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

#ifdef _HOOKER_LIB
#pragma comment(lib, "hooker.lib")
#pragma comment(linker, "/DLL /ENTRY:HookMain@12")
#endif  // _HOOKER_LIB

enum RedirectType
{
	REDIRECT_CALL = 0xE8,
	REDIRECT_JUMP = 0xE9,
	REDIRECT_JUMP_SHORT = 0xEB
};

typedef VOID* HOOKER;
typedef unsigned __int64 QWORD;

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
	/// Reads pointer value
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="lpValue"></param>
	/// <returns></returns>
	BOOL __stdcall ReadPtr(HOOKER hooker, DWORD address, VOID** lpValue);

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
	/// Reads quad word value
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="lpValue"></param>
	/// <returns></returns>
	BOOL __stdcall ReadQWord(HOOKER hooker, DWORD address, QWORD* lpValue);

	/// <summary>
	/// Reads short value
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="lpValue"></param>
	/// <returns></returns>
	BOOL __stdcall ReadShort(HOOKER hooker, DWORD address, SHORT* lpValue);

	/// <summary>
	/// Reads long value
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="lpValue"></param>
	/// <returns></returns>
	BOOL __stdcall ReadLong(HOOKER hooker, DWORD address, LONG* lpValue);

	/// <summary>
	/// Reads long long value
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="lpValue"></param>
	/// <returns></returns>
	BOOL __stdcall ReadLongLong(HOOKER hooker, DWORD address, LONGLONG* lpValue);

	/// <summary>
	/// Reads float value
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="lpValue"></param>
	/// <returns></returns>
	BOOL __stdcall ReadFloat(HOOKER hooker, DWORD address, FLOAT* lpValue);

	/// <summary>
	/// Reads double float value
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="lpValue"></param>
	/// <returns></returns>
	BOOL __stdcall ReadDouble(HOOKER hooker, DWORD address, DOUBLE* lpValue);

	/// <summary>
	/// Find data block address
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="block"></param>
	/// <param name="size"></param>
	/// <param name="flags"></param>
	/// <param name="start"></param>
	/// <returns></returns>
	DWORD __stdcall FindBlock(HOOKER hooker, const VOID* block, DWORD size, DWORD flags = 0, DWORD start = 0);
	
	/// <summary>
	/// Find data block address by bit mask
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="block"></param>
	/// <param name="mask"></param>
	/// <param name="size"></param>
	/// <param name="flags"></param>
	/// <param name="start"></param>
	/// <returns></returns>
	DWORD __stdcall FindBlockByMask(HOOKER hooker, const VOID* block, const VOID* mask, DWORD size, DWORD flags = 0, DWORD start = 0);
	
	/// <summary>
	/// Find relative function call
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="flags"></param>
	/// <param name="start"></param>
	/// <returns></returns>
	DWORD __stdcall FindCall(HOOKER hooker, DWORD address, DWORD flags = 0, DWORD start = 0);

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
	BOOL __stdcall PatchHook(HOOKER hooker, DWORD address, const VOID* hookAddress, DWORD nopCount = 0);

	/// <summary>
	/// Writes call to new function
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="funcAddress"></param>
	/// <param name="nopCount"></param>
	/// <returns></returns>
	BOOL __stdcall PatchCall(HOOKER hooker, DWORD address, const VOID* funcAddress, DWORD nopCount = 0);

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
	BOOL __stdcall PatchHex(HOOKER hooker, DWORD address, const CHAR* hex);

	/// <summary>
	/// Writes new data block
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="block"></param>
	/// <param name="size"></param>
	/// <returns></returns>
	BOOL __stdcall PatchBlock(HOOKER hooker, DWORD address, const VOID* block, DWORD size);
	
	/// <summary>
	/// Writes new data block by bit mask
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="block"></param>
	/// <param name="mask"></param>
	/// <param name="size"></param>
	/// <returns></returns>
	BOOL __stdcall PatchBlockByMask(HOOKER hooker, DWORD address, const VOID* block, const VOID* mask, DWORD size);

	/// <summary>
	/// Writes new pointer value
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="value"></param>
	/// <returns></returns>
	BOOL __stdcall PatchPtr(HOOKER hooker, DWORD address, const VOID* value);

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
	/// Writes new quad word value
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="value"></param>
	/// <returns></returns>
	BOOL __stdcall PatchQWord(HOOKER hooker, DWORD address, QWORD value);

	/// <summary>
	/// Writes new short value
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="value"></param>
	/// <returns></returns>
	BOOL __stdcall PatchShort(HOOKER hooker, DWORD address, SHORT value);

	/// <summary>
	/// Writes new long value
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="value"></param>
	/// <returns></returns>
	BOOL __stdcall PatchLong(HOOKER hooker, DWORD address, LONG value);

	/// <summary>
	/// Writes new long long value
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="value"></param>
	/// <returns></returns>
	BOOL __stdcall PatchLongLong(HOOKER hooker, DWORD address, LONGLONG value);

	/// <summary>
	/// Writes new float value
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="value"></param>
	/// <returns></returns>
	BOOL __stdcall PatchFloat(HOOKER hooker, DWORD address, FLOAT value);

	/// <summary>
	/// Writes new double float value
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="value"></param>
	/// <returns></returns>
	BOOL __stdcall PatchDouble(HOOKER hooker, DWORD address, DOUBLE value);

	/// <summary>
	/// Redirect relative call to new address
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="address"></param>
	/// <param name="funcAddress"></param>
	/// <returns></returns>
	DWORD __stdcall RedirectCall(HOOKER hooker, DWORD address, const VOID* funcAddress);

	/// <summary>
	/// Redirects module imported function and retrives old address
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="name"></param>
	/// <param name="funcAddress"></param>
	/// <param name="old_value"></param>
	/// <returns></returns>
	DWORD __stdcall PatchImportByName(HOOKER hooker, const CHAR* name, const VOID* funcAddress, DWORD* old_value = NULL);
	
	/// <summary>
	/// Redirects module imported function and retrives old address
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="ordinal"></param>
	/// <param name="funcAddress"></param>
	/// <param name="old_value"></param>
	/// <returns></returns>
	DWORD __stdcall PatchImportByOrdinal(HOOKER hooker, DWORD ordinal, const VOID* funcAddress, DWORD* old_value = NULL);

	/// <summary>
	/// Redirects module exported function and retrives old address
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="name"></param>
	/// <param name="funcAddress"></param>
	/// <param name="old_value"></param>
	/// <returns></returns>
	DWORD __stdcall PatchExport(HOOKER hooker, const CHAR* name, const VOID* funcAddress, DWORD* old_value = NULL);

	/// <summary>
	/// Redirects module entry point and retrives old address
	/// </summary>
	/// <param name="hooker"></param>
	/// <param name="funcAddress"></param>
	/// <returns></returns>
	DWORD __stdcall PatchEntry(HOOKER hooker, const VOID* funcAddress);
	
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
	DWORD __stdcall MapFile(HOOKER hooker);
	
	/// <summary>
	/// Unmap module file from memory
	/// </summary>
	/// <param name="hooker"></param>
	/// <returns></returns>
	VOID __stdcall UnmapFile(HOOKER hooker);
}