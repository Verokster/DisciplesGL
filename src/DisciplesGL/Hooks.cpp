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
#include "Shellapi.h"
#include "Objbase.h"
#include "Mmsystem.h"
#include "Commdlg.h"
#include "process.h"
#include "Hooks.h"
#include "Main.h"
#include "Config.h"
#include "Resource.h"
#include "Window.h"
#include "PngLib.h"
#include "IPlay4.h"
#include "MappedFile.h"

#define CHECKVALUE (WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX)

namespace Hooks
{
	HMODULE hModule;
	INT baseOffset;

	const BYTE nop4[] = { 0x83, 0xC4, 0x04, 0x90, 0x90 };

	const AddressSpaceV1 addressArrayV1[] = {
		// v99.12.15.1
		0x004CD137, WS_POPUP, 0x004DFFA4, 0x00000000, 0x004E06D3,
		0x0058F7E3,
		0x005E6320, 0x005DB2A0, 0x004E21B5, 0x00550B69, 0x00465296, 0x004F07F5, 0x004F1006, 0x004F12AA,

		// v2000.6.22.1 - Eng
		0x004CD13E, WS_POPUP, 0x004DFF9C, 0x00000000, 0x004E0759,
		0x0058F0D3,
		0x005E6358, 0x005DB2D8, 0x004E225C, 0x0055037A, 0x0046523E, 0x004F07B0, 0x004F0FB6, 0x004F125A,

		// v2000.6.22.1 - Rus1
		0x004CD408, WS_POPUP, 0x004E0323, 0x00000000, 0x004E0AA8,
		0x00590E03,
		0x005E8430, 0x005DD278, 0x004E2565, 0x0055106A, 0x0046541D, 0x004F0B39, 0x004F12D6, 0x004F157A,

		// v2000.6.22.1 Editor
		0x00443EDE, WS_POPUP, 0x0045147D, 0x004265E3, 0x00451BE8,
		0x00000000,
		0x00519568, 0x00000000, 0x004E2565, 0x004A041A, 0x00000000, 0x0046229D, 0x00462AF6, 0x00462D9A
	};

	const AddressSpaceV2 addressArrayV2[] = {
#pragma region v1 .10
		// v1.10
		1, 0x00558687, CHECKVALUE, 0x00000000, 0x00000000, 96, 0x005BBF85,
		0x0065C991,

		0x005ADFCC, 0x0059A30B, 0x005AECAF, 0x006622BE, 0x0066242E, 0x0059A203,
		0x006D3728, 0x00552876, 0x0066378C, 0x00552A0C, 0x006CD194, 0x005A98B0,
		0x005A97B0, 0x0051E67B, 0x005A10A1, 0x006CCD00, 0x005261BF, 0x00526258,
		0x0052DBEE, 0x0052DC2D, 0x0052DC36, 0x0052DC88,

		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000,

		0x00000000, 0x0053BBFE, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x0053C619, 0x0053DD7C, 0x0053CA81, 0x0053C255, 0x0053C7A3, 0x0053C4E1, 0x0053CC52, 0x0053D35C,

		0x00532618, 0x00532D00, 0x00000000, 0x00000000, 0x00532DA2, 0x0052FB85,

		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x005A3079, 0x005A30C2, 0x005A310B, 0x005200AA,

		0x0048ACA4, 0x0053607F, 0x005A22B1, 0x00531627,
		0x00000000, 0x00000000, 0x00000000, 0x00480A18,
		0x00487F80, 0x004059C7, 0x004857C7,

		0x00000000, 0x00000000, 0x00000000, 0x005094B1, 0x00509575,
#pragma endregion

#pragma region v1 .40
		// v1.40
		1, 0x0055EA82, CHECKVALUE, 0x00000000, 0x00000000, 144, 0x005C293F,
		0x006665C1,

		0x005B4516, 0x005A0826, 0x005B5216, 0x0066BEBB, 0x0066C014, 0x005A071E,
		0x006DFD98, 0x00558982, 0x0066D37F, 0x00558B18, 0x006D950C, 0x005AFD70,
		0x005AFC70, 0x00523979, 0x005A740F, 0x006D9078, 0x0052BD52, 0x0052BDEB,
		0x005339AF, 0x005339EE, 0x005339F7, 0x00533A49,

		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000,

		0x00000000, 0x00541C5E, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00542679, 0x00543DDC, 0x00542AE1, 0x005422B5, 0x00542803, 0x00542541, 0x00542CB2, 0x005433BC,

		0x00538701, 0x00538DE6, 0x00000000, 0x00000000, 0x00538E88, 0x00535C7B,

		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x005A9402, 0x005A944B, 0x005A9494, 0x00525390,

		0x0048D187, 0x004C2423, 0x005A863F, 0x00537712,
		0x00000000, 0x00000000, 0x00000000, 0x00482654,
		0x0048A53D, 0x004059F2, 0x00487CEB,

		0x00000000, 0x00000000, 0x00000000, 0x0050E461, 0x0050E525,
#pragma endregion

#pragma region v2 .01
		// v2.01
		2, 0x005645FD, CHECKVALUE, 0x00000000, 0x00000000, 144, 0x005C97D9,
		0x0066FFC1,

		0x005BB2A9, 0x005A7434, 0x005BC059, 0x006758C0, 0x00675A27, 0x005A732C,
		0x006EA4E8, 0x0055E55E, 0x00676D96, 0x0055E6F4, 0x006E398C, 0x005B69F0,
		0x005B68F0, 0x00529178, 0x005AE075, 0x006E34C8, 0x005317C9, 0x00531862,
		0x00539521, 0x00539560, 0x00539569, 0x005395BB,

		0x0061331D, 0x00613381, 0x00614CC5, 0x005646BF, 0x0040263B, 0x0053A1BD,
		0x00515A22, 0x00519FF0, 0x00515112, 0x005C829E, 0x005C825F, 0x00488C30, 0x004859E2,
		0x00515E2F, 0x00515F3E, 0x00669434,

		0x00546870, 0x0054789E, 0x005476F9, 0x00547344, 0x00546A5B, 0x005472F7,
		0x005482B9, 0x00549A1C, 0x00548721, 0x00547EF5, 0x00548443, 0x00548181, 0x005488F2, 0x00548FFC,

		0x0053E350, 0x0053EAC6, 0x00000000, 0x00000000, 0x0053EB68, 0x0053B927,

		0x00625C15, 0x0063FB98, 0x00626D03, 0x006265AF, 0x00629F9A, 0x00630BA4, 0x0062689D, 0x00630C38,
		0x0062752E, 0x0062757D, 0x006275AB, 0x006279DD, 0x00627A2C, 0x00627A5A,
		0x00626A07, 0x00625B4A, 0x00625DEC, 0x00000000,
		0x005B008C, 0x005B00D5, 0x005B011E, 0x0052AB8A,

		0x0048FAB4, 0x0056CEEE, 0x005AF2A4, 0x0053D265,
		0x0052FD28, 0x00484900, 0x00484922, 0x004847FF,
		0x0048CD6F, 0x00405F03, 0x0048A44F,

		0x00450226, 0x004502B3, 0x0040A789, 0x00513981, 0x00513A45,
#pragma endregion

#pragma region v2 .01 - Steam
		// v2.01 - Steam
		2, 0x00564D69, CHECKVALUE, 0x00000000, 0x00000000, 144, 0x005C9BFA,
		0x00672351,

		0x005BB7DE, 0x005A79EA, 0x005BC568, 0x00663E55, 0x00663FBC, 0x005A78E2,
		0x006EA690, 0x0055EC4C, 0x006652F8, 0x0055EDE2, 0x006E39DC, 0x005B6F70,
		0x005B6E70, 0x0052983E, 0x005AE623, 0x006E3518, 0x00531EE5, 0x00531F7E,
		0x00539C09, 0x00539C48, 0x00539C51, 0x00539CA3,

		0x00613712, 0x00613776, 0x00614FAB, 0x00564E2B, 0x00402253, 0x0053A89E,
		0x00516222, 0x0051A760, 0x00515912, 0x005C86BF, 0x005C8680, 0x00488CC7, 0x00485A69,
		0x0051662F, 0x0051673E, 0x0066B804,

		0x00546F70, 0x00547F9E, 0x00547DF9, 0x00547A44, 0x0054715B, 0x005479F7,
		0x005489B9, 0x0054A11C, 0x00548E21, 0x005485F5, 0x00548B43, 0x00548881, 0x00548FF2, 0x005496FC,

		0x0053EA15, 0x0053F162, 0x00000000, 0x00000000, 0x0053F204, 0x0053C02B,

		0x00625FB5, 0x0063FE68, 0x006270A3, 0x0062694F, 0x0062A33A, 0x00630F44, 0x00626C3D, 0x00630FD8,
		0x006278CE, 0x0062791D, 0x0062794B, 0x00627D7D, 0x00627DCC, 0x00627DFA,
		0x00626DA7, 0x00625EEA, 0x0062618C, 0x0050B3AF,
		0x005B065B, 0x005B06A4, 0x005B06ED, 0x0052B25E,

		0x0048FB37, 0x005AA77E, 0x005AF894, 0x0053D936,
		0x00530462, 0x00484960, 0x00484982, 0x0048485F,
		0x0048CE0E, 0x00405B80, 0x0048A4EE,

		0x0045010A, 0x00450197, 0x0040A4AE, 0x005141A1, 0x00514265,
#pragma endregion

#pragma region v2 .02
		// v2.02
		2, 0x0056339E, CHECKVALUE, 0x00000000, 0x00000000, 144, 0x005C848D,
		0x0066E831,

		0x005B9F86, 0x005A6311, 0x005BACE1, 0x0067427E, 0x00674400, 0x005A6209,
		0x006E9408, 0x0055D283, 0x00675771, 0x0055D419, 0x006E2894, 0x005B5660,
		0x005B5560, 0x005280D9, 0x005ACF11, 0x006E23D0, 0x0053056A, 0x00530603,
		0x0053835D, 0x0053839C, 0x005383A5, 0x005383F7,

		0x00611C8B, 0x00611CEF, 0x00613523, 0x00563460, 0x00402129, 0x00538FEB,
		0x00514DD2, 0x005193F0, 0x005144C2, 0x005C6F52, 0x005C6F13, 0x00489124, 0x00485ED6,
		0x005151DF, 0x005152EE, 0x00667DC4,

		0x00545670, 0x0054669E, 0x005464F9, 0x00546144, 0x0054585B, 0x005460F7,
		0x005470B9, 0x0054881C, 0x00547521, 0x00546CF5, 0x00547243, 0x00546F81, 0x005476F2, 0x00547DFC,

		0x0053D185, 0x0053D8A9, 0x00000000, 0x00000000, 0x0053D94B, 0x0053A73D,

		0x00624595, 0x0063E6B8, 0x00625683, 0x00624F2F, 0x0062891A, 0x0062F524, 0x0062521D, 0x0062F5B8,
		0x00625EAE, 0x00625EFD, 0x00625F2B, 0x0062635D, 0x006263AC, 0x006263DA,
		0x00625387, 0x006244CA, 0x0062476C, 0x00000000,
		0x005AEF44, 0x005AEF8D, 0x005AEFD6, 0x00529AEB,

		0x0048FF90, 0x004F146A, 0x005AE161, 0x0053C095,
		0x0052EAAE, 0x00484DD1, 0x00484DF3, 0x00484CD0,
		0x0048D25C, 0x00405A1A, 0x0048A93C,

		0x00450417, 0x004504A4, 0x0040A438, 0x00512D71, 0x00512E35,
#pragma endregion

#pragma region v2 .02 - Crack
		// v2.02 - Crack
		2, 0x00564C4D, CHECKVALUE, 0x00000000, 0x00000000, 144, 0x005C9B4B,
		0x0066FFE1,

		0x005BB74F, 0x005A7BFA, 0x005BC465, 0x006758E0, 0x00675A47, 0x005A7AF2,
		0x006EA4F0, 0x0055EBAE, 0x00676DB6, 0x0055ED44, 0x006E398C, 0x005B6F90,
		0x005B6E90, 0x00529698, 0x005AE83B, 0x006E34C8, 0x00531C99, 0x00531D32,
		0x005399F1, 0x00539A30, 0x00539A39, 0x00539A8B,

		0x00613530, 0x00613594, 0x00614DD2, 0x00564D0F, 0x0040254B, 0x0053A68D,
		0x00515F52, 0x0051A520, 0x00515642, 0x005C8610, 0x005C85D1, 0x004892E5, 0x00486097,
		0x0051635F, 0x0051646E, 0x00669454,

		0x00546EC0, 0x00547EEE, 0x00547D49, 0x00547994, 0x005470AB, 0x00547947,
		0x00548909, 0x0054A06C, 0x00548D71, 0x00548545, 0x00548A93, 0x005487D1, 0x00548F42, 0x0054964C,

		0x0053E820, 0x0053EF96, 0x00000000, 0x00000000, 0x0053F038, 0x0053BDF7,

		0x00625D25, 0x0063FCA8, 0x00626E13, 0x006266BF, 0x0062A0AA, 0x00630CB4, 0x006269AD, 0x00630D48,
		0x0062763E, 0x0062768D, 0x006276BB, 0x00627AED, 0x00627B3C, 0x00627B6A,
		0x00626B17, 0x00625C5A, 0x00625EFC, 0x00000000,
		0x005B0852, 0x005B089B, 0x005B08E4, 0x0052B0AA,

		0x00490169, 0x0056D6B4, 0x005AFA6A, 0x0053D735,
		0x005301F8, 0x00484FB5, 0x00484FD7, 0x00484EB4,
		0x0048D424, 0x00405F19, 0x0048AB04,

		0x004506BD, 0x0045074A, 0x0040A79F, 0x00513EB1, 0x00513F75,
#pragma endregion

#pragma region v3 .00 - Steam
		// v3.00 - Steam
		3, 0x005674DF, CHECKVALUE, 0x00000000, 0x00000000, 144, 0x005CD93F,
		0x00678C51,

		0x005BF272, 0x005AB0A4, 0x005C0048, 0x0067E78C, 0x0067E8D5, 0x005AAF9C,
		0x006F4E40, 0x005614BE, 0x0067FC5F, 0x00561654, 0x006EDE44, 0x005BA970,
		0x005BA870, 0x0052CB61, 0x005B1CE2, 0x006ED978, 0x00534FFD, 0x00535096,
		0x0053CD9E, 0x0053CDDD, 0x0053CDE6, 0x0053CE38,

		0x00619B3D, 0x00619BA2, 0x0061B492, 0x005675A1, 0x004021CE, 0x0053DA29,
		0x00519372, 0x0051D940, 0x00518A62, 0x005CC404, 0x005CC3C5, 0x0048BF6E, 0x00488BB0,
		0x0051977F, 0x0051988E, 0x006721C4,

		0x00549EE0, 0x0054AF0E, 0x0054AD69, 0x0054A9B4, 0x0054A0CB, 0x0054A967,
		0x0054B929, 0x0054D08C, 0x0054BD91, 0x0054B565, 0x0054BAB3, 0x0054B7F1, 0x0054BF62, 0x0054C66C,

		0x00541BA5, 0x005422C9, 0x00000000, 0x00000000, 0x0054236B, 0x0053F1A4,

		0x0062D6C5, 0x00647508, 0x0062E84E, 0x0062E0C8, 0x00631C1E, 0x00638BD8, 0x0062E3E8, 0x00638CFD,
		0x0062F13B, 0x0062F188, 0x0062F1B6, 0x0062F5E9, 0x0062F63C, 0x0062F670,
		0x0062E552, 0x0062D5FA, 0x0062D905, 0x0050E535,
		0x005B3D11, 0x005B3D5A, 0x005B3DA3, 0x0052E569,

		0x00492DB4, 0x00581521, 0x005B2F1A, 0x00540AC6,
		0x00533515, 0x00487AAD, 0x00487ACF, 0x004879AC,
		0x00490052, 0x00405929, 0x0048D732,

		0x0045257F, 0x0045260C, 0x0040A3D9, 0x00517311, 0x005173D5,
#pragma endregion

#pragma region v3 .01a
		// v3.01a and Crack
		3, 0x005676DB, CHECKVALUE, 0x00000000, 0x00000000, 144, 0x005CDF6F,
		0x00679991,

		0x005BF7E4, 0x005AB6D8, 0x005C0654, 0x0067F2BE, 0x0067F42A, 0x005AB5D0,
		0x006F5E50, 0x005616BE, 0x006807FE, 0x00561854, 0x006EEE74, 0x005BAFC0,
		0x005BAEC0, 0x0052CAA6, 0x005B22A1, 0x006EE9A8, 0x00535048, 0x005350E1,
		0x0053CE21, 0x0053CE60, 0x0053CE69, 0x0053CEBB,

		0x0061A6C5, 0x0061A72A, 0x0061C023, 0x0056779D, 0x00402444, 0x0053DAAC,
		0x005191B2, 0x0051D6E0, 0x005188A2, 0x005CCA34, 0x005CC9F5, 0x0048BF35, 0x00488B9E,
		0x005195BF, 0x005196CE, 0x00672E84,

		0x0054A0E0, 0x0054B10E, 0x0054AF69, 0x0054ABB4, 0x0054A2CB, 0x0054AB67,
		0x0054BB29, 0x0054D28C, 0x0054BF91, 0x0054B765, 0x0054BCB3, 0x0054B9F1, 0x0054C162, 0x0054C86C,

		0x00541C2D, 0x0054237C, 0x00000000, 0x00000000, 0x0054241E, 0x0053F237,

		0x0062E345, 0x006482A8, 0x0062F4CE, 0x0062ED48, 0x0063289E, 0x00639858, 0x0062F068, 0x0063997D,
		0x0062FDBB, 0x0062FE08, 0x0062FE36, 0x00630269, 0x006302BC, 0x006302F0,
		0x0062F1D2, 0x0062E27A, 0x0062E585, 0x00000000,
		0x005B4308, 0x005B4351, 0x005B439A, 0x0052E4C0,

		0x00492EEA, 0x00403FC0, 0x005B3528, 0x00540B48,
		0x005335EC, 0x00487AB9, 0x00487ADB, 0x004879B8,
		0x004900C8, 0x00405C95, 0x0048D7A8,

		0x004526E8, 0x00452775, 0x0040A79D, 0x00517111, 0x005171D5,
#pragma endregion

#pragma region v3 .01b
		// v3.01b
		3, 0x00566E05, CHECKVALUE, 0x00000000, 0x00000000, 144, 0x005CCE8B,
		0x006781F1,

		0x005BE82F, 0x005AA960, 0x005BF589, 0x0067DC5A, 0x0067DDDC, 0x005AA858,
		0x006F3E00, 0x00560E5B, 0x0067F172, 0x00560FF1, 0x006ECE14, 0x005B9F70,
		0x005B9E70, 0x0052C03C, 0x005B15A2, 0x006EC948, 0x00534626, 0x005346BF,
		0x0053C4CD, 0x0053C50C, 0x0053C515, 0x0053C567,

		0x006191E3, 0x00619248, 0x0061AB38, 0x00566EC7, 0x0040218A, 0x0053D158,
		0x00518742, 0x0051CCA0, 0x00517E32, 0x005CB950, 0x005CB911, 0x0048BB1E, 0x00488787,
		0x00518B4F, 0x00518C5E, 0x00671694,

		0x005497F0, 0x0054A81E, 0x0054A679, 0x0054A2C4, 0x005499DB, 0x0054A277,
		0x0054B239, 0x0054C99C, 0x0054B6A1, 0x0054AE75, 0x0054B3C3, 0x0054B101, 0x0054B872, 0x0054BF7C,

		0x005412DD, 0x00541A01, 0x00000000, 0x00000000, 0x00541AA3, 0x0053E89F,

		0x0062CD85, 0x00646B28, 0x0062DF0E, 0x0062D788, 0x006312DE, 0x00638298, 0x0062DAA8, 0x006383BD,
		0x0062E7FB, 0x0062E848, 0x0062E876, 0x0062ECA9, 0x0062ECFC, 0x0062ED30,
		0x0062DC12, 0x0062CCBA, 0x0062CFC5, 0x00000000,
		0x005B35EA, 0x005B3633, 0x005B367C, 0x0052DA66,

		0x00492942, 0x0041B1B3, 0x005B27EE, 0x005401F9,
		0x00532B79, 0x0048768A, 0x004876AC, 0x00487589,
		0x0048FBCC, 0x00405921, 0x0048D2E2,

		0x004520B4, 0x00452141, 0x0040A3F9, 0x00516621, 0x005166E5,
#pragma endregion

#pragma region v1 .10 Editor
		// v1.10 Editor
		1, 0x00472638, CHECKVALUE, 0x00000000, 0x00000480, 0, 0x00000000,
		0x0049E08D,

		0x0053C64D, 0x0052CE6F, 0x0053D324, 0x0056FC19, 0x0056FD80, 0x0052CD67,
		0x005B8000, 0x0055E8C3, 0x0057114B, 0x0055EA59, 0x005B56C4, 0x00536B10,
		0x00536A10, 0x004A6BE2, 0x005348C6, 0x005B52E0, 0x0047532E, 0x004753C7,
		0x0055FF5C, 0x0055FF9B, 0x0055FFA4, 0x0055FFF6,

		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000,

		0x00000000, 0x0054D87E, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x0054E299, 0x0054F9FC, 0x0054E701, 0x0054DED5, 0x0054E423, 0x0054E161, 0x0054E8D2, 0x0054EFDC,

		0x004813D2, 0x00481AE4, 0x00446339, 0x0045FBD4, 0x00481B86, 0x0047E658,

		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,

		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000,

		0x00000000, 0x00000000, 0x00000000, 0x004947C1, 0x00494885,
#pragma endregion

#pragma region v1 .41 Editor
		// v1.14 Editor
		1, 0x004728EF, CHECKVALUE, 0x00000000, 0x00000480, 0, 0x00000000,
		0x0049F55E,

		0x0053FAC7, 0x0052FBE8, 0x005407DC, 0x005736A4, 0x005737F5, 0x0052FAE0,
		0x005BCDE0, 0x00561ADF, 0x00574BB4, 0x00561C75, 0x005BA60C, 0x00539EE0,
		0x00539DE0, 0x004A82A9, 0x005378A2, 0x005BA070, 0x00475664, 0x004756FD,
		0x00563339, 0x00563378, 0x00563381, 0x005633D3,

		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000,

		0x00000000, 0x00550A9E, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x005514B9, 0x00552C1C, 0x00551921, 0x005510F5, 0x00551643, 0x00551381, 0x00551AF2, 0x005521FC,

		0x004823D1, 0x00482AC0, 0x00446667, 0x004601FA, 0x00482B62, 0x0047F6A4,

		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,

		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000,

		0x00000000, 0x00000000, 0x00000000, 0x00495941, 0x00495A05,
#pragma endregion

#pragma region v2 .00 Editor - Steam
		// v2.00 Editor - Steam
		2, 0x00489D25, CHECKVALUE, 0x00000000, 0x00000000, 144, 0x004D4257,
		0x004B7025,

		0x0055C4B6, 0x0054C79A, 0x0055D13A, 0x00591E06, 0x00591F6A, 0x0054C68E,
		0x005DEDE0, 0x0057F12A, 0x005932E9, 0x0057F2C0, 0x005DC574, 0x005568D0,
		0x005567D0, 0x004BFB0E, 0x0055429D, 0x005DBFA8, 0x0048CA22, 0x0048CABB,
		0x0058097B, 0x005809BA, 0x005809C3, 0x00580A15,

		0x0051B64F, 0x0051B6B3, 0x005190D5, 0x00489DE7, 0x00000000, 0x0049435D,
		0x004AF3B2, 0x004B38E0, 0x004AEAA2, 0x004D2597, 0x004D2558, 0x00000000, 0x00000000,
		0x004AF7BF, 0x004AF8CE, 0x00584EE4,

		0x0056D0D0, 0x0056E0FE, 0x0056DF59, 0x0056DBA4, 0x0056D2BB, 0x0056DB57,
		0x0056EB19, 0x0057027C, 0x0056EF81, 0x0056E755, 0x0056ECA3, 0x0056E9E1, 0x0056F152, 0x0056F85C,

		0x00499B8E, 0x0049A2D0, 0x00448D1C, 0x0046320E, 0x0049A372, 0x00496D92,

		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,

		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000,

		0x00000000, 0x00000000, 0x00000000, 0x004AD0E1, 0x004AD1A5,
#pragma endregion

#pragma region v2 .01 Editor
		// v2.01 Editor
		2, 0x004893BD, CHECKVALUE, 0x00000000, 0x00000000, 144, 0x004D38B1,
		0x004B66EB,

		0x0055B9F7, 0x0054C08C, 0x0055C66B, 0x00591270, 0x005913E0, 0x0054BF84,
		0x005DDD70, 0x0057E23F, 0x005927BD, 0x0057E3D5, 0x005DB524, 0x00555F10,
		0x00555E10, 0x004BF0E2, 0x005538E4, 0x005DAF58, 0x0048C109, 0x0048C1A2,
		0x0057FA99, 0x0057FAD8, 0x0057FAE1, 0x0057FB33,

		0x0051ADF0, 0x0051AE54, 0x005188BC, 0x0048947F, 0x00000000, 0x00493A1D,
		0x004AEAC2, 0x004B2FE0, 0x004AE1B2, 0x004D1BF1, 0x004D1BB2, 0x00000000, 0x00000000,
		0x004AEECF, 0x004AEFDE, 0x005842E4,

		0x0056C1D0, 0x0056D1FE, 0x0056D059, 0x0056CCA4, 0x0056C3BB, 0x0056CC57,
		0x0056DC19, 0x0056F37C, 0x0056E081, 0x0056D855, 0x0056DDA3, 0x0056DAE1, 0x0056E252, 0x0056E95C,

		0x0049925F, 0x00499A08, 0x00448D8C, 0x0046346A, 0x00499AAA, 0x00496412,

		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,

		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000,

		0x00000000, 0x00000000, 0x00000000, 0x004AC811, 0x004AC8D5,
#pragma endregion

#pragma region v2 .02 Editor
		// v2.02 Editor
		2, 0x004893BD, CHECKVALUE, 0x0054E7D3, 0x00000480, 144, 0x004D38B1,
		0x004B66EB,

		0x0055BB17, 0x0054C1AC, 0x0055C78B, 0x00591379, 0x005914E9, 0x0054C0A4,
		0x005DDD78, 0x0057E35F, 0x005928C6, 0x0057E4F5, 0x005DB52C, 0x00556030,
		0x00555F30, 0x004BF0E2, 0x00553A04, 0x005DAF60, 0x0048C109, 0x0048C1A2,
		0x0057FBB9, 0x0057FBF8, 0x0057FC01, 0x0057FC53,

		0x0051AE07, 0x0051AE6B, 0x005188BC, 0x0048947F, 0x00000000, 0x00493A1D,
		0x004AEAC2, 0x004B2FE0, 0x004AE1B2, 0x004D1BF1, 0x004D1BB2, 0x00000000, 0x00000000,
		0x004AEECF, 0x004AEFDE, 0x00584404,

		0x0056C2F0, 0x0056D31E, 0x0056D179, 0x0056CDC4, 0x0056C4DB, 0x0056CD77,
		0x0056DD39, 0x0056F49C, 0x0056E1A1, 0x0056D975, 0x0056DEC3, 0x0056DC01, 0x0056E372, 0x0056EA7C,

		0x0049925F, 0x00499A08, 0x00448D8C, 0x0046346A, 0x00499AAA, 0x00496412,

		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,

		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000,

		0x00000000, 0x00000000, 0x00000000, 0x004AC811, 0x004AC8D5,
#pragma endregion

#pragma region v3 .00 Editor - Steam
		// v3.00 Editor - Steam
		3, 0x0048C842, CHECKVALUE, 0x00000000, 0x00000000, 144, 0x004D71D0,
		0x004B9C35,

		0x0056234B, 0x005527FA, 0x00562FDC, 0x00597EDA, 0x00598050, 0x005526EE,
		0x005E6700, 0x00585176, 0x005993B6, 0x0058530C, 0x005E3E24, 0x0055C7D0,
		0x0055C6D0, 0x004C264A, 0x0055A1A3, 0x005E3850, 0x0048F5B3, 0x0048F64C,
		0x005869CA, 0x00586A09, 0x00586A12, 0x00586A64,

		0x0052019C, 0x00520201, 0x0051DBBB, 0x0048C904, 0x00000000, 0x00496F6F,
		0x004B1F82, 0x004B64F0, 0x004B1672, 0x004D550E, 0x004D54CF, 0x00000000, 0x00000000,
		0x004B238F, 0x004B249E, 0x0058AF54,

		0x00573120, 0x0057414E, 0x00573FA9, 0x00573BF4, 0x0057330B, 0x00573BA7,
		0x00574B69, 0x005762CC, 0x00574FD1, 0x005747A5, 0x00574CF3, 0x00574A31, 0x005751A2, 0x005758AC,

		0x0049C83A, 0x0049CF68, 0x0044AB98, 0x0046555C, 0x0049D00A, 0x004999FB,

		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,

		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000,

		0x00000000, 0x00000000, 0x00000000, 0x004AFD41, 0x004AFE05,
#pragma endregion

#pragma region v3 .01 Editor
		// v3.01 Editor
		3, 0x0048BA1B, CHECKVALUE, 0x00000000, 0x00000000, 144, 0x004D642A,
		0x004B8BFF,

		0x00561820, 0x00551CA9, 0x005624CD, 0x00597125, 0x0059728C, 0x00551BA1,
		0x005E5660, 0x0058422F, 0x005986BD, 0x005843C5, 0x005E2D9C, 0x0055BCF0,
		0x0055BBF0, 0x004C1729, 0x0055969C, 0x005E27C8, 0x0048E6DF, 0x0048E778,
		0x00585A83, 0x00585AC2, 0x00585ACB, 0x00585B1D,

		0x0051F560, 0x0051F5C5, 0x0051CF95, 0x0048BADD, 0x00000000, 0x0049604C,
		0x004B1002, 0x004B5560, 0x004B06F2, 0x004D4768, 0x004D4729, 0x00000000, 0x00000000,
		0x004B140F, 0x004B151E, 0x0058A1B4,

		0x005721C0, 0x005731EE, 0x00573049, 0x00572C94, 0x005723AB, 0x00572C47,
		0x00573C09, 0x0057536C, 0x00574071, 0x00573845, 0x00573D93, 0x00573AD1, 0x00574242, 0x0057494C,

		0x0049B857, 0x0049BFFF, 0x0044A8E6, 0x00465353, 0x0049C0A1, 0x00498A27,

		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,

		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000,

		0x00000000, 0x00000000, 0x00000000, 0x004AED61, 0x004AEE25
#pragma endregion
	};

#pragma region Hook helpers
#pragma optimize("s", on)
	BOOL __fastcall PatchNop(DWORD addr, DWORD size)
	{
		DWORD address = addr + baseOffset;

		DWORD old_prot;
		if (VirtualProtect((VOID*)address, size, PAGE_EXECUTE_READWRITE, &old_prot))
		{
			MemorySet((VOID*)address, 0x90, size);
			VirtualProtect((VOID*)address, size, old_prot, &old_prot);

			return TRUE;
		}
		return FALSE;
	}

	BOOL __fastcall PatchRedirect(DWORD addr, VOID* hook, BYTE instruction, DWORD nop)
	{
		DWORD address = addr + baseOffset;

		DWORD size = instruction == 0xEB ? 2 : 5;

		DWORD old_prot;
		if (VirtualProtect((VOID*)address, size + nop, PAGE_EXECUTE_READWRITE, &old_prot))
		{
			BYTE* jump = (BYTE*)address;
			*jump = instruction;
			++jump;
			*(DWORD*)jump = (DWORD)hook - (DWORD)address - size;

			if (nop)
				MemorySet((VOID*)(address + size), 0x90, nop);

			VirtualProtect((VOID*)address, size + nop, old_prot, &old_prot);

			return TRUE;
		}
		return FALSE;
	}

	BOOL __fastcall PatchJump(DWORD addr, DWORD dest)
	{
		INT relative = dest - addr - baseOffset - 2;
		return PatchRedirect(addr, (VOID*)dest, relative >= -128 && relative <= 127 ? 0xEB : 0xE9, 0);
	}

	BOOL __fastcall PatchHook(DWORD addr, VOID* hook, DWORD nop = 0)
	{
		return PatchRedirect(addr, hook, 0xE9, nop);
	}

	BOOL __fastcall PatchCall(DWORD addr, VOID* hook, DWORD nop = 0)
	{
		return PatchRedirect(addr, hook, 0xE8, nop);
	}

	BOOL __fastcall PatchBlock(DWORD addr, VOID* block, DWORD size)
	{
		DWORD address = addr + baseOffset;

		DWORD old_prot;
		if (VirtualProtect((VOID*)address, size, PAGE_EXECUTE_READWRITE, &old_prot))
		{
			switch (size)
			{
			case 4:
				*(DWORD*)address = *(DWORD*)block;
				break;
			case 2:
				*(WORD*)address = *(WORD*)block;
				break;
			case 1:
				*(BYTE*)address = *(BYTE*)block;
				break;
			default:
				MemoryCopy((VOID*)address, block, size);
				break;
			}

			VirtualProtect((VOID*)address, size, old_prot, &old_prot);

			return TRUE;
		}
		return FALSE;
	}

	BOOL __fastcall ReadBlock(DWORD addr, VOID* block, DWORD size)
	{
		DWORD address = addr + baseOffset;

		DWORD old_prot;
		if (VirtualProtect((VOID*)address, size, PAGE_READONLY, &old_prot))
		{
			switch (size)
			{
			case 4:
				*(DWORD*)block = *(DWORD*)address;
				break;
			case 2:
				*(WORD*)block = *(WORD*)address;
				break;
			case 1:
				*(BYTE*)block = *(BYTE*)address;
				break;
			default:
				MemoryCopy(block, (VOID*)address, size);
				break;
			}

			VirtualProtect((VOID*)address, size, old_prot, &old_prot);

			return TRUE;
		}
		return FALSE;
	}

	BOOL __fastcall PatchWord(DWORD addr, WORD value)
	{
		return PatchBlock(addr, &value, sizeof(value));
	}

	BOOL __fastcall PatchDWord(DWORD addr, DWORD value)
	{
		return PatchBlock(addr, &value, sizeof(value));
	}

	BOOL __fastcall PatchByte(DWORD addr, BYTE value)
	{
		return PatchBlock(addr, &value, sizeof(value));
	}

	BOOL __fastcall ReadWord(DWORD addr, WORD* value)
	{
		return ReadBlock(addr, value, sizeof(*value));
	}

	BOOL __fastcall ReadDWord(DWORD addr, DWORD* value)
	{
		return ReadBlock(addr, value, sizeof(*value));
	}

	BOOL __fastcall ReadByte(DWORD addr, BYTE* value)
	{
		return ReadBlock(addr, value, sizeof(*value));
	}

	BOOL __fastcall ReadRedirect(DWORD addr, DWORD* value)
	{
		if (ReadDWord(addr + 1, value))
		{
			*value += addr + baseOffset + 5;
			return TRUE;
		}
		else
			return FALSE;
	}

	BOOL __fastcall RedirectCall(DWORD addr, VOID* hook, DWORD* old)
	{
		if (ReadDWord(addr + 1, old))
		{
			*old += addr + 5 + baseOffset;
			return PatchCall(addr, hook);
		}

		return FALSE;
	}

	DWORD __fastcall PatchFunction(MappedFile* file, const CHAR* function, VOID* addr)
	{
		DWORD res = NULL;

		DWORD base = (DWORD)file->hModule;
		PIMAGE_NT_HEADERS headNT = (PIMAGE_NT_HEADERS)((BYTE*)base + ((PIMAGE_DOS_HEADER)file->hModule)->e_lfanew);

		PIMAGE_DATA_DIRECTORY dataDir = &headNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
		if (dataDir->Size)
		{
			PIMAGE_IMPORT_DESCRIPTOR imports = (PIMAGE_IMPORT_DESCRIPTOR)(base + dataDir->VirtualAddress);
			for (DWORD idx = 0; imports->Name; ++idx, ++imports)
			{
				CHAR* libraryName = (CHAR*)(base + imports->Name);

				PIMAGE_THUNK_DATA addressThunk = (PIMAGE_THUNK_DATA)(base + imports->FirstThunk);
				PIMAGE_THUNK_DATA nameThunk;
				if (imports->OriginalFirstThunk)
					nameThunk = (PIMAGE_THUNK_DATA)(base + imports->OriginalFirstThunk);
				else
				{
					if (!file->address)
					{
						file->Load();
						if (!file->address)
							return res;
					}

					headNT = (PIMAGE_NT_HEADERS)((BYTE*)file->address + ((PIMAGE_DOS_HEADER)file->address)->e_lfanew);
					PIMAGE_SECTION_HEADER sh = (PIMAGE_SECTION_HEADER)((DWORD)&headNT->OptionalHeader + headNT->FileHeader.SizeOfOptionalHeader);

					nameThunk = NULL;
					DWORD sCount = headNT->FileHeader.NumberOfSections;
					while (sCount)
					{
						--sCount;

						if (imports->FirstThunk >= sh->VirtualAddress && imports->FirstThunk < sh->VirtualAddress + sh->Misc.VirtualSize)
						{
							nameThunk = PIMAGE_THUNK_DATA((DWORD)file->address + sh->PointerToRawData + imports->FirstThunk - sh->VirtualAddress);
							break;
						}

						++sh;
					}

					if (!nameThunk)
						return res;
				}

				for (; nameThunk->u1.AddressOfData; ++nameThunk, ++addressThunk)
				{
					PIMAGE_IMPORT_BY_NAME name = PIMAGE_IMPORT_BY_NAME(base + nameThunk->u1.AddressOfData);

					WORD hint;
					if (ReadWord((INT)name - baseOffset, &hint) && !StrCompare((CHAR*)name->Name, function))
					{
						INT address = (INT)&addressThunk->u1.AddressOfData - baseOffset;
						if (ReadDWord(address, &res))
							PatchDWord(address, (DWORD)addr);

						return res;
					}
				}
			}
		}

		return res;
	}

	DWORD __fastcall PatchEntryPoint(const CHAR* library, VOID* entryPoint)
	{
		DWORD base = (DWORD)GetModuleHandle(library);
		if (!base)
			return FALSE;

		PIMAGE_NT_HEADERS headNT = (PIMAGE_NT_HEADERS)((BYTE*)base + ((PIMAGE_DOS_HEADER)base)->e_lfanew);
		return PatchHook(base + headNT->OptionalHeader.AddressOfEntryPoint, entryPoint);
	}
#pragma optimize("", on)
#pragma endregion

	// ===============================================================
	HWND hWndMain;

	UINT uAiMsg;
	DWORD aiTime;

	WinMessage* messages;
	WinMessage* __fastcall FindWinMessage(LPCSTR msg)
	{
		WinMessage* current = messages;
		while (current)
		{
			if (!StrCompare(current->name, msg))
				return current;

			current = current->prev;
		}

		return NULL;
	}

	WinMessage* __fastcall FindWinMessage(UINT id)
	{
		WinMessage* current = messages;
		while (current)
		{
			if (current->id == id)
				return current;

			current = current->prev;
		}

		return NULL;
	}

	WNDPROC OldWindowProc;
	LRESULT __stdcall WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT res;
		if (uMsg == WM_TIMER)
		{
			if (config.ai.thinking)
			{
				MSG msg;
				if (!config.ai.fast || !PeekMessage(&msg, hWnd, NULL, NULL, PM_NOREMOVE))
				{
					res = CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);

					if (timeGetTime() - aiTime >= 2000)
						config.ai.thinking = FALSE;
					else if (config.ai.fast)
					{
						Sleep(0);
						PostMessage(hWnd, uMsg, wParam, lParam);
					}
				}
				else
					res = NULL;
			}
			else
				res = CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}
		else
		{
			res = CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);

			if (uMsg == uAiMsg)
			{
				config.ai.thinking = TRUE;
				aiTime = timeGetTime();
			}
		}

		return res;
	}

	UINT __stdcall RegisterWindowMessageHook(LPCSTR lpString)
	{
		WinMessage* msg = FindWinMessage(lpString);
		if (!msg)
		{
			msg = (WinMessage*)MemoryAlloc(sizeof(WinMessage));
			msg->id = 0;
			msg->name = StrDuplicate(lpString);
			msg->prev = messages;
			messages = msg;
		}

		if (!msg->id)
			msg->id = RegisterWindowMessage(lpString);

		if (!StrCompare(lpString, "AIMESSAGE"))
			uAiMsg = msg->id;

		return msg->id;
	}

	HWND __stdcall CreateWindowExHook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, INT X, INT Y, INT nWidth, INT nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
	{
		BOOL isMain = !StrCompare(lpClassName, "MQ_UIManager");
		if (isMain)
		{
			dwStyle = WS_WINDOWED;
			RECT rect = { 0, 0, (LONG)config.mode->width, (LONG)config.mode->height };
			AdjustWindowRect(&rect, dwStyle, TRUE);

			nWidth = rect.right - rect.left;
			nHeight = rect.bottom - rect.top;

			X = (GetSystemMetrics(SM_CXSCREEN) - nWidth) >> 1;
			if (X < 0)
				X = 0;

			Y = (GetSystemMetrics(SM_CYSCREEN) - nHeight) >> 1;
			if (Y < 0)
				Y = 0;
		}

		HWND hWnd = CreateWindowEx(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		if (isMain)
		{
			hWndMain = hWnd;

			if (config.windowedMode)
				SetMenu(hWnd, config.menu);
		}
		else if (!StrCompare(lpClassName, "ThreadWindowClass"))
			OldWindowProc = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (LPARAM)WindowProc);

		return hWnd;
	}

	ATOM __stdcall RegisterClassHook(WNDCLASSA* lpWndClass)
	{
		if (!StrCompare(lpWndClass->lpszClassName, "MQ_UIManager"))
		{
			config.cursor = lpWndClass->hCursor;
			config.icon = lpWndClass->hIcon;
			lpWndClass->hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		}

		return RegisterClass(lpWndClass);
	}

	LONG __stdcall SetWindowLongHook(HWND hWnd, INT nIndex, LONG dwNewLong)
	{
		if (hWnd == hWndMain)
		{
			if (nIndex == GWL_WNDPROC)
			{
				LONG res = SetWindowLong(hWnd, nIndex, dwNewLong);
				Window::SetCaptureWindow(hWnd);
				return res;
			}

			if (nIndex == GWL_STYLE)
				dwNewLong = WS_FULLSCREEN;
		}

		return SetWindowLong(hWnd, nIndex, dwNewLong);
	}

	HWND __stdcall GetForegroundWindowHook()
	{
		HWND hWnd = GetForegroundWindow();
		OpenDraw* ddraw = Main::FindOpenDrawByWindow(hWnd);
		return ddraw ? ddraw->hWnd : hWnd;
	}

	INT __stdcall MessageBoxHook(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
	{
		RECT rc;
		GetClipCursor(&rc);

		INT res;
		ClipCursor(NULL);
		{
			ULONG_PTR cookie = NULL;
			if (hActCtx && hActCtx != INVALID_HANDLE_VALUE && !ActivateActCtxC(hActCtx, &cookie))
				cookie = NULL;

			res = MessageBox(hWnd, lpText, lpCaption, uType);

			if (cookie)
				DeactivateActCtxC(0, cookie);
		}
		ClipCursor(&rc);

		return res;
	}

	INT __stdcall ShowCursorHook(BOOL bShow)
	{
		return bShow ? 1 : -1;
	}

	BOOL __stdcall ClipCursorHook(RECT*)
	{
		return TRUE;
	}

	BOOL __stdcall GetClientRectHook(HWND hWnd, LPRECT lpRect)
	{
		if (hWnd == hWndMain)
		{
			lpRect->left = 0;
			lpRect->top = 0;
			lpRect->right = config.mode->width;
			lpRect->bottom = config.mode->height;
			return TRUE;
		}
		else
			return GetClientRect(hWnd, lpRect);
	}

	BOOL __stdcall GetWindowRectHook(HWND hWnd, LPRECT lpRect)
	{
		if (GetWindowRect(hWnd, lpRect))
		{
			if (hWnd == hWndMain)
			{
				RECT rect = { 0, 0, (LONG)config.mode->width, (LONG)config.mode->height };
				AdjustWindowRect(&rect, config.windowedMode ? WS_WINDOWED : WS_FULLSCREEN,
					config.windowedMode);

				lpRect->right = lpRect->left + rect.right - rect.left;
				lpRect->bottom = lpRect->top + rect.bottom - rect.top;
			}

			return TRUE;
		}

		return FALSE;
	}

	BOOL __stdcall GetCursorPosHookV1(LPPOINT lpPoint)
	{
		OpenDraw* ddraw = Main::FindOpenDrawByWindow(GetForegroundWindow());
		if (ddraw)
		{
			if (GetCursorPos(lpPoint))
			{
				RECT rect;
				if (GetClientRect(hWndMain, &rect) && ClientToScreen(hWndMain, (LPPOINT)&rect))
				{
					if (!config.windowedMode && config.borderless.mode)
						rect.bottom -= BORDERLESS_OFFSET;

					FLOAT fx = (FLOAT)config.mode->width / rect.right;
					FLOAT fy = (FLOAT)config.mode->height / rect.bottom;

					POINTFLOAT offset = { 0.0f, 0.0f };
					if (config.image.aspect && fx != fy)
					{
						if (fx < fy)
						{
							fx = fy;
							offset.x = ((FLOAT)rect.right - (FLOAT)config.mode->width / fx) * 0.5f;
						}
						else
						{
							fy = fx;
							offset.y = ((FLOAT)rect.bottom - (FLOAT)config.mode->height / fy) * 0.5f;
						}
					}

					lpPoint->x = LONG(fx * (FLOAT(lpPoint->x - rect.left) - offset.x));
					lpPoint->y = LONG(fy * (FLOAT(lpPoint->y - rect.top) - offset.y));

					if (lpPoint->x < 0)
						lpPoint->x = 0;

					if (lpPoint->y < 0)
						lpPoint->y = 0;

					if (lpPoint->x >= *(INT*)&config.mode->width)
						lpPoint->x = *(INT*)&config.mode->width - 1;

					if (lpPoint->y >= *(INT*)&config.mode->height)
						lpPoint->y = *(INT*)&config.mode->height - 1;
				}

				return TRUE;
			}

			return FALSE;
		}
		else
		{
			lpPoint->x = config.mode->width >> 1;
			lpPoint->y = config.mode->height >> 1;
			return TRUE;
		}
	}

	BOOL __stdcall GetCursorPosHookV2(LPPOINT lpPoint)
	{
		if (GetCursorPos(lpPoint))
		{
			RECT rect;
			if (GetClientRect(hWndMain, &rect) && ClientToScreen(hWndMain, (LPPOINT)&rect))
			{
				if (!config.windowedMode && config.borderless.mode)
					rect.bottom -= BORDERLESS_OFFSET;

				FLOAT fx = (FLOAT)config.mode->width / rect.right;
				FLOAT fy = (FLOAT)config.mode->height / rect.bottom;

				POINTFLOAT offset = { 0.0f, 0.0f };
				if (config.image.aspect && fx != fy)
				{
					if (fx < fy)
					{
						fx = fy;
						offset.x = ((FLOAT)rect.right - (FLOAT)config.mode->width / fx) * 0.5f;
					}
					else
					{
						fy = fx;
						offset.y = ((FLOAT)rect.bottom - (FLOAT)config.mode->height / fy) * 0.5f;
					}
				}

				if (!Config::IsZoomed())
				{
					lpPoint->x = LONG(fx * (FLOAT(lpPoint->x - rect.left) - offset.x)) + rect.left;
					lpPoint->y = LONG(fy * (FLOAT(lpPoint->y - rect.top) - offset.y)) + rect.top;
				}
				else
				{
					FLOAT kx = config.zoom.sizeFloat.width / config.mode->width;
					FLOAT ky = config.zoom.sizeFloat.height / config.mode->height;

					lpPoint->x = LONG(fx * kx * (FLOAT(lpPoint->x - rect.left) - offset.x) + ((FLOAT)config.mode->width - config.zoom.sizeFloat.width) * 0.5f) + rect.left;
					lpPoint->y = LONG(fy * kx * (FLOAT(lpPoint->y - rect.top) - offset.y) + ((FLOAT)config.mode->height - config.zoom.sizeFloat.height) * 0.5f) + rect.top;
				}
			}

			return TRUE;
		}

		return FALSE;
	}

	DWORD cursorPos;
	LPPOINT __stdcall GetCursorPosHookV2a(LPPOINT lpPoint)
	{
		OpenDraw* ddraw = Main::FindOpenDrawByWindow(GetForegroundWindow());
		if (ddraw)
		{
			GetCursorPos(lpPoint);

			if (lpPoint->x < 3)
				lpPoint->x = 0;
			else if (lpPoint->x >= GetSystemMetrics(SM_CXSCREEN) - 3)
				lpPoint->x = config.mode->width;
			else
				lpPoint->x = config.mode->width >> 1;

			if (lpPoint->y < 3)
				lpPoint->y = 0;
			else if (lpPoint->y >= GetSystemMetrics(SM_CYSCREEN) - 3)
				lpPoint->y = config.mode->height;
			else
				lpPoint->y = config.mode->height >> 1;
		}
		else
		{
			lpPoint->x = config.mode->width >> 1;
			lpPoint->y = config.mode->height >> 1;
		}

		return lpPoint;
	}

	VOID __declspec(naked) hook_cursorPos()
	{
		__asm {
			MOV EAX, config.windowedMode
			TEST EAX, EAX
			JZ lbl_win
			JMP GetCursorPosHookV2a
			lbl_win : JMP cursorPos
		}
	}

	BOOL __stdcall SetCursorPosHook(INT X, INT Y)
	{
		RECT rect;
		if (GetClientRect(hWndMain, &rect) && ClientToScreen(hWndMain, (LPPOINT)&rect))
		{
			if (!config.windowedMode && config.borderless.mode)
				rect.bottom -= BORDERLESS_OFFSET;

			FLOAT fx = (FLOAT)rect.right / config.mode->width;
			FLOAT fy = (FLOAT)rect.bottom / config.mode->height;

			POINTFLOAT offset = { 0.0f, 0.0f };
			if (config.image.aspect && fx != fy)
			{
				if (fx > fy)
				{
					fx = fy;
					offset.x = ((FLOAT)rect.right - (fx * config.mode->width)) * 0.5f;
				}
				else
				{
					fy = fx;
					offset.y = ((FLOAT)rect.bottom - (fy * config.mode->height)) * 0.5f;
				}
			}

			if (!Config::IsZoomed())
			{
				X = LONG(fx * FLOAT(X - rect.left) + offset.x) + rect.left;
				Y = LONG(fy * FLOAT(Y - rect.top) + offset.y) + rect.top;
			}
			else
			{
				FLOAT kx = (FLOAT)config.mode->width / config.zoom.sizeFloat.width;
				FLOAT ky = (FLOAT)config.mode->height / config.zoom.sizeFloat.height;

				X = LONG(fx * kx * (FLOAT(X - rect.left) - ((FLOAT)config.mode->width - config.zoom.sizeFloat.width) * 0.5f) + offset.x) + rect.left;
				Y = LONG(fy * ky * (FLOAT(Y - rect.top) - ((FLOAT)config.mode->height - config.zoom.sizeFloat.height) * 0.5f) + offset.y) + rect.top;
			}
		}

		return SetCursorPos(X, Y);
	}

	BOOL __stdcall ClientToScreenHook(HWND hWnd, LPPOINT lpPoint)
	{
		if (hWnd == hWndMain)
			return TRUE;

		return ClientToScreen(hWnd, lpPoint);
	}

	INT __stdcall GetDeviceCapsHook(HDC hdc, INT index)
	{
		if (index == BITSPIXEL)
			return 16;

		return GetDeviceCaps(hdc, index);
	}

	const CLSID CLSID_DirectDraw = { 0xD7B70EE0, 0x4340, 0x11CF, 0xB0, 0x63, 0x00, 0x20, 0xAF, 0xC2, 0xCD, 0x35 };
	const IID IID_IDirectDraw4 = { 0x9C59509A, 0x39BD, 0x11D1, 0x8C, 0x4A, 0x00, 0xC0, 0x4F, 0xD9, 0x30, 0xC5 };

	const CLSID CLSID_DirectPlay = { 0xD1EB6D20, 0x8923, 0x11D0, 0x9D, 0x97, 0x0, 0xA0, 0xC9, 0xA, 0x43, 0xCB };
	const IID IID_IDirectPlay4A = { 0xAB1C531, 0x4745, 0x11D1, 0xA7, 0xA1, 0x0, 0x0, 0xF8, 0x3, 0xAB, 0xFC };

	HRESULT __stdcall CoCreateInstanceHook(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID* ppv)
	{
		if (!MemoryCompare(&rclsid, &CLSID_DirectDraw, sizeof(CLSID)) && !MemoryCompare(&riid, &IID_IDirectDraw4, sizeof(IID)))
			return Main::DrawCreateEx(NULL, ppv, riid, NULL);

		HRESULT res = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
		if (!MemoryCompare(&rclsid, &CLSID_DirectPlay, sizeof(CLSID)) && !MemoryCompare(&riid, &IID_IDirectPlay4A, sizeof(IID)))
			*ppv = new IPlay4((LPDIRECTPLAY4)*ppv, hWndMain);

		return res;
	}

#pragma region 32 BPP
	BOOL isBink;
	DWORD pBinkCopyToBuffer;
	INT __stdcall BinkCopyToBufferHook(VOID* hBnk, VOID* dest, INT pitch, DWORD height, DWORD x, DWORD y, DWORD flags)
	{
		isBink = TRUE;
		return ((INT(__stdcall*)(VOID*, VOID*, INT, DWORD, DWORD, DWORD, DWORD))pBinkCopyToBuffer)(hBnk, dest, pitch << 1, height, x, y, BINKCOPYALL | (config.renderer == RendererGDI ? BINKSURFACE32 : BINKSURFACE32R));
	}

	BlendData blendList[32];

#pragma region Memory
	VOID __declspec(naked) hook_005A6311()
	{
		__asm {
			SHL EAX, 0x1
			IMUL EAX, [ESI + 0x4]
			POP EDX
			PUSH EBX
			PUSH EDX
			RETN
		}
	}

	DWORD back_005BACE7;
	VOID __declspec(naked) hook_005BACE1()
	{
		__asm {
			SHL EAX, 0x2
			IMUL EAX, [EBP - 0x14]
			JMP back_005BACE7
		}
	}

	VOID __declspec(naked) hook_00674400()
	{
		__asm {
			SHL EAX, 0x1
			IMUL EAX, [EDI + 0x4]
			POP EDX
			PUSH EAX
			PUSH EDX
			RETN
		}
	}

	VOID __declspec(naked) hook_005A6209()
	{
		__asm
		{
			POP EDI
			PUSH 0x0
			PUSH EDI
			XOR EDI, EDI
			SHL EAX, 0x2
			RETN
		}
	}
#pragma endregion

#pragma region PIXEL
	struct {
		BOOL active;
		BOOL ready;
		BOOL mirror;
		SIZE size;
	} battleBgIndices;

	DWORD __fastcall ConvertToRGB(DWORD color)
	{
		return ((color & 0x001F) << 19) | ((color & 0x07E0) << 5) | ((color & 0xF800) >> 8) | ALPHA_COMPONENT;
	}

	DWORD __fastcall ConvertToBGR(DWORD color)
	{
		return ((color & 0x001F) << 3) | ((color & 0x07E0) << 5) | ((color & 0xF800) << 8) | ALPHA_COMPONENT;
	}

	DWORD(__fastcall* Convert565toRGB)
	(DWORD color);

	VOID __stdcall Pixel_Blit_Indexed_to_565(BYTE* srcData, DWORD srcPitch, DWORD* palette, DWORD* dstData, DWORD dstPitch, RECT* rect)
	{
		BYTE* lpSrc = srcData + rect->top * srcPitch;
		INT srcInc;

		BOOL found = FALSE;
		if (battleBgIndices.ready && battleBgIndices.size.cx == rect->right - rect->left && battleBgIndices.size.cy == rect->bottom - rect->top)
		{
			found = TRUE;
			battleBgIndices.ready = FALSE;
			battleBgIndices.active = FALSE;
		}

		if (found && battleBgIndices.mirror)
		{
			lpSrc += rect->right - 1;
			srcInc = -1;
		}
		else
		{
			lpSrc += rect->left;
			srcInc = 1;
		}

		dstPitch >>= 1;
		DWORD* lpDst = dstData + rect->top * dstPitch + rect->left;

		DWORD width = rect->right - rect->left;
		DWORD height = rect->bottom - rect->top;

		if (config.renderer == RendererGDI)
		{
			do
			{
				BYTE* src = lpSrc;
				DWORD* dst = lpDst;

				DWORD count = width;
				do
				{
					*dst++ = _byteswap_ulong(_rotl(palette[*src], 8)) | ALPHA_COMPONENT;
					src += srcInc;
				} while (--count);

				lpSrc += srcPitch;
				lpDst += dstPitch;
			} while (--height);
		}
		else
		{
			do
			{
				BYTE* src = lpSrc;
				DWORD* dst = lpDst;

				DWORD count = width;
				do
				{
					*dst++ = palette[*src] | ALPHA_COMPONENT;
					src += srcInc;
				} while (--count);

				lpSrc += srcPitch;
				lpDst += dstPitch;
			} while (--height);
		}
	}

	DWORD __stdcall Pixel_ConvertPixel_565(BYTE red, BYTE green, BYTE blue)
	{
		return (blue >> 3) | (green >> 3 << 6) | (red >> 3 << 11);
	}

	VOID __stdcall Pixel_Blit_By_Masks(DWORD* srcData, LONG srcPitch, DWORD redMask, DWORD greenMask, DWORD blueMask, DWORD alphaMask, BYTE* dstData, LONG dstPitch, RECT* rect)
	{
		srcPitch >>= 1;

		if (dstPitch & 3)
			dstPitch = (dstPitch & 0xFFFFFFFC) + 4;

		srcData += rect->top * srcPitch + rect->left;

		if (!Config::IsZoomed())
			dstData += rect->top * dstPitch + rect->left * 3;

		LONG height = rect->bottom - rect->top;
		do
		{
			BYTE* src = (BYTE*)srcData;
			BYTE* dst = dstData;

			LONG width = rect->right - rect->left;
			do
			{
				*dst++ = *++src;
				*dst++ = *++src;
				*dst++ = *++src;

				++src;
			} while (--width);

			srcData += srcPitch;
			dstData += dstPitch;
		} while (--height);
	}

	VOID __stdcall Pixel_ConvertPixel_565_to_RGB(DWORD color, BYTE* red, BYTE* green, BYTE* blue)
	{
		BYTE* px = (BYTE*)&color;

		*red = *px++;
		*green = *px++;
		*blue = *px;
	}

	VOID __stdcall Pixel_Blit_RGB_to_565(BYTE* srcData, DWORD srcPitch, DWORD* dstData, DWORD dstPitch, RECT* rect)
	{
		dstPitch >>= 1;

		srcData += rect->left + rect->top * srcPitch;
		dstData += rect->left + rect->top * dstPitch;

		DWORD height = rect->bottom - rect->top;
		do
		{
			BYTE* src = srcData;
			BYTE* dst = (BYTE*)dstData;

			DWORD width = rect->right - rect->left;
			do
			{
				*dst++ = *src++;
				*dst++ = *src++;
				*dst++ = *src++;
				*dst++ = 0xFF;
			} while (--width);

			srcData += srcPitch;
			dstData += dstPitch;
		} while (--height);
	}

	VOID __stdcall Pixel_RGB_Swap(BYTE* data, LONG pitch, SIZE* size)
	{
		if (config.renderer == RendererGDI)
		{
			DWORD height = size->cy;
			while (height)
			{
				--height;

				BYTE* px = data;

				DWORD width = size->cx;
				while (width)
				{
					--width;
					BYTE temp = *(px + 2);
					*(px + 2) = *px;
					*px = temp;

					px += 3;
				}

				data += pitch;
			}
		}
	}

	DWORD __stdcall Pixel_Blend(DWORD dstData, DWORD srcData, DWORD msk)
	{
		DWORD res;

		msk &= 0x000000FF;

		if (msk == 255)
			res = srcData;
		else if (msk)
		{
			++msk;

			DWORD src = srcData & 0x000000FF;
			DWORD dst = dstData & 0x000000FF;
			res = (dst + ((src - dst) * msk) / 256) & 0x000000FF;

			src = srcData & 0x0000FF00;
			dst = dstData & 0x0000FF00;
			res |= (dst + ((src - dst) * msk) / 256) & 0x0000FF00;

			src = srcData & 0x00FF0000;
			dst = dstData & 0x00FF0000;
			res |= (dst + ((src - dst) * msk) / 256) & 0x00FF0000;
		}
		else
			res = dstData;

		return res | ALPHA_COMPONENT;
	}

	DWORD __stdcall Pixel_BlendSome(DWORD pix, BYTE b, BYTE g, BYTE r, BYTE msk)
	{
		DWORD color = r | (g << 8) | (b << 16);
		return Pixel_Blend(pix, color, msk);
	}

	VOID __stdcall Pixel_BlitBlend(DWORD* srcData, DWORD* dstData, DWORD count, BYTE* mskData)
	{
		while (count)
		{
			--count;
			*dstData = Pixel_Blend(*dstData, *srcData, *(BYTE*)mskData);

			++srcData;
			++dstData;
			++mskData;
		}
	}

	VOID __stdcall Pixel_BlitBlendWithColorKey(BlendData* blendItem, DWORD count, DWORD colorKey)
	{
		colorKey = Convert565toRGB(colorKey);
		while (count)
		{
			--count;
			DWORD* srcData = blendItem->srcData;
			DWORD* dstData = blendItem->dstData;
			BYTE* mskData = (BYTE*)blendItem->mskData;

			DWORD length = blendItem->length;
			while (length)
			{
				--length;
				if ((*dstData & COLORKEY_AND) != colorKey)
					*dstData = Pixel_Blend(*dstData, *srcData, *(BYTE*)mskData);

				++srcData;
				++dstData;
				++mskData;
			}

			++blendItem;
		}
	}

	VOID __stdcall Pixel_BlitBlendAvarage(DWORD* src, LONG srcPitch, POINT* srcPos, DWORD* dst, LONG dstPitch, POINT* dstPos, SIZE* size, BYTE flag, DWORD colorKey)
	{
		if (!flag || flag == 0xFF || !dst)
			return;

		colorKey = Convert565toRGB(colorKey);
		srcPitch >>= 1;
		dstPitch >>= 1;

		src += srcPitch * srcPos->y + srcPos->x;
		dst += dstPitch * dstPos->y + dstPos->x;

		srcPitch -= size->cx;
		dstPitch -= size->cx;

		DWORD height = size->cy;
		while (height)
		{
			--height;
			DWORD width = size->cx;
			while (width)
			{
				--width;
				if ((*src & COLORKEY_AND) != colorKey)
				{
					DWORD s = *src & 0x000000FF;
					DWORD d = *dst & 0x000000FF;
					DWORD res = (d + (s - d) / 2) & 0x000000FF;

					s = *src & 0x0000FF00;
					d = *dst & 0x0000FF00;
					res |= (d + (s - d) / 2) & 0x0000FF00;

					s = *src & 0x00FF0000;
					d = *dst & 0x00FF0000;
					res |= (d + (s - d) / 2) & 0x00FF0000;

					*dst = res | ALPHA_COMPONENT;
				}

				++src;
				++dst;
			}

			src += srcPitch;
			dst += dstPitch;
		}
	}

	VOID __stdcall Pixel_Add(DWORD* src, LONG srcPitch, POINT* srcPos, DWORD* dst, LONG dstPitch, POINT* dstPos, SIZE* size)
	{
		if (!dst)
			return;

		srcPitch >>= 1;
		dstPitch >>= 1;

		src += srcPitch * srcPos->y + srcPos->x;
		dst += dstPitch * dstPos->y + dstPos->x;

		srcPitch -= size->cx;
		dstPitch -= size->cx;

		DWORD height = size->cy;
		while (height)
		{
			--height;
			DWORD width = size->cx;
			while (width)
			{
				--width;
				DWORD cs = *src++;
				DWORD cd = *dst;

				DWORD r = (cd & 0x000000FF) + (cs & 0x000000FF);
				if (r > 0x000000FF)
					r = 0x000000FF;

				DWORD g = (cd & 0x0000FF00) + (cs & 0x0000FF00);
				if (g > 0x0000FF00)
					g = 0x0000FF00;

				DWORD b = (cd & 0x00FF0000) + (cs & 0x00FF0000);
				if (b > 0x00FF0000)
					b = 0x00FF0000;

				*dst++ = r | g | b | ALPHA_COMPONENT;
			}

			src += srcPitch;
			dst += dstPitch;
		}
	}

	VOID __stdcall Pixel_Sub(DWORD* src, LONG srcPitch, POINT* srcPos, DWORD* dst, LONG dstPitch, POINT* dstPos, SIZE* size)
	{
		if (!dst)
			return;

		srcPitch >>= 1;
		dstPitch >>= 1;

		src += srcPitch * srcPos->y + srcPos->x;
		dst += dstPitch * dstPos->y + dstPos->x;

		srcPitch -= size->cx;
		dstPitch -= size->cx;

		DWORD height = size->cy;
		while (height)
		{
			--height;
			DWORD width = size->cx;
			while (width)
			{
				--width;
				DWORD cs = *src++;
				DWORD cd = *dst;

				INT r = (cd & 0x000000FF) - (cs & 0x000000FF);
				if (r < 0)
					r = 0;

				INT g = (cd & 0x0000FF00) - (cs & 0x0000FF00);
				if (g < 0)
					g = 0;

				INT b = (cd & 0x00FF0000) - (cs & 0x00FF0000);
				if (b < 0)
					b = 0;

				*dst++ = r | g | b | ALPHA_COMPONENT;
			}

			src += srcPitch;
			dst += dstPitch;
		}
	}

	VOID __stdcall Pixel_BlitColorKey(DWORD* srcData, LONG srcPitch, POINT* srcPos, DWORD* dstData, LONG dstPitch, POINT* dstPos, SIZE* size, BYTE flag, DWORD colorKey)
	{
		if (!flag || flag == 0xFF || !dstData)
			return;

		colorKey = Convert565toRGB(colorKey);
		srcPitch >>= 1;
		dstPitch >>= 1;

		srcData += srcPitch * srcPos->y + srcPos->x;
		dstData += dstPitch * dstPos->y + dstPos->x;

		BOOL check = (~dstPos->y ^ dstPos->x) & 1;

		DWORD height = size->cy;
		while (height)
		{
			--height;
			DWORD* src = srcData;
			DWORD* dst = dstData;

			DWORD width = size->cx;
			if (check)
			{
				++src;
				++dst;
				--width;
			}

			width = (width + 1) >> 1;
			while (width)
			{
				--width;
				if ((*src & COLORKEY_AND) != colorKey)
					*dst = *src;

				src += 2;
				dst += 2;
			}

			srcData += srcPitch;
			dstData += dstPitch;

			check = !check;
		}
	}

	VOID __stdcall Pixel_BlitEmptyColor(DWORD* srcData, LONG srcPitch, POINT* srcPos, DWORD* dstData, LONG dstPitch, POINT* dstPos, SIZE* size)
	{
		if (!dstData)
			return;

		srcPitch >>= 1;
		dstPitch >>= 1;

		srcData += srcPitch * srcPos->y + srcPos->x;
		dstData += dstPitch * dstPos->y + dstPos->x;

		BOOL check = (~dstPos->y ^ dstPos->x) & 1;

		DWORD height = size->cy;
		while (height)
		{
			--height;
			DWORD* src = srcData;
			DWORD* dst = dstData;

			DWORD width = size->cx;
			if (check)
			{
				++src;
				++dst;
				--width;
			}

			width = (width + 1) >> 1;
			while (width)
			{
				--width;
				if (*src & COLORKEY_AND)
					*dst = *src;

				src += 2;
				dst += 2;
			}

			srcData += srcPitch;
			dstData += dstPitch;

			check = !check;
		}
	}

	VOID __stdcall Pixel_DoubleLighter(DWORD* data, LONG pitch, SIZE* size, DWORD colorKey, BYTE flag)
	{
		colorKey = Convert565toRGB(colorKey);
		pitch >>= 1;
		pitch -= size->cx;

		DWORD height = size->cy;
		while (height)
		{
			--height;
			DWORD width = size->cx;
			while (width)
			{
				--width;
				if ((*data & COLORKEY_AND) == colorKey)
					*data = 0;
				else if (flag)
				{
					DWORD r = *data & 0xFF;
					DWORD g = (*data >> 8) & 0xFF;
					DWORD b = (*data >> 16) & 0xFF;
					DWORD a = (*data >> 24) & 0xFF;

					if (r + g + b > 56)
					{
						r <<= 1;
						if (r > 0xFF)
							r = 0xFF;

						g <<= 1;
						if (g > 0xFF)
							g = 0xFF;

						b <<= 1;
						if (b > 0xFF)
							b = 0xFF;

						*data = r | (g << 8) | (b << 16) | (a << 24);
					}
					else
						*data = 0;
				}

				++data;
			}

			data += pitch;
		}
	}

	const VOID* const pixelFunctions[] = { Pixel_Blit_Indexed_to_565,
		Pixel_ConvertPixel_565,
		Pixel_Blit_By_Masks,
		Pixel_ConvertPixel_565_to_RGB,
		Pixel_Blit_RGB_to_565,
		Pixel_RGB_Swap,
		Pixel_BlendSome,
		Pixel_Blend,
		Pixel_BlitBlend,
		Pixel_BlitBlendWithColorKey,
		Pixel_BlitBlendAvarage,
		Pixel_Add,
		Pixel_Sub,
		Pixel_BlitColorKey,
		Pixel_BlitEmptyColor,
		Pixel_DoubleLighter };
#pragma endregion

	BOOL __fastcall Clip(LONG* shiftX, LONG* shiftY, LONG* left, LONG* top, LONG* width, LONG* height, RECT* clipper)
	{
		if (*left < clipper->left)
		{
			*width += *left - clipper->left;
			*shiftX += clipper->left - *left;
			*left = clipper->left;
		}

		if (*left + *width > clipper->right)
			*width = clipper->right - *left;

		if (*top < clipper->top)
		{
			*height += *top - clipper->top;
			*shiftY += clipper->top - *top;
			*top = clipper->top;
		}

		if (*top + *height > clipper->bottom)
			*height = clipper->bottom - *top;

		return *width > 0 && *height > 0;
	}

	VOID __stdcall DrawMinimapObjects(BlitObject* obj, VOID* data, RECT* rect, DWORD pitch, DWORD colorKey, POINT* loc)
	{
		if (data && rect->left <= rect->right && rect->top <= rect->bottom && (colorKey <= (obj->isTrueColor ? 0xFFFFu : 0xFFu) || colorKey == 0xFFFFFFFF))
		{
			LONG shiftX = rect->left;
			LONG shiftY = rect->top;

			LONG left = loc->x;
			LONG top = loc->y;

			LONG width = rect->right - rect->left;
			LONG height = rect->bottom - rect->top;

			if (Clip(&shiftX, &shiftY, &left, &top, &width, &height, &obj->rect))
			{
				if (colorKey == 0xFFFFFFFF)
				{
					if (obj->isTrueColor)
					{
						DWORD srcPitch = pitch >> 1;
						DWORD dstPitch = obj->pitch >> 1;

						DWORD* srcData = (DWORD*)data + shiftY * srcPitch + shiftX;
						DWORD* dstData = (DWORD*)obj->data + top * dstPitch + left;

						do
						{
							MemoryCopy(dstData, srcData, width << 2);
							dstData += dstPitch;
							srcData += srcPitch;
						} while (--height);
					}
					else
					{
						DWORD srcPitch = pitch;
						DWORD dstPitch = obj->pitch;

						BYTE* srcData = (BYTE*)data + shiftY * srcPitch + shiftX;
						BYTE* dstData = (BYTE*)obj->data + top * dstPitch + left;

						do
						{
							MemoryCopy(dstData, srcData, width);
							dstData += dstPitch;
							srcData += srcPitch;
						} while (--height);
					}
				}
				else
				{
					if (obj->isTrueColor)
					{
						colorKey = Convert565toRGB(colorKey);

						DWORD srcPitch = pitch >> 1;
						DWORD dstPitch = obj->pitch >> 1;

						DWORD* srcData = (DWORD*)data + shiftY * srcPitch + shiftX;
						DWORD* dstData = (DWORD*)obj->data + top * dstPitch + left;

						if (config.renderer == RendererGDI)
						{
							do
							{
								DWORD* src = srcData;
								DWORD* dst = dstData;

								DWORD count = width;
								do
								{
									if ((*src & COLORKEY_AND) != colorKey)
										*dst = *src;

									++src;
									++dst;
								} while (--count);

								dstData += dstPitch;
								srcData += srcPitch;
							} while (--height);
						}
						else
						{
							do
							{
								DWORD* src = srcData;
								DWORD* dst = dstData;

								DWORD count = width;
								do
								{
									if ((*src & COLORKEY_AND) != colorKey)
										*dst = _byteswap_ulong(_rotl(*src, 8)); // swap for map objects

									++src;
									++dst;
								} while (--count);

								dstData += dstPitch;
								srcData += srcPitch;
							} while (--height);
						}
					}
					else
					{
						DWORD srcPitch = pitch;
						DWORD dstPitch = obj->pitch;

						BYTE* srcData = (BYTE*)data + shiftY * srcPitch + shiftX;
						BYTE* dstData = (BYTE*)obj->data + top * dstPitch + left;

						do
						{
							BYTE* src = srcData;
							BYTE* dst = dstData;

							DWORD count = width;
							do
							{
								if (*src != colorKey)
									*dst = *src;

								++src;
								++dst;
							} while (--count);

							dstData += dstPitch;
							srcData += srcPitch;
						} while (--height);
					}
				}
			}
		}
	}

	VOID __declspec(naked) hook_0055D419()
	{
		__asm
		{
			POP EAX
			PUSH ECX
			PUSH EAX
			JMP DrawMinimapObjects
		}
	}

	VOID __stdcall FillColor(BlitObject* obj, DWORD color, RECT* rect)
	{
		if (obj->isTrueColor)
		{
			DWORD pitch = obj->pitch >> 1;
			color = Convert565toRGB(color);

			DWORD* data = (DWORD*)obj->data + rect->top * pitch + rect->left;
			DWORD height = rect->bottom - rect->top;
			while (height)
			{
				--height;
				DWORD* ptr = data;
				DWORD width = rect->right - rect->left;
				while (width)
				{
					--width;
					*ptr++ = color;
				}

				data += pitch;
			}
		}
		else
		{
			DWORD pitch = obj->pitch;

			BYTE* data = (BYTE*)obj->data + rect->top * pitch + rect->left;
			DWORD height = rect->bottom - rect->top;
			while (height)
			{
				--height;
				BYTE* ptr = data;
				DWORD width = rect->right - rect->left;
				while (width)
				{
					--width;
					*ptr++ = LOBYTE(color);
				}

				data += pitch;
			}
		}
	}

	VOID __declspec(naked) hook_0055D283()
	{
		__asm
		{
			POP EAX
			PUSH ECX
			PUSH EAX
			JMP FillColor
		}
	}

	VOID __stdcall DrawWaterBorders(DWORD* thisObj, BlitObject* obj, POINT* loc, RECT* rect)
	{
		DWORD pitch = obj->pitch >> 1;

		DWORD* srcData = (*(DWORD * (__thiscall**)(DWORD*))((*thisObj) + 12))(thisObj);
		DWORD* dstData = (DWORD*)obj->data + loc->y * pitch;

		LONG offset = 30;
		LONG width = 2;

		LONG idx = 0;
		do
		{
			LONG y = loc->y + idx;
			if (y >= rect->top && y < rect->bottom)
			{
				LONG srcX = offset;
				LONG size = width;

				LONG dstX = loc->x + srcX;

				if (dstX < rect->left)
				{
					size += dstX - rect->left;
					srcX += rect->left - dstX;
					dstX = 0;
				}

				if (size >= rect->right - dstX)
					size = rect->right - dstX;

				if (size > 0)
				{
					DWORD* src = srcData + srcX;
					DWORD* dst = dstData + dstX;
					do
					{
						if ((*src & COLORKEY_AND) != COLORKEY_CHECK)
							*dst = *src;

						++src;
						++dst;
					} while (--size);
				}
			}

			if (++idx == 32)
				return;

			if (idx < 16)
			{
				offset -= 2;
				width += 4;
			}
			else if (idx != 16)
			{
				offset += 2;
				width -= 4;
			}

			srcData += 64;
			dstData += pitch;
		} while (TRUE);
	}

	VOID __declspec(naked) hook_005B5560()
	{
		__asm
		{
			POP EAX
			PUSH ECX
			PUSH EAX
			JMP DrawWaterBorders
		}
	}

	VOID __stdcall DrawGround(DWORD* thisObj, BlitObject* obj, POINT* srcLoc, POINT* dstLoc, RECT* rect, BYTE* blendMask, DWORD* alphaMask)
	{
		DWORD pitch = obj->pitch >> 1;

		DWORD* srcData = **(DWORD***)thisObj[1] + srcLoc->y * 192;
		DWORD* dstData = (DWORD*)obj->data + dstLoc->y * pitch;

		BlendData* blendItem = blendList;
		DWORD blendCount = 0;

		LONG offset = 30;
		LONG width = 2;

		LONG idx = 0;
		do
		{
			LONG srcY = srcLoc->y + idx;
			LONG dstY = dstLoc->y + idx;

			if (srcY >= 0 && srcY < 192 && dstY >= rect->top && dstY < rect->bottom)
			{
				LONG srcX = offset + srcLoc->x;
				LONG dstX = offset + dstLoc->x;

				LONG off = offset;
				LONG size = width;

				if (srcX < 0)
				{
					size += srcX;
					off = offset - srcX;
					dstX -= srcX;
					srcX = 0;
				}

				if (dstX < rect->left)
				{
					size += dstX - rect->left;
					off += rect->left - dstX;
					srcX += rect->left - dstX;
					dstX = 0;
				}

				if (size >= 192 - srcX)
					size = 192 - srcX;

				if (size >= rect->right - dstX)
					size = rect->right - dstX;

				if (size > 0)
				{
					if (blendMask) // Water
					{
						blendItem->srcData = srcData + srcX;
						blendItem->dstData = dstData + dstX;
						blendItem->length = size;
						blendItem->mskData = blendMask + (idx * 64) + off;

						++blendCount;
						++blendItem;
					}
					else if (alphaMask)
					{
						DWORD* msk = alphaMask + (idx * 32) + off;
						DWORD* src = srcData + srcX;
						DWORD* dst = dstData + dstX;

						do
						{
							DWORD pix = *msk;
							if ((pix & COLORKEY_AND) != COLORKEY_CHECK)
							{
								if ((pix & COLORKEY_AND) == 0x00F8FC80)
									pix = *src;

								*dst = pix;
							}

							++msk;
							++src;
							++dst;
						} while (--size);
					}
					else // ground
						MemoryCopy(dstData + dstX, srcData + srcX, size * sizeof(DWORD));
				}
			}

			if (++idx == 32)
				break;

			if (idx < 16)
			{
				offset -= 2;
				width += 4;
			}
			else if (idx != 16)
			{
				offset += 2;
				width -= 4;
			}

			srcData += 192;
			dstData += pitch;
		} while (TRUE);

		if (blendCount)
			Pixel_BlitBlendWithColorKey(blendList, blendCount, 0xF81F);
	}

	VOID __declspec(naked) hook_005B5660()
	{
		__asm
		{
			POP EAX
			PUSH ECX
			PUSH EAX
			JMP DrawGround
		}
	}

	VOID __stdcall ClearGround(BlitObject* obj, POINT* loc, RECT* rect)
	{
		DWORD pitch = obj->pitch >> 1;

		DWORD* dstData = (DWORD*)obj->data + loc->y * pitch;

		LONG offset = 30;
		LONG width = 2;

		LONG idx = 0;
		do
		{
			LONG y = loc->y + idx;
			if (y >= rect->top && y < rect->bottom)
			{
				LONG dstX = loc->x + offset;
				LONG size = width;

				if (dstX < rect->left)
				{
					size += dstX - rect->left;
					dstX = 0;
				}

				if (size >= rect->right - dstX)
					size = rect->right - dstX;

				if (size > 0)
				{
					DWORD* dst = dstData + dstX;
					do
						*dst++ = ALPHA_COMPONENT;
					while (--size);
				}
			}

			if (++idx == 32)
				return;

			if (idx < 16)
			{
				offset -= 2;
				width += 4;
			}
			else if (idx != 16)
			{
				offset += 2;
				width -= 4;
			}

			dstData += pitch;
		} while (TRUE);
	}

	VOID __stdcall DrawSymbol(DWORD* obj, DWORD* data, LONG dstPitch, LONG left, LONG top, RECT* clipper, RECT* rect, DWORD colorFill, DWORD colorShadow, BYTE castShadow, CHAR symbol)
	{
		if (symbol != '\n' && symbol != '\r')
		{
			LONG* font = *(LONG**)(*obj + 4 * *(BYTE*)&symbol + 16);

			POINT shift = { 0, 0 };
			SIZE size = { font[0], font[1] };

			if (Clip(&shift.x, &shift.y, &left, &top, &size.cx, &size.cy, clipper))
			{
				colorFill = Convert565toRGB(colorFill);
				colorShadow = Convert565toRGB(colorShadow);

				DWORD srcPitch = font[3];
				BYTE* src = (BYTE*)font[4] + shift.y * srcPitch + (shift.x >> 3);

				DWORD mod = shift.x % 8;
				if (!mod)
					mod = 8;

				dstPitch >>= 1;
				DWORD* dst = (DWORD*)data + top * dstPitch + left;

				LONG height = size.cy;
				LONG y = top;
				do
				{
					BYTE* srcPtr = src;
					BYTE srcVal = *srcPtr;

					DWORD count = 8 - mod;
					while (count)
					{
						--count;
						srcVal <<= 1;
					}
					DWORD mask = mod;

					DWORD* dstPtr = dst;

					LONG width = size.cx;
					LONG x = left;
					do
					{
						if (srcVal & 0x80)
						{
							*dstPtr = colorFill;

							if (castShadow)
							{
								if (x > rect->left && *(dstPtr - 1) != colorFill)
									*(dstPtr - 1) = colorShadow;

								if (x < rect->right - 1 && *(dstPtr + 1) != colorFill)
									*(dstPtr + 1) = colorShadow;

								if (y > rect->top && *(dstPtr - dstPitch) != colorFill)
									*(dstPtr - dstPitch) = colorShadow;

								if (y < rect->bottom - 1 && *(dstPtr + dstPitch) != colorFill)
									*(dstPtr + dstPitch) = colorShadow;
							}
						}

						++x;

						if (!--mask)
						{
							mask = 8;
							srcVal = *(++srcPtr);
						}
						else
							srcVal <<= 1;

						++dstPtr;
					} while (--width);

					++y;
					src += srcPitch;
					dst += dstPitch;
				} while (--height);
			}
		}
	}

	VOID __declspec(naked) hook_005280D9()
	{
		__asm
		{
			POP EAX
			PUSH ECX
			PUSH EAX
			JMP DrawSymbol
		}
	}

	VOID __stdcall DrawLineHorizontal(DWORD* data, SIZE* sizePitch, LONG left, LONG top, LONG width, DWORD colorFill)
	{
		POINT shift = { 0, 0 };
		LONG height = 1;
		RECT clipper = { 0, 0, sizePitch->cx, sizePitch->cy };

		if (Clip(&shift.x, &shift.y, &left, &top, &width, &height, &clipper))
		{
			colorFill = Convert565toRGB(colorFill);

			DWORD* dst = (DWORD*)data + top * sizePitch->cx + left;
			do
				*dst++ = colorFill;
			while (--width);
		}
	}

	VOID __declspec(naked) hook_0053056A()
	{
		__asm { JMP DrawLineHorizontal }
	}

	VOID __stdcall DrawLineVertical(DWORD* data, SIZE* sizePitch, LONG left, LONG top, LONG height, DWORD colorFill)
	{
		POINT shift = { 0, 0 };
		LONG width = 1;
		RECT clipper = { 0, 0, sizePitch->cx, sizePitch->cy };

		if (Clip(&shift.x, &shift.y, &left, &top, &width, &height, &clipper))
		{
			colorFill = Convert565toRGB(colorFill);

			DWORD* dst = (DWORD*)data + top * sizePitch->cx + left;
			do
			{
				*dst = colorFill;
				dst += sizePitch->cx;
			} while (--height);
		}
	}

	VOID __declspec(naked) hook_00530603()
	{
		__asm { JMP DrawLineVertical }
	}

	VOID __stdcall DrawMinimapGround(DWORD* thisObj, LONG left, LONG top, BlitObject* obj)
	{
		if (left > 0 && left < obj->rect.right && top > 0 && top < obj->rect.bottom)
		{
			SIZE* size = (*(SIZE * (__thiscall**)(DWORD*))(*thisObj + 4))(thisObj);
			LONG pitch = (*(LONG(__thiscall**)(DWORD*))(*thisObj + 12))(thisObj);
			VOID* data = (*(VOID * (__thiscall**)(DWORD*))(*thisObj + 8))(thisObj);

			LONG shiftX = left % size->cx;
			if (shiftX >= size->cx - 4)
				shiftX -= size->cx - 4;

			LONG shiftY = top % size->cy;
			if (shiftY >= size->cy - 2)
				shiftY -= size->cy - 2;

			LONG srcPitch = pitch >> 1;
			DWORD* srcData = (DWORD*)data + shiftY * srcPitch + shiftX;

			LONG dstPitch = obj->pitch >> 1;
			DWORD* dstData = (DWORD*)obj->data + top * dstPitch + left;

			DWORD y = 1;

			if (config.renderer == RendererGDI)
			{
				do
				{
					DWORD* src = srcData;
					DWORD* dst = dstData;

					DWORD x = 3;
					do
					{
						if ((0x4E >> ((y << 2) + x)) & 1)
							*dst = *src;

						++src;
						++dst;
					} while (x--);

					srcData += srcPitch;
					dstData += dstPitch;
				} while (y--);
			}
			else
			{
				do
				{
					DWORD* src = srcData;
					DWORD* dst = dstData;

					DWORD x = 3;
					do
					{
						if ((0x4E >> ((y << 2) + x)) & 1)
							*dst = _byteswap_ulong(_rotl(*src, 8));

						++src;
						++dst;
					} while (x--);

					srcData += srcPitch;
					dstData += dstPitch;
				} while (y--);
			}
		}
	}

	VOID __stdcall DrawCastleBuildings(DWORD* thisObj, BlitObject* obj)
	{
		obj->color = 0xF81F;
		FillColor(obj, obj->color, &obj->rect);

		LONG width = thisObj[12];
		LONG height = thisObj[13];
		if (height > 0 && width > 0)
		{
			BOOL isBlend;
			DWORD blend;

			switch (thisObj[14])
			{
			case 1: // selected line
				isBlend = TRUE;
				blend = config.renderer == RendererGDI ? 0x000000C4 : 0x00C40000;
				break;

			case 2: // unavailable yet
				isBlend = TRUE;
				blend = config.renderer == RendererGDI ? 0x00FF0000 : 0x000000FF;
				break;

			case 3: // other line
				isBlend = TRUE;
				blend = 0x00000000;
				break;

			default:
				isBlend = FALSE;
				break;
			}

			LONG srcPitch = width;
			LONG dstPitch = obj->pitch >> 1;

			DWORD* srcData = *((DWORD**)thisObj[11] + 1);
			DWORD* dstData = (DWORD*)obj->data;

			while (height)
			{
				--height;
				DWORD* src = srcData;
				DWORD* dst = dstData;

				INT count = width;
				while (count)
				{
					--count;
					DWORD pix = *src;
					if ((pix & COLORKEY_AND) != COLORKEY_CHECK)
					{
						if (isBlend)
							pix = Pixel_Blend(pix, blend, 128);

						*dst = pix;
					}

					++src;
					++dst;
				}

				srcData += srcPitch;
				dstData += dstPitch;
			}
		}

		*(BYTE*)&thisObj[9] = 0;
	}

	VOID __stdcall DrawFaces(VOID* dstData, LONG dstPitch, VOID* srcData, LONG srcPitch, LONG top, LONG bottom, LONG width, BYTE isMirror, BYTE k, DWORD minIdx, PixObject* pixData_1, PixObject* pixData_2, VOID* mskData, LONG mskPitch)
	{
		DWORD* source = (DWORD*)srcData + top * srcPitch;
		DWORD* mask = (DWORD*)mskData + top * mskPitch;
		DWORD* destination = (DWORD*)dstData + top * dstPitch + (isMirror ? width - 1 : 0);

		DWORD idx = 0;

		DWORD height = bottom - top;

		if (config.renderer == RendererGDI)
		{
			while (height)
			{
				--height;
				DWORD* src = source;
				DWORD* msk = mask;
				DWORD* dst = destination;

				LONG count = width;
				while (count)
				{
					--count;
					DWORD pix = *src;
					if (mskData || pixData_1->exists)
						pix = Pixel_Blend(pix, mskData ? *msk : _byteswap_ulong(_rotl(pixData_1->color, 8)), k);

					if (idx >= minIdx && pixData_2->exists) // color
						pix = Pixel_Blend(pix, _byteswap_ulong(_rotl(pixData_2->color, 8)), k);

					*dst = pix;

					++src;
					++msk;

					if (isMirror)
						--dst;
					else
						++dst;
				}

				source += srcPitch;
				mask += mskPitch;
				destination += dstPitch;

				++idx;
			}
		}
		else
		{
			while (height)
			{
				--height;
				DWORD* src = source;
				DWORD* msk = mask;
				DWORD* dst = destination;

				LONG count = width;
				while (count)
				{
					--count;
					DWORD pix = *src;
					if (mskData || pixData_1->exists)
						pix = Pixel_Blend(pix, mskData ? *msk : pixData_1->color, k);

					if (idx >= minIdx && pixData_2->exists) // color
						pix = Pixel_Blend(pix, pixData_2->color, k);

					*dst = pix;

					++src;
					++msk;

					if (isMirror)
						--dst;
					else
						++dst;
				}

				source += srcPitch;
				mask += mskPitch;
				destination += dstPitch;

				++idx;
			}
		}
	}

	VOID __stdcall DrawLine(BlitObject* obj, POINT* loc, DWORD count, DWORD color)
	{
		DWORD pitch = obj->pitch >> 1;
		color = Convert565toRGB(color);

		DWORD* data = (DWORD*)obj->data + loc->y * pitch + loc->x;
		while (count)
		{
			--count;
			*data++ = color;
		}
	}

	DWORD __stdcall Color565toRGB(DWORD color)
	{
		return Convert565toRGB(color);
	}

	DWORD back_005383AA;
	VOID __declspec(naked) hook_005383A5()
	{
		__asm {
			MOV EAX, [EBP - 0x1C]
			PUSH EAX
			CALL Color565toRGB
			MOV [EBP - 0x1C], EAX

			MOV EAX, [EBP - 0x14]
			TEST EAX, EAX

			JMP back_005383AA
		}
	}

	DWORD back_00538413;
	VOID __declspec(naked) hook_005383F7()
	{
		__asm {
			SHL EBX, 0x1
			MOV ECX, EAX
			MOV EAX, [ECX + 0x4]
			IMUL EAX, EBX
			ADD ESI, EAX
			MOV EAX, [ECX]
			MOV ECX, [EBP - 0x1C]
			MOV [EAX * 0x4 + ESI], ECX
			INC EDI

			JMP back_00538413
		}
	}

#pragma endregion

#pragma region HD
	VOID __stdcall SetResolution(SIZE* size)
	{
		size->cx = config.mode->width;
		size->cy = config.mode->height;
	}

	DWORD back_00611CEF;
	VOID __declspec(naked) hook_00611C8B()
	{
		__asm
		{
			MOV EAX, [ESI]
			ADD EAX, 0x198
			PUSH EAX
			PUSH back_00611CEF
			JMP SetResolution
		}
	}

	DWORD battleAddress;
	VOID __stdcall CheckBorders(DWORD* object, BOOL isborder)
	{
		config.border.active = isborder & 1;
		if (*(object + 2) != 0xFFFFFFFF)
			config.battle.active = *object == battleAddress;
	}

	VOID __declspec(naked) hook_00538FEB()
	{
		__asm {
			CALL [EAX+0x24]

			PUSH EAX
			PUSH EDI
			CALL CheckBorders

			XOR EAX, EAX
			RETN
		}
	}

	DWORD back_00489136;
	VOID __declspec(naked) hook_00489124()
	{
		__asm {
			CMP [ESI + 0x68] , EBX
			JE lbl_success
			CMP config.border.active, EBX
			JE lbl_back

			lbl_success : MOV EAX, [ECX]
						  ADD ESI, 0x64

						  MOV EDX, [ESI + 0x4]
						  CMP config.border.active, EBX
						  JE lbl_continue
						  SHR EDX, 0x1

						  lbl_continue : PUSH EDX
										 MOV EDX, [ESI]
										 CMP config.border.active, EBX
										 JE lbl_continue2
										 SHR EDX, 0x1

										 lbl_continue2 : PUSH EDX
														MOV ESI, ESP
														PUSH EBX
														PUSH EBX
														PUSH EBX
														PUSH ESI
														PUSH EDI
														CALL[EAX + 0x14]

														ADD ESP, 0x8

														lbl_back : JMP back_00489136
		}
	}
#pragma endregion

	TimeScale gameSpeed;
	DWORD __fastcall GetTimeSpeed(TimeScale* time, DOUBLE scale)
	{
		if (time->scale != scale)
		{
			time->scale = scale;

			time->lastReal = time->real;
			time->lastVirt = time->virt;
		}

		time->real = timeGetTime();
		time->virt = time->lastVirt + DWORD(time->scale * (time->real - time->lastReal));

		return time->virt;
	}

	DWORD __stdcall timeGetTimeHook()
	{
		return GetTimeSpeed(&gameSpeed, config.speed.enabled ? config.speed.value : 1.0);
	}

	DWORD __stdcall GetDoubleClickTimeHook()
	{
		return DWORD((FLOAT)GetDoubleClickTime() * (config.speed.enabled ? config.speed.value : 1.0));
	}

	VOID __declspec(naked) hook_0053A73D()
	{
		__asm
		{
			PUSH EAX
			CALL GetDoubleClickTimeHook
			MOV ECX, EAX
			POP EAX
			RETN
		}
	}

	DWORD scrollTime;
	DWORD __stdcall GetScrollTime(DWORD time)
	{
		scrollTime = time;
		return DWORD(1000.0 / 60.0 * (config.speed.enabled ? config.speed.value : 1.0));
	}

	VOID __declspec(naked) hook_004DFC7A()
	{
		__asm {
			POP EAX
			MOV ECX, [ECX]
			PUSH [ECX + 0x44]
			PUSH EAX
			JMP GetScrollTime
		}
	}

	VOID __declspec(naked) hook_0053CA08()
	{
		__asm {
			POP EAX
			MOV ECX,  [ECX]
			PUSH [ECX + 0x40]
			PUSH EAX
			JMP GetScrollTime
		}
	}

	struct {
		DWORD time;
		FLOAT multi;
		POINT offset;
	} scrollSpeed;

	VOID __stdcall GetScrollOffset(POINT* offset)
	{
		if (scrollSpeed.time != scrollTime)
		{
			scrollSpeed.time = scrollTime;

			DOUBLE off = scrollSpeed.multi - 0.10 * (FLOAT)scrollSpeed.time;
			scrollSpeed.offset.x = (DWORD)MathRound(off * 2.0);
			scrollSpeed.offset.y = (DWORD)MathRound(off);
		}

		*offset = scrollSpeed.offset;
	}

	VOID __declspec(naked) hook_0053D185()
	{
		__asm {
			POP ECX

			PUSH EAX
			PUSH EAX

			PUSH ECX
			PUSH EAX

			LEA ECX, [ESP+0x8]
			PUSH ECX
			CALL GetScrollOffset

			POP EAX
			MOV EDI, [EAX]
			
			RETN
		}
	}

	VOID __declspec(naked) hook_004E0323()
	{
		__asm
		{
			POP ECX

			PUSH EAX
			PUSH EAX
			MOV EAX, ESP
			PUSH ESI
			PUSH EAX

			PUSH ECX
			JMP GetScrollOffset
		}
	}

	BOOL __stdcall GetOpenFileNameHook(LPOPENFILENAMEA lpofn)
	{
		BOOL wasWindowed = config.windowedMode;
		if (!wasWindowed)
			SendMessage(hWndMain, WM_COMMAND, IDM_RES_FULL_SCREEN, NULL);

		BOOL res = GetOpenFileName(lpofn);

		if (!wasWindowed && config.windowedMode)
			SendMessage(hWndMain, WM_COMMAND, IDM_RES_FULL_SCREEN, NULL);

		return res;
	}

	BOOL __stdcall GetSaveFileNameHook(LPOPENFILENAMEA lpofn)
	{
		BOOL wasWindowed = config.windowedMode;
		if (!wasWindowed)
			SendMessage(hWndMain, WM_COMMAND, IDM_RES_FULL_SCREEN, NULL);

		BOOL res = GetSaveFileName(lpofn);

		if (!wasWindowed && config.windowedMode)
			SendMessage(hWndMain, WM_COMMAND, IDM_RES_FULL_SCREEN, NULL);

		return res;
	}

#pragma region CPU patch
	BOOL __stdcall PeekMessageHook(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
	{
		if (PeekMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg))
			return TRUE;

		if (config.coldCPU && !config.ai.thinking)
			Sleep(1);

		return FALSE;
	}

	BOOL __stdcall SetThreadPriorityHook(HANDLE hThread, INT nPriority)
	{
		return TRUE;
	}

	DWORD waitAddress;
	VOID __stdcall DrawDialog(DWORD* lpOld, DWORD dlgNew)
	{
		DWORD dlgOld = *lpOld;
		if (dlgOld != dlgNew)
		{
			*lpOld = dlgNew;

			if (dlgOld)
				((VOID(__thiscall*)(DWORD, BOOL))(**(DWORD**)dlgOld))(dlgOld, TRUE);

			config.ai.waiting = dlgNew && *(DWORD*)dlgNew == waitAddress;
		}
	}

	VOID __declspec(naked) hook_004F1464()
	{
		__asm {
			MOV EAX, [ESP+0x4]
			PUSH EAX
			PUSH ECX
			CALL DrawDialog
			RETN 0x4
		}
	}

	DWORD waitObject;
	DWORD sub_005AE161;
	DWORD sub_006ACED2;
	VOID __declspec(naked) hook_005AE161()
	{
		__asm {
			MOV EAX, [ESP+0x4]
			MOV ECX, [EAX]
			PUSH ECX

			PUSH EAX
			MOV EAX, sub_006ACED2
			CALL sub_005AE161;

			POP ECX
			CMP ECX, 0x1
			JNE non_wait

			MOV waitObject, EAX

			non_wait:
			RETN 0x4
		}
	}

	DWORD sub_0052EE9E;
	VOID __declspec(naked) hook_0053C095()
	{
		__asm {
			CMP ECX, waitObject
			JNE lbl_non
			MOV config.ai.waiting, 0x1
			JMP lbl_cont
			
			lbl_non:
			MOV config.ai.waiting, 0x0

			lbl_cont:
			MOV EAX, [ESP+0x8]
			PUSH EAX
			MOV EAX, [ESP+0x8]
			PUSH EAX
			CALL sub_0052EE9E

			RETN 0x8
		}
	}
#pragma endregion

#pragma region Widescreen Battle
	DWORD sub_00629FAA;
	VOID __declspec(naked) hook_0062891A()
	{
		__asm {
			MOV EAX, config.battle.wide
			TEST EAX, EAX
			JZ non_wide
			RETN 0x4

			non_wide :
			LEA ECX, [EBP-0x1C]
			PUSH ECX
			MOV ECX, [EBP-0x30]
			CALL sub_00629FAA
			RETN 0x4
		}
	}

	DWORD back_0062F52A;
	VOID __declspec(naked) hook_0062F524()
	{
		__asm {
 			MOV EAX, config.battle.wide
			TEST EAX, EAX
			JZ non_wide
			RETN

			non_wide :
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 0x10
			JMP back_0062F52A
		}
	}

	DWORD back_00625688;
	VOID __declspec(naked) hook_00625683()
	{
		__asm {
 			MOV EAX, config.battle.wide
			TEST EAX, EAX
			JZ non_wide
			RETN 0x4

			non_wide :
			PUSH EBP
			MOV EBP, ESP
			PUSH ECX
			PUSH ESI
			JMP back_00625688
		}
	}

	VOID __declspec(naked) hook_00624F2F()
	{
		__asm {
			MOV EAX, config.battle.wide
			TEST EAX, EAX
			JZ non_wide

			INC EDX
			RETN

			non_wide :
			MOV DL, [ECX+0x0B55]
			RETN
		}
	}

	VOID __declspec(naked) hook_0062ED48()
	{
		__asm {
			MOV EAX, config.battle.wide
			TEST EAX, EAX
			JZ non_wide

			MOV DL, [ECX+0x14F7]
			RETN

			non_wide :
			MOV DL, [ECX+0x14F6]
			RETN
		}
	}

	VOID __declspec(naked) hook_0062521D()
	{
		__asm {
			MOV EAX, config.battle.wide
			TEST EAX, EAX
			JZ non_wide
			
			MOV ECX, [ECX+0x1C]
			MOV EAX, [ECX+0x9E4]
			MOV EAX, [EAX+0x8]
			MOV [EAX+0x50], 0x1
			MOV EAX, [ECX+0x9E8]
			MOV EAX, [EAX+0x8]
			MOV [EAX+0x50], 0x1
			RETN

			non_wide :
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 0x10
			JMP back_0062F52A
		}
	}

	VOID __declspec(naked) hook_0062DAA8()
	{
		__asm {
			MOV EAX, config.battle.wide
			TEST EAX, EAX
			JZ non_wide
			
			MOV ECX, [ECX+0x1C]
			MOV EAX, [ECX+0x1384]
			MOV EAX, [EAX+0x8]
			MOV [EAX+0x50], 0x1
			MOV EAX, [ECX+0x1388]
			MOV EAX, [EAX+0x8]
			MOV [EAX+0x50], 0x1
			RETN

			non_wide :
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 0x10
			JMP back_0062F52A
		}
	}

	DWORD sub_00643E80;
	VOID __declspec(naked) hook_0062F5B8()
	{
		__asm {
			MOV EAX, config.battle.wide
			TEST EAX, EAX
			JZ non_wide
			RETN

			non_wide :
			JMP sub_00643E80
		}
	}

	VOID __declspec(naked) hook_00625EAE()
	{
		__asm {
			MOV EAX, config.battle.wide
			TEST EAX, EAX
			POP EAX
			ADD ECX, 0x30
			PUSH ECX
			JZ non_wide

			PUSH 0x1
			PUSH 0x1
			PUSH EAX
			RETN

			non_wide :
			PUSH 0x1
			PUSH 0x0
			PUSH EAX
			RETN
		}
	}

	VOID __declspec(naked) hook_00625EFD()
	{
		__asm {
			MOV EAX, config.battle.wide
			TEST EAX, EAX
			JZ non_wide

			ADD ECX, 0x9F8
			RETN

			non_wide :
			ADD ECX, 0x9E4
			RETN
		}
	}

	VOID __declspec(naked) hook_0062635D()
	{
		__asm {
			MOV EAX, config.battle.wide
			TEST EAX, EAX
			POP EAX
			ADD EDX, 0x30
			PUSH EDX
			JZ non_wide

			PUSH 0x1
			PUSH 0x0
			PUSH EAX
			RETN

			non_wide :
			PUSH 0x1
			PUSH 0x1
			PUSH EAX
			RETN
		}
	}

	VOID __declspec(naked) hook_006263AC()
	{
		__asm {
			MOV EAX, config.battle.wide
			TEST EAX, EAX
			JZ non_wide

			ADD ECX, 0x9E4
			RETN

			non_wide :
			ADD ECX, 0x9F8
			RETN
		}
	}

	VOID __declspec(naked) hook_0062E7FB()
	{
		__asm {
			MOV EAX, config.battle.wide
			TEST EAX, EAX
			POP EAX
			ADD ECX, 0x30
			PUSH ECX
			JZ non_wide

			PUSH 0x0
			PUSH EAX
			RETN

			non_wide :
			PUSH 0x1
			PUSH EAX
			RETN
		}
	}

	VOID __declspec(naked) hook_0062E848()
	{
		__asm {
			MOV EAX, config.battle.wide
			TEST EAX, EAX
			JZ non_wide

			ADD ECX, 0x1398
			RETN

			non_wide :
			ADD ECX, 0x1384
			RETN
		}
	}

	VOID __declspec(naked) hook_0062ECA9()
	{
		__asm {
			MOV EAX, config.battle.wide
			TEST EAX, EAX
			POP EAX
			ADD EDX, 0x30
			PUSH EDX
			JZ non_wide

			PUSH 0x1
			PUSH EAX
			RETN

			non_wide :
			PUSH 0x0
			PUSH EAX
			RETN
		}
	}

	VOID __declspec(naked) hook_0062ECFC()
	{
		__asm {
			MOV EAX, config.battle.wide
			TEST EAX, EAX
			JZ non_wide

			ADD ECX, 0x1384
			RETN

			non_wide :
			ADD ECX, 0x1398
			RETN
		}
	}

	const CHAR* const dlgNames[2] = {
		"DLG_BATTLE_A",
		"DLG_BATTLE_B"
	};

	VOID __stdcall CalcWideBattle()
	{
		if (config.wide.allowed)
		{
			config.battle.zoomable = TRUE;
			config.battle.wide = config.zoom.enabled ? config.zoom.size.width >= WIDE_WIDTH : TRUE;
		}
		else
			config.battle.wide = config.battle.zoomable = FALSE;
	}

	VOID __declspec(naked) hook_006244CA()
	{
		__asm {
			CALL CalcWideBattle

			LEA ECX, dlgNames

			MOV EAX, config.battle.wide
			TEST EAX, EAX
			JZ non_wide

			ADD ECX, 0x4
			
			non_wide :
			MOV ECX, [ECX]
			RETN
		}
	}

	VOID __declspec(naked) hook_0062476C()
	{
		__asm {
			LEA EDX, dlgNames

			MOV EAX, config.battle.wide
			TEST EAX, EAX
			JZ non_wide

			ADD EDX, 0x4
			
			non_wide :
			MOV EDX, [EDX]
			RETN
		}
	}

	const ImageIndices battleIndices = {
		990, 200,
		&battleIndices.indices,
		11,

		0, 96, 346, 0, 279, 104,
		279, 96, 594, 0, 20, 104,
		299, 96, 550, 0, 180, 104,

		511, 96, 346, 104, 180, 104,
		691, 96, 458, 104, 20, 104,
		711, 96, 451, 104, 279, 104,

		0, 64, 346, 208, 148, 32,
		843, 64, 346, 240, 147, 32,
		0, 34, 346, 272, 123, 30,
		869, 35, 346, 302, 121, 29,

		479, 94, 346, 331, 32, 106
	};

	VOID __stdcall ChangeBattleIndices(DWORD* object)
	{
		((DWORD*)object[1])[7] = (DWORD)&battleIndices;
	}

	DWORD sub_00625387;
	VOID __declspec(naked) hook_00625387()
	{
		__asm {
			MOV EDX, config.battle.wide
			TEST EDX, EDX
			JZ non_wide

			PUSH ECX
			PUSH EAX
			CALL ChangeBattleIndices
			POP ECX

			non_wide :
			JMP sub_00625387
		}
	}

	VOID __declspec(naked) hook_0063E6B8()
	{
		__asm {
			SUB ESI, [EAX]

			MOV EAX, config.battle.wide
			TEST EAX, EAX
			JZ non_wide

			SAR ESI, 0x1
			JMP result

			non_wide :
			MOV EAX, [EBP+0x10]
			TEST AL, AL
			JNZ result

			XOR ESI, ESI

			result : MOV [EBP-0x8], ESI
			RETN
		}
	}

	struct {
		Stream stream;
		BOOL loaded;
		BOOL unloaded;
	} dlgOptions;

	BOOL __fastcall GetStr(CHAR* str, INT num, FILE* stream)
	{
		if (!dlgOptions.loaded)
		{
			dlgOptions.loaded = TRUE;
			if (!Main::LoadResource(MAKEINTRESOURCE(IDR_DLG_BATTLE_B), &dlgOptions.stream))
				dlgOptions.unloaded = TRUE;
		}

		if (!dlgOptions.unloaded)
		{
			Stream* stream = &dlgOptions.stream;

			CHAR* p = (CHAR*)stream->data;
			DWORD size = stream->size - stream->position;
			while (TRUE)
			{
				if (!size)
				{
					p = NULL;
					break;
				}

				if (*p == '\n' || *p == '\0')
					break;

				++p;
				--size;
			}

			if (p && p - (CHAR*)stream->data <= num - 1)
			{
				DWORD length = p - (CHAR*)stream->data;
				MemoryCopy(str, stream->data, length);
				stream->position += length + 1;
				stream->data += length + 1;
				str[length] = NULL;
			}
			else
			{
				DWORD length = num - 1;
				MemoryCopy(str, stream->data, length);
				stream->position += length;
				stream->data += length;
				str[length] = NULL;
			}

			if (stream->position == stream->size)
				dlgOptions.unloaded = TRUE;

			return FALSE;
		}

		return TRUE;
	}

	CHAR* __cdecl FileGetStrHook(CHAR* str, INT num, FILE* stream)
	{
		return GetStr(str, num, stream) ? FileGetStr(str, num, stream) : str;
	}

	DWORD sub_fgets;
	CHAR* __stdcall SteamFileGetStrHook(CHAR* str, INT num, FILE* stream)
	{
		return GetStr(str, num, stream) ? ((CHAR * (__stdcall*)(CHAR*, INT, FILE*)) sub_fgets)(str, num, stream) : str;
	}

	DWORD** __stdcall ChangeBattleBgIndices(DWORD** object)
	{
		if (object)
		{
			SIZE* size = (SIZE*)object[1][5];

			if (!battleBgIndices.active)
			{
				battleBgIndices.active = TRUE;
				battleBgIndices.ready = TRUE;
				battleBgIndices.mirror = Random() & 1u;
				battleBgIndices.size = *size;
			}

			ImageIndices* indices = (ImageIndices*)object[1][7];

			BOOL isReversed = indices->size.height >> 31;
			if (battleBgIndices.mirror)
				indices->size.height |= 0x80000000;
			else
				indices->size.height &= 0x7FFFFFFF;

			if (battleBgIndices.mirror != isReversed)
			{
				SpritePosition* pos = (SpritePosition*)indices->lpIndices;
				DWORD count = indices->count;
				while (count)
				{
					--count;
					pos->dstPos.x = indices->size.width - (pos->dstPos.x + pos->srcRect.width);
					pos->srcRect.x = size->cx - (pos->srcRect.x + pos->srcRect.width);
					++pos;
				}
			}
		}

		return object;
	}

	DWORD sub_005A9AE9;
	VOID __declspec(naked) hook_005AEF44()
	{
		__asm {
			MOV EAX, [ESP+0x4]
			PUSH EAX
			CALL sub_005A9AE9

			PUSH EAX
			CALL ChangeBattleBgIndices

			RETN 0x4
		}
	}

	VOID __declspec(naked) hook_00529AEB()
	{
		__asm {
			AND ECX, 0x7FFFFFFF
			MOV [EAX+0x4], ECX
			RETN 0x4
		}
	}
#pragma endregion

#pragma region Debug window& messages
	VOID __stdcall CalcDebugPOsition(POINT* point)
	{
		if (Config::IsZoomed())
		{
			point->x = (config.mode->width - config.zoom.size.width) >> 1;
			point->y = (config.mode->height - config.zoom.size.height) >> 1;
		}
		else
		{
			point->x = 0;
			point->y = 0;
		}
	}

	VOID __declspec(naked) hook_0052EAAE()
	{
		__asm {
			PUSH EAX

			LEA EAX, [EBP-8]
			PUSH EAX
			CALL CalcDebugPOsition

			POP EAX
			XOR ECX, ECX

			RETN
		}
	}

	VOID __stdcall CalcMessage(DWORD* object, POINT* point)
	{
		if (Config::IsZoomed())
		{
			POINT offset = {
				LONG((config.mode->width - config.zoom.size.width) >> 1),
				LONG((config.mode->height - config.zoom.size.height) >> 1)
			};

			point->x += offset.x;

			if (*(BYTE*)(object + 24))
				point->y += offset.y;
			else
				point->y -= offset.y;
		}
	}

	VOID __declspec(naked) hook_00484DD1()
	{
		__asm {
			PUSH ECX

			MOV EAX, [ESP+0xC]
			PUSH EAX
			PUSH ESI
			CALL CalcMessage
			
			POP ECX
			MOV EAX, [ECX]
			JMP DWORD PTR [EAX+0x14]
		}
	}

	TimeScale msgScale;
	DWORD __stdcall MessageTimeout()
	{
		return GetTimeSpeed(&msgScale, config.msgTimeScale.value);
	}

	VOID __declspec(naked) hook_00484CD0()
	{
		__asm {
			PUSH ECX

			CALL MessageTimeout
			
			POP ECX
			RETN
		}
	}
#pragma endregion

#pragma region Map direction bit - mask Fix
	VOID __stdcall SetMapBitMask96(BYTE* mask, POINT* p)
	{
		if (p->x >= 0 && p->y >= 0 && p->x < 144 && p->y < 144)
		{
			BYTE* address = &mask[16 * p->y] + (p->x >> 3);
			*address |= 1 << (p->x & 7);
		}
	}

	VOID __declspec(naked) hook_005BBF85()
	{
		__asm
		{
			MOV EAX, [ESP+0x4]
			PUSH EAX
			PUSH ECX
			CALL SetMapBitMask96

			RETN 0x4
		}
	}

	VOID __stdcall SetMapBitMask144(BYTE* mask, POINT* p)
	{
		if (p->x >= 0 && p->y >= 0 && p->x < 144 && p->y < 144)
		{
			BYTE* address = &mask[18 * p->y] + (p->x >> 3);
			*address |= 1 << (p->x & 7);
		}
	}

	VOID __declspec(naked) hook_005C848D()
	{
		__asm
		{
			MOV EAX, [ESP+0x4]
			PUSH EAX
			PUSH ECX
			CALL SetMapBitMask144

			RETN 0x4
		}
	}
#pragma endregion

#pragma region Locale
	INT __cdecl IsAlphaHook(BYTE ch)
	{
		return IsAlpha(ch);
	}

	INT __cdecl IsAlNumHook(BYTE ch)
	{
		return IsAlNum(ch);
	}

	INT __cdecl IsDigitHook(BYTE ch)
	{
		return IsDigit(ch);
	}

	INT __cdecl IsSpaceHook(BYTE ch)
	{
		return IsSpace(ch);
	}

	INT __cdecl IsPunctHook(BYTE ch)
	{
		return IsPunct(ch);
	}

	INT __cdecl IsCntrlHook(BYTE ch)
	{
		return IsCntrl(ch);
	}

	INT __cdecl IsUpperHook(BYTE ch)
	{
		return IsUpper(ch);
	}

	INT __cdecl ToUpperHook(BYTE ch)
	{
		return ToUpper(ch);
	}

	INT __cdecl ToLowerHook(BYTE ch)
	{
		return ToLower(ch);
	}

	const CHAR* __cdecl StrCharHook(const CHAR* str, BYTE ch)
	{
		return StrChar(str, ch);
	}

	const VOID* __cdecl MemoryCharHook(const VOID* block, BYTE ch, size_t length)
	{
		return MemoryChar(block, ch, length);
	}

	BYTE wideCharBuffer[4096];

	BOOL __fastcall ConvertChars(const CHAR* srcData, UINT srcCP, CHAR* dstData, UINT dstCP)
	{
		DWORD srcLength = StrLength(srcData);
		if (srcLength)
		{
			INT dstLength = MultiByteToWideChar(srcCP, NULL, srcData, srcLength, (LPWSTR)wideCharBuffer, sizeof(wideCharBuffer));
			return dstLength && WideCharToMultiByte(dstCP, NULL, (LPWSTR)wideCharBuffer, dstLength, dstData, srcLength, NULL, NULL);
		}
		else
		{
			*dstData = NULL;
			return TRUE;
		}
	}

	BOOL __stdcall OemToCharHook(LPCSTR pSrc, LPSTR pDst)
	{
		if (ConvertChars(pSrc, config.locales.current.oem, pDst, config.locales.current.ansi))
			return TRUE;

		return OemToChar(pSrc, pDst);
	}

	BOOL __stdcall CharToOemHook(LPCSTR pSrc, LPSTR pDst)
	{
		if (ConvertChars(pSrc, config.locales.current.ansi, pDst, config.locales.current.oem))
			return TRUE;

		return CharToOem(pSrc, pDst);
	}
#pragma endregion

#pragma region Print text
	DWORD gameObject;
	DWORD sub_PrintText;
	VOID __fastcall PrintText(CHAR* str)
	{
		if (gameObject)
			((VOID(__thiscall*)(DWORD, CHAR*))(sub_PrintText))(gameObject, str);
	}

	DWORD sub_GameObjectInit;
	VOID __declspec(naked) hook_00405A1A()
	{
		__asm
		{
			MOV gameObject, ECX
			JMP sub_GameObjectInit
		}
	}

	DWORD sub_GameObjectDeInit;
	VOID __declspec(naked) hook_0048A93C()
	{
		__asm {
			MOV EAX, gameObject
			CMP ECX, EAX
			JNE lbl_return

			XOR EAX, EAX
			MOV gameObject, EAX

			lbl_return : JMP sub_GameObjectDeInit
		}
	}
#pragma endregion

#pragma region AI list fix
	DWORD sub_GetType;
	DWORD sub_45287F;
	DWORD __stdcall sub_45287F_Hook(DWORD a1, DWORD a2, DWORD a3, DWORD type, DWORD a5, DWORD a6)
	{
		type = ((DWORD(__stdcall*)(DWORD))sub_GetType)(a5 - 4);
		return ((DWORD(__stdcall*)(DWORD, DWORD, DWORD, DWORD, DWORD, DWORD))sub_45287F)(a1, a2, a3, type, a5, a6);
	}
#pragma endregion

#pragma region Sacred Lands
	HANDLE hInterf;
	BOOL isStrategic;
	DWORD* lineList;

	struct {
		DWORD pitch;
		BYTE* buffer;
	} hud;

	struct SlBlitObject {
		BYTE* data;
		INT pitch;
		DWORD unknown;
		RECT* rc;
	} slBLitObject;

	union
	{
		struct
		{
			DWORD iso01;
			DWORD iso02;
			DWORD isoWait;
			DWORD stra00;
			DWORD stra01;
			DWORD stra02;
		} game;
		struct
		{
			DWORD iso;
			DWORD pal;
			DWORD info;
			DWORD object;
			DWORD map;
			DWORD land;
			DWORD except;
		} editor;
	} dlgAddress;

	union
	{
		struct {
			Stream iso;
			Stream isoView;
			Stream stra;
			Stream straSpell;
			Stream straWait;
		} game;
		struct {
			Stream iso;
			Stream map;
			Stream land;
			Stream except;
		} editor;
	} dlgResources;

	DWORD** __stdcall LoadImageV1(DWORD** object, CHAR* name)
	{
		if (!config.isEditor)
		{
			if (!StrCompare(name, "ISOX01IX"))
				dlgAddress.game.iso01 = object[1][1];
			else if (!StrCompare(name, "ISOX02IX"))
				dlgAddress.game.iso02 = object[1][1];
			else if (!StrCompare(name, "ISOWAIT"))
				dlgAddress.game.isoWait = object[1][1];
			else if (!StrCompare(name, "STRA00IX"))
				dlgAddress.game.stra00 = object[1][1];
			else if (!StrCompare(name, "STRA01IX"))
				dlgAddress.game.stra01 = object[1][1];
			else if (!StrCompare(name, "STRA02IX"))
				dlgAddress.game.stra02 = object[1][1];
		}
		else
		{
			if (!StrCompare(name, "DLG_ISO"))
				dlgAddress.editor.iso = object[1][1];
			else if (!StrCompare(name, "DL_IPAL"))
				dlgAddress.editor.pal = object[1][1];
			else if (!StrCompare(name, "DLG_INFO"))
				dlgAddress.editor.info = object[1][1];
			else if (!StrCompare(name, "DLG_OBJ"))
				dlgAddress.editor.object = object[1][1];
			else if (!StrCompare(name, "DLG_MAP"))
				dlgAddress.editor.map = object[1][1];
			else if (!StrCompare(name, "DLG_LAND"))
				dlgAddress.editor.land = object[1][1];
			else if (!StrCompare(name, "DL_EXCE"))
				dlgAddress.editor.except = object[1][1];
		}

		return object;
	}

	VOID __declspec(naked) hook_004F0B39()
	{
		__asm
		{
			POP EBX
			LEAVE

			MOV ECX, [ESP+0x8]
			PUSH ECX
			PUSH EAX
			CALL LoadImageV1

			RETN 0x8
		}
	}

	VOID BlitHUD(BYTE* dstData, INT dstPitch, BYTE* srcData, INT srcPitch, Rect* srcRect, Rect* dstRect, BOOL revX, BOOL revY)
	{
		if (dstRect->width && dstRect->height)
		{
			srcData += srcRect->y * srcPitch + srcRect->x;
			dstData += dstRect->y * dstPitch + dstRect->x;

			if (revX)
				srcData += srcRect->width - 1;

			if (revY)
			{
				srcData += (srcRect->height - 1) * srcPitch;
				srcPitch = -srcPitch;
			}

			INT height = dstRect->height;
			do
			{
				if (!revX)
					MemoryCopy(dstData, srcData, dstRect->width);
				else
				{
					BYTE* src = srcData;
					BYTE* dst = dstData;

					INT width = dstRect->width;
					do
						*dst++ = *src--;
					while (--width);
				}

				srcData += srcPitch;
				dstData += dstPitch;
			} while (--height);
		}
	}

	VOID ClearHUD(BYTE* data, INT pitch, Rect* rect)
	{
		if (rect->width && rect->height)
		{
			data += rect->y * pitch + rect->x;

			LONG height = rect->height;
			do
			{
				MemoryZero(data, rect->width);
				data += pitch;
			} while (--height);
		}
	}

	VOID __stdcall StartDecodeImage(DWORD object, SlBlitObject* surface)
	{
		if (!config.isEditor)
		{
			if (object == dlgAddress.game.iso01)
			{
				RECT check = { 0, 0, *(LONG*)&config.mode->width - 640 + 464, *(LONG*)&config.mode->height };

				if (surface->rc)
				{
					if (!MemoryCompare(surface->rc, &check, sizeof(RECT)))
					{
						slBLitObject.data = surface->data;
						slBLitObject.pitch = surface->pitch;

						surface->data = hud.buffer;
						surface->pitch = hud.pitch;
					}
					else
					{
						check = { 64, *(LONG*)&config.mode->height - 480 + 452, 91, *(LONG*)&config.mode->height - 480 + 472 };
						RECT check2 = { 91, *(LONG*)&config.mode->height - 480 + 453, 414, *(LONG*)&config.mode->height - 480 + 470 };

						// bottom bar
						if (!MemoryCompare(surface->rc, &check, sizeof(RECT)) || !MemoryCompare(surface->rc, &check2, sizeof(RECT)))
						{
							Rect srcRect = { surface->rc->left, 480 - *(INT*)&config.mode->height + surface->rc->top, surface->rc->right - surface->rc->left, surface->rc->bottom - surface->rc->top };
							Rect dstRect = { srcRect.x, surface->rc->top, srcRect.width, srcRect.height };

							BlitHUD(surface->data, surface->pitch, hud.buffer, hud.pitch, &srcRect, &dstRect, FALSE, FALSE);
						}
					}
				}
			}
			else if (object == dlgAddress.game.iso02)
			{
				RECT check = { *(LONG*)&config.mode->width - 640 + 463, 0, *(LONG*)&config.mode->width, 201 };

				if (surface->rc && !MemoryCompare(surface->rc, &check, sizeof(RECT)))
				{
					slBLitObject.data = surface->data;
					slBLitObject.pitch = surface->pitch;

					surface->data = hud.buffer;
					surface->pitch = hud.pitch;
				}
			}
			else if (object == dlgAddress.game.isoWait || object == dlgAddress.game.stra00 || object == dlgAddress.game.stra01 || object == dlgAddress.game.stra02)
			{
				RECT check = { *(LONG*)&config.mode->width - 640 + 463, 201, *(LONG*)&config.mode->width, *(LONG*)&config.mode->height };

				if (surface->rc && !MemoryCompare(surface->rc, &check, sizeof(RECT)))
				{
					slBLitObject.data = surface->data;
					slBLitObject.pitch = surface->pitch;

					surface->data = hud.buffer;
					surface->pitch = hud.pitch;
				}
			}
		}
		else
		{
			if (object == dlgAddress.editor.iso)
			{
				RECT check = { 0, 0, *(LONG*)&config.mode->width - 640 + 461, *(LONG*)&config.mode->height };

				if (surface->rc && !MemoryCompare(surface->rc, &check, sizeof(RECT)))
				{
					slBLitObject.data = surface->data;
					slBLitObject.pitch = surface->pitch;

					surface->data = hud.buffer;
					surface->pitch = hud.pitch;
				}
			}
			else if (object == dlgAddress.editor.info || object == dlgAddress.editor.object || object == dlgAddress.editor.map || object == dlgAddress.editor.land || object == dlgAddress.editor.except)
			{
				RECT check = { *(LONG*)&config.mode->width - 640 + 461, 215, *(LONG*)&config.mode->width, 480 };

				if (surface->rc && !MemoryCompare(surface->rc, &check, sizeof(RECT)))
				{
					slBLitObject.data = surface->data;
					slBLitObject.pitch = surface->pitch;

					surface->data = hud.buffer;
					surface->pitch = hud.pitch;
				}
			}
		}
	}

	VOID __declspec(naked) hook_004F12D6()
	{
		__asm
		{
			POP EAX
			PUSH EDI
			MOV EDI, [ESP+0x30]
			PUSH EAX

			MOV EAX, [ESP+0x28]
			PUSH EDI
			PUSH EAX
			CALL StartDecodeImage

			RETN
		}
	}

	VOID __stdcall EndDecodeImage(DWORD object, POINT pos, SlBlitObject* surface, DWORD func, DWORD a6)
	{
		if (surface->data == hud.buffer)
		{
			surface->data = slBLitObject.data;
			surface->pitch = slBLitObject.pitch;

			if (!config.isEditor)
			{
				if (object == dlgAddress.game.iso01)
				{
					// main
					{
						Rect rect = { 0, 0, 463, 480 };
						ClearHUD(surface->data, surface->pitch, &rect);
					}

					Rect srcRect, dstRect;

					// top bar
					{
						srcRect = { 0, 0, 463, 26 };
						dstRect = srcRect;

						INT max = *(INT*)&config.mode->width - 177;
						BOOL rev = FALSE;
						for (INT i = dstRect.x; i < max; i += dstRect.width)
						{
							dstRect.x = i;

							INT min = max - i;
							dstRect.width = min < dstRect.width ? min : dstRect.width;

							BlitHUD(surface->data, surface->pitch, hud.buffer, hud.pitch, &srcRect, &dstRect, rev, FALSE);

							rev = !rev;
						}
					}

					// left bar
					{
						srcRect = { 0, 26, 15, 454 - 42 };
						dstRect = srcRect;

						INT max = *(INT*)&config.mode->height - 42;
						BOOL rev = FALSE;
						for (INT j = dstRect.y; j < max; j += dstRect.height)
						{
							dstRect.y = j;

							INT min = max - j;
							dstRect.height = min < dstRect.height ? min : dstRect.height;

							BlitHUD(surface->data, surface->pitch, hud.buffer, hud.pitch, &srcRect, &dstRect, FALSE, rev);

							rev = !rev;
						}

						srcRect = { 0, 480 - 42, 15, 42 };
						dstRect = { 0, *(INT*)&config.mode->height - 42, 15, 42 };
						BlitHUD(surface->data, surface->pitch, hud.buffer, hud.pitch, &srcRect, &dstRect, FALSE, FALSE);
					}

					// bottom bar
					{
						srcRect = { 15, 480 - 35, 59, 35 };
						dstRect = { 15, *(INT*)&config.mode->height - 35, 59, 35 };
						BlitHUD(surface->data, surface->pitch, hud.buffer, hud.pitch, &srcRect, &dstRect, FALSE, FALSE);

						srcRect = { 640 - 239, 480 - 35, 62, 35 };
						dstRect = { *(INT*)&config.mode->width - 239, *(INT*)&config.mode->height - 35, 62, 35 };
						BlitHUD(surface->data, surface->pitch, hud.buffer, hud.pitch, &srcRect, &dstRect, FALSE, FALSE);

						srcRect = { 74, 480 - 35, 327, 35 };
						dstRect = { 74, *(INT*)&config.mode->height - 35, 327, 35 };

						INT max = *(INT*)&config.mode->width - 239;
						BOOL rev = FALSE;
						for (INT i = dstRect.x; i < max; i += dstRect.width)
						{
							dstRect.x = i;

							INT min = max - i;
							dstRect.width = min < dstRect.width ? min : dstRect.width;

							BlitHUD(surface->data, surface->pitch, hud.buffer, hud.pitch, &srcRect, &dstRect, rev, FALSE);

							rev = !rev;
						}

						srcRect = { 64, *(INT*)&config.mode->height - 35, 350, 35 };
						dstRect = { 64, 480 - 35, 350, 35 };

						BlitHUD(hud.buffer, hud.pitch, surface->data, surface->pitch, &srcRect, &dstRect, FALSE, FALSE);
					}
				}
				else if (object == dlgAddress.game.iso02)
				{
					surface->data = slBLitObject.data;
					surface->pitch = slBLitObject.pitch;

					// main
					Rect rect = { surface->rc->left, surface->rc->top, surface->rc->right - surface->rc->left, surface->rc->bottom - surface->rc->top };
					BlitHUD(surface->data, surface->pitch, hud.buffer, hud.pitch, &rect, &rect, FALSE, FALSE);
				}
				else if (object == dlgAddress.game.isoWait || object == dlgAddress.game.stra00 || object == dlgAddress.game.stra01 || object == dlgAddress.game.stra02)
				{
					surface->data = slBLitObject.data;
					surface->pitch = slBLitObject.pitch;

					// main
					{
						Rect rect = { *(INT*)&config.mode->width - 177, 201, 177, 480 - 201 - 35 };
						BlitHUD(surface->data, surface->pitch, hud.buffer, hud.pitch, &rect, &rect, FALSE, FALSE);
					}

					// bottom bar
					{
						Rect srcRect = { *(INT*)&config.mode->width - 177, 480 - 35, 177, 35 };
						Rect dstRect = { *(INT*)&config.mode->width - 177, *(INT*)&config.mode->height - 35, 177, 35 };
						BlitHUD(surface->data, surface->pitch, hud.buffer, hud.pitch, &srcRect, &dstRect, FALSE, FALSE);

						srcRect = { *(INT*)&config.mode->width - 177, 27, 177, 158 };
						dstRect = { *(INT*)&config.mode->width - 177, 480 - 35, 177, 158 };

						INT max = *(INT*)&config.mode->height - 35;
						INT min = max - dstRect.y;
						dstRect.height = min < dstRect.height ? min : dstRect.height;

						BlitHUD(surface->data, surface->pitch, hud.buffer, hud.pitch, &srcRect, &dstRect, FALSE, FALSE);

						srcRect = { *(INT*)&config.mode->width - 177, 32, 177, 153 };
						dstRect = { *(INT*)&config.mode->width - 177, 480 - 35 + 158, 177, 153 };

						BOOL rev = TRUE;
						for (INT j = dstRect.y; j < max; j += dstRect.height)
						{
							dstRect.y = j;

							INT min = max - dstRect.y;
							dstRect.height = min < dstRect.height ? min : dstRect.height;

							BlitHUD(surface->data, surface->pitch, hud.buffer, hud.pitch, &srcRect, &dstRect, FALSE, rev);

							rev = !rev;
						}
					}
				}
			}
			else
			{
				if (object == dlgAddress.editor.iso)
				{
					// main
					{
						Rect rect = { 0, 0, 461, 480 };
						ClearHUD(surface->data, surface->pitch, &rect);
					}

					Rect srcRect, dstRect;

					// top bar
					{
						srcRect = { 0, 0, 13, 31 };
						dstRect = srcRect;

						BlitHUD(surface->data, surface->pitch, hud.buffer, hud.pitch, &srcRect, &dstRect, FALSE, FALSE);

						srcRect = { 13, 0, 448, 31 };
						dstRect = srcRect;

						INT max = *(INT*)&config.mode->width - 640 + 461;
						BOOL rev = FALSE;
						for (INT i = dstRect.x; i < max; i += dstRect.width)
						{
							dstRect.x = i;

							INT min = max - i;
							dstRect.width = min < dstRect.width ? min : dstRect.width;

							BlitHUD(surface->data, surface->pitch, hud.buffer, hud.pitch, &srcRect, &dstRect, rev, FALSE);

							rev = !rev;
						}
					}

					// body
					{
						srcRect = { 13, 31, 448, 420 };
						dstRect = srcRect;

						INT maxX = *(INT*)&config.mode->width - 640 + 461;
						INT maxY = *(INT*)&config.mode->height - 29;

						for (INT j = srcRect.y; j < maxY; j += srcRect.height)
						{
							dstRect.y = j;

							INT min = maxY - j;
							dstRect.height = min < srcRect.height ? min : srcRect.height;

							for (INT i = srcRect.x; i < maxX; i += srcRect.width)
							{
								dstRect.x = i;

								min = maxX - i;
								dstRect.width = min < srcRect.width ? min : srcRect.width;

								BlitHUD(surface->data, surface->pitch, hud.buffer, hud.pitch, &srcRect, &dstRect, FALSE, FALSE);
							}
						}
					}

					// left bar
					{
						srcRect = { 0, 31, 13, 420 };
						dstRect = srcRect;

						INT max = *(INT*)&config.mode->height - 29;
						BOOL rev = FALSE;
						for (INT j = dstRect.y; j < max; j += dstRect.height)
						{
							dstRect.y = j;

							INT min = max - j;
							dstRect.height = min < dstRect.height ? min : dstRect.height;

							BlitHUD(surface->data, surface->pitch, hud.buffer, hud.pitch, &srcRect, &dstRect, FALSE, rev);

							rev = !rev;
						}
					}

					// bottom bar
					{
						srcRect = { 0, 451, 13, 29 };
						dstRect = { 0, *(INT*)&config.mode->height - 480 + 451, 13, 29 };

						BlitHUD(surface->data, surface->pitch, hud.buffer, hud.pitch, &srcRect, &dstRect, FALSE, FALSE);

						srcRect = { 13, 451, 448, 29 };
						dstRect = { 13, *(INT*)&config.mode->height - 480 + 451, 448, 29 };

						INT max = *(INT*)&config.mode->width - 640 + 461;
						BOOL rev = FALSE;
						for (INT i = dstRect.x; i < max; i += dstRect.width)
						{
							dstRect.x = i;

							INT min = max - i;
							dstRect.width = min < dstRect.width ? min : dstRect.width;

							BlitHUD(surface->data, surface->pitch, hud.buffer, hud.pitch, &srcRect, &dstRect, rev, FALSE);

							rev = !rev;
						}
					}
				}
				else if (object == dlgAddress.editor.info || object == dlgAddress.editor.object || object == dlgAddress.editor.map || object == dlgAddress.editor.land || object == dlgAddress.editor.except)
				{
					Rect srcRect, dstRect;

					// left bar
					{
						srcRect = { *(INT*)&config.mode->width - 640 + 461, 215, 179, 236 };
						dstRect = srcRect;

						INT max = *(INT*)&config.mode->height - 2;
						BOOL rev = FALSE;
						for (INT j = dstRect.y; j < max; j += dstRect.height)
						{
							dstRect.y = j;

							INT min = max - j;
							dstRect.height = min < dstRect.height ? min : dstRect.height;

							BlitHUD(surface->data, surface->pitch, hud.buffer, hud.pitch, &srcRect, &dstRect, FALSE, rev);

							rev = !rev;
						}
					}

					{
						srcRect = { *(INT*)&config.mode->width - 640 + 461, 451, 2, 29 };
						dstRect = { *(INT*)&config.mode->width - 640 + 461, *(INT*)&config.mode->height - 480 + 451, 2, 29 };

						BlitHUD(surface->data, surface->pitch, hud.buffer, hud.pitch, &srcRect, &dstRect, FALSE, FALSE);
					}

					{
						srcRect = { *(INT*)&config.mode->width - 640 + 461 + 2, 478, 177, 2 };
						dstRect = { *(INT*)&config.mode->width - 640 + 461 + 2, *(INT*)&config.mode->height - 2, 177, 2 };

						BlitHUD(surface->data, surface->pitch, hud.buffer, hud.pitch, &srcRect, &dstRect, FALSE, FALSE);
					}
				}
			}
		}
	}

	VOID __declspec(naked) hook_004F157A()
	{
		__asm
		{
			MOV [ESP+0x30], EDI

			POP EDI
			POP ESI
			POP EBP
			POP EBX
			ADD ESP, 0x10
			JMP EndDecodeImage
		}
	}

	VOID __stdcall CreateIsoDialog(CHAR* name)
	{
		isStrategic = !StrCompare(name, "DLG_STRATEGIC");
	}

	DWORD off_0046541D;
	VOID __declspec(naked) hook_0046541D()
	{
		__asm
		{
			PUSH ECX
			MOV EAX, [ESP+0x10]
			PUSH EAX
			CALL CreateIsoDialog

			POP ECX
			MOV EAX, off_0046541D
			RETN
		}
	}

	VOID __stdcall CreateViewDialog(CHAR** name, POINT* p1, POINT* p2)
	{
		if (!config.isEditor)
		{
			if (isStrategic)
			{
				if (!StrCompare(*name, "DLG_ISO_VIEW"))
					*name = "DLG_ISO_VIEW_NEW";
				else if (!StrCompare(*name, "DLG_ISO"))
					*name = "DLG_ISO_NEW";
			}
		}
		else
		{
			if (!StrCompare(*name, "DLG_ISO"))
			{
				p2->x = config.mode->width;
				p2->y = config.mode->height;
			}
			else if (!StrCompare(*name, "DLG_ISO_PAL"))
			{
				p1->x = config.mode->width - 640 + 461;
				p2->x = config.mode->width;
				p2->y = config.mode->height;
			}
			else if (!StrCompare(*name, "DLG_INFO") || !StrCompare(*name, "DLG_OBJECTS"))
			{
				p1->x = config.mode->width - 640 + 461;
				p2->x = config.mode->width;
				p2->y = config.mode->height;
			}
			else if (!StrCompare(*name, "DLG_MAP"))
			{
				p1->x = config.mode->width - 640 + 461;
				p2->x = config.mode->width;
				p2->y = config.mode->height;
			}
			else if (!StrCompare(*name, "DLG_LANDMARK"))
			{
				p1->x = config.mode->width - 640 + 461;
				p2->x = config.mode->width;
				p2->y = config.mode->height;
			}
			else if (!StrCompare(*name, "DLG_EXCEPTIONAL"))
			{
				p1->x = config.mode->width - 640 + 461;
				p2->x = config.mode->width;
				p2->y = config.mode->height;
			}
		}
	}

	DWORD off_0055106A;
	VOID __declspec(naked) hook_0055106A()
	{
		__asm
		{
			PUSH ECX
			MOV EAX, [ESP+0x1C]
			PUSH EAX
			MOV EAX, [ESP+0x1C]
			PUSH EAX
			LEA EAX, [ESP+0x18]
			PUSH EAX
			CALL CreateViewDialog

			POP ECX
			MOV EAX, off_0055106A
			RETN
		}
	}

	HANDLE __stdcall CreateFileHook(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
	{
		HANDLE hFile = CreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

		CHAR* p = StrLastChar((CHAR*)lpFileName, '\\');
		if (p && !StrCompareInsensitive(p, !config.isEditor ? "\\Interf.dlg" : "\\ScenEdit.dlg"))
			hInterf = hFile;

		return hFile;
	}

	DWORD __fastcall CalcStrNumbers(Stream* stream)
	{
		DWORD res = 0;
		BOOL found = FALSE;
		CHAR* p = (CHAR*)stream->data;
		DWORD size = stream->size;

		if (size)
		{
			while (--size)
			{
				if (p[0] == '%' && p[1] == 'd')
				{
					++res;
					++p;
					--size;
				}

				++p;
			}
		}

		return res;
	}

	DWORD __stdcall GetFileSizeHook(HANDLE hFile, LPDWORD lpFileSizeHigh)
	{
		DWORD size = GetFileSize(hFile, lpFileSizeHigh);

		if (hInterf && hFile == hInterf)
		{
			if (!config.isEditor)
			{
				if (Main::LoadResource(MAKEINTRESOURCE(IDR_DLG_ISO_NEW), &dlgResources.game.iso))
					size += dlgResources.game.iso.size + CalcStrNumbers(&dlgResources.game.iso) * 9 + 4;

				if (Main::LoadResource(MAKEINTRESOURCE(IDR_DLG_ISO_VIEW_NEW), &dlgResources.game.isoView))
					size += dlgResources.game.isoView.size + CalcStrNumbers(&dlgResources.game.isoView) * 9 + 4;

				if (Main::LoadResource(MAKEINTRESOURCE(IDR_DLG_STRATEGIC_NEW), &dlgResources.game.stra))
					size += dlgResources.game.stra.size + CalcStrNumbers(&dlgResources.game.stra) * 9 + 4;

				if (Main::LoadResource(MAKEINTRESOURCE(IDR_DLG_STRAT_SPELL_NEW), &dlgResources.game.straSpell))
					size += dlgResources.game.straSpell.size + CalcStrNumbers(&dlgResources.game.straSpell) * 9 + 4;

				if (Main::LoadResource(MAKEINTRESOURCE(IDR_DLG_STRAT_WAIT_NEW), &dlgResources.game.straWait))
					size += dlgResources.game.straWait.size + CalcStrNumbers(&dlgResources.game.straWait) * 9 + 4;
			}
			else
			{
				if (Main::LoadResource(MAKEINTRESOURCE(IDR_DLG_ISO_EDIT), &dlgResources.editor.iso))
					size += dlgResources.editor.iso.size + CalcStrNumbers(&dlgResources.editor.iso) * 9 + 4;

				if (Main::LoadResource(MAKEINTRESOURCE(IDR_DLG_MAP_EDIT), &dlgResources.editor.map))
					size += dlgResources.editor.map.size + CalcStrNumbers(&dlgResources.editor.map) * 9 + 4;

				if (Main::LoadResource(MAKEINTRESOURCE(IDR_DLG_LANDMARK_EDIT), &dlgResources.editor.land))
					size += dlgResources.editor.land.size + CalcStrNumbers(&dlgResources.editor.land) * 9 + 4;

				if (Main::LoadResource(MAKEINTRESOURCE(IDR_DLG_EXCEPTIONAL_EDIT), &dlgResources.editor.except))
					size += dlgResources.editor.except.size + CalcStrNumbers(&dlgResources.editor.except) * 9 + 4;
			}
		}

		return size;
	}

	VOID __cdecl FormatDialog(Stream* stream, CHAR** buffer, ...)
	{
		CHAR* format = (CHAR*)MemoryAlloc(stream->size + 1);
		if (format)
		{
			MemoryCopy(format, stream->data, stream->size);
			format[stream->size] = NULL;

			va_list args;
			va_start(args, buffer);
			StrPrintVar(*buffer, format, args);
			va_end(args);

			MemoryFree(format);

			*buffer += StrLength(*buffer);
			StrCopy(*buffer, "\r\n\r\n");
			*buffer += 4;
		}
	}

	BOOL __stdcall ReadFileHook(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
	{
		DWORD read = 0;

		if (hInterf && hFile == hInterf)
		{
			DisplayMode* mode = config.mode;
			CHAR* dst = (CHAR*)lpBuffer;

			if (!config.isEditor)
			{
				FormatDialog(&dlgResources.game.iso, &dst, mode->width - 640 + 464, mode->height,
					mode->height - 480 + 453, mode->height - 480 + 470,
					mode->width - 640 + 464, mode->height - 480 + 445,
					mode->height - 480 + 452, mode->height - 480 + 472);

				FormatDialog(&dlgResources.game.isoView, &dst, mode->width, mode->height,
					mode->width - 640 + 464, mode->height,
					mode->width - 640 + 463, mode->width,
					mode->width - 640 + 463, mode->width);

				FormatDialog(&dlgResources.game.stra, &dst, mode->height - 480 + 279,
					mode->height - 480 + 244, mode->height - 480 + 279,
					mode->height - 480 + 254, mode->height - 480 + 274);

				FormatDialog(&dlgResources.game.straSpell, &dst, mode->height - 480 + 279);

				FormatDialog(&dlgResources.game.straWait, &dst, mode->height - 480 + 279);
			}
			else
			{
				FormatDialog(&dlgResources.editor.iso, &dst, mode->width - 640 + 461, mode->height,
					mode->width - 640 + 461, mode->height - 480 + 451);

				FormatDialog(&dlgResources.editor.map, &dst);

				FormatDialog(&dlgResources.editor.land, &dst);

				FormatDialog(&dlgResources.editor.except, &dst);
			}

			read = dst - (CHAR*)lpBuffer;
			nNumberOfBytesToRead -= read;
			lpBuffer = dst;
		}

		if (ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped))
		{
			*lpNumberOfBytesRead += read;
			return TRUE;
		}

		return FALSE;
	}

	BOOL __stdcall CloseHandleHook(HANDLE hObject)
	{
		if (hInterf && hObject == hInterf)
			hInterf = NULL;

		return CloseHandle(hObject);
	}

	VOID __stdcall PopolateList(DWORD** object, Size* size)
	{
		object[2] = lineList;

		DWORD total = 0;
		DWORD* item = lineList;
		DWORD count = size->height;
		while (count)
		{
			--count;
			*item++ = total;
			total += size->width;
		}
	}
#pragma endregion

#pragma region Skip Draw
	DWORD sub_00512BF0;
	VOID __declspec(naked) hook_00512BF0()
	{
		__asm {
			MOV EAX, config.drawEnabled
			TEST EAX, EAX
			JNZ lbl_draw

			MOV EDX, [ECX]
			MOV EAX, [EDX+48]
			MOV ECX, [EAX+20]
			MOV EAX, [ECX+8]
			PUSH 1
			PUSH 0
			PUSH EAX
			MOV ECX, [EAX]
			CALL [ECX+44]
			RETN

			lbl_draw:
			JMP sub_00512BF0
		}
	}
#pragma endregion

	BOOL __stdcall FakeEntryPoint(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
	{
		return TRUE;
	}

#pragma optimize("s", on)
	VOID __fastcall LoadV1(const AddressSpaceV1* hookSpace, MappedFile* file)
	{
		PatchCall(hookSpace->scroll_speed, hook_004E0323);
		scrollSpeed.multi = 14.0;
		scrollSpeed.offset.y = 14;
		scrollSpeed.offset.x = scrollSpeed.offset.y << 1;

		if (hookSpace->scroll_nop)
		{
			PatchBlock(hookSpace->scroll_nop, (VOID*)nop4, sizeof(nop4));
			PatchBlock(hookSpace->scroll_nop + 33, (VOID*)nop4, sizeof(nop4));
		}

		PatchCall(hookSpace->scroll_hook, hook_004DFC7A);

		// Random
		if (hookSpace->random_nop)
			PatchNop(hookSpace->random_nop, 5);

		// HD
		if (config.hd)
		{
			config.resHooked = TRUE;

			Size size;
			if (!config.isExist)
			{
				size.width = (DWORD)GetSystemMetrics(SM_CXSCREEN);
				size.height = (DWORD)GetSystemMetrics(SM_CYSCREEN);

				Config::CalcZoomed(&size, &size, config.zoom.factor);

				Config::Set(CONFIG_WRAPPER, "DisplayWidth", *(INT*)&size.width);
				Config::Set(CONFIG_WRAPPER, "DisplayHeight", *(INT*)&size.height);
			}
			else
			{
				size.width = (DWORD)Config::Get(CONFIG_WRAPPER, "DisplayWidth", *(INT*)&GAME_WIDTH);
				size.height = (DWORD)Config::Get(CONFIG_WRAPPER, "DisplayHeight", *(INT*)&GAME_HEIGHT);

				if (size.width < *(DWORD*)&GAME_WIDTH || size.height < *(DWORD*)&GAME_HEIGHT || size.width > 2048 || size.height > 1024)
				{
					size.width = GAME_WIDTH;
					size.height = GAME_HEIGHT;
				}
			}

			config.mode = modesList;

			*(Size*)config.mode = size;
			config.resolution.width = LOWORD(config.mode->width);
			config.resolution.height = LOWORD(config.mode->height);

			if (config.mode->width != GAME_WIDTH || config.mode->height != GAME_HEIGHT)
			{
				{
					PatchFunction(file, "CreateFileA", CreateFileHook);
					PatchFunction(file, "GetFileSize", GetFileSizeHook);
					PatchFunction(file, "ReadFile", ReadFileHook);
					PatchFunction(file, "CloseHandle", CloseHandleHook);
				}

				hud.pitch = config.mode->width + 179;
				if (hud.pitch & 3)
					hud.pitch = (hud.pitch & 0xFFFFFFFC) + 4;
				hud.buffer = (BYTE*)MemoryAlloc(hud.pitch * config.mode->height);

				PatchBlock(hookSpace->res_mode_1, config.mode, sizeof(DWORD) * 2);
				if (hookSpace->res_mode_2)
					PatchBlock(hookSpace->res_mode_2, config.mode, sizeof(DWORD) * 2);

				lineList = (DWORD*)MemoryAlloc(config.mode->height * sizeof(DWORD));
				PatchCall(hookSpace->res_linelist_hook, PopolateList);

				// CreateDialog hook
				RedirectCall(hookSpace->res_CreateDialog, hook_0055106A, &off_0055106A);

				// CreateDialog ISO hook
				if (hookSpace->res_CreateIsoDialog)
					RedirectCall(hookSpace->res_CreateIsoDialog, hook_0046541D, &off_0046541D);

				// Load Image V1
				PatchHook(hookSpace->res_LoadImage, hook_004F0B39);

				// StartDecodeImage
				PatchCall(hookSpace->res_StartDecodeImage, hook_004F12D6);

				// EndDecodeImage
				PatchHook(hookSpace->res_EndDecodeImage, hook_004F157A);
			}
		}
	}

	VOID __fastcall LoadV2(const AddressSpaceV2* hookSpace, MappedFile* file)
	{
		// Scroll
		{
			if (ReadRedirect(hookSpace->scroll_check, &cursorPos))
				PatchCall(hookSpace->scroll_check, hook_cursorPos);

			if (hookSpace->scroll_nop_1)
			{
				PatchBlock(hookSpace->scroll_nop_1, (VOID*)nop4, sizeof(nop4));
				PatchBlock(hookSpace->scroll_nop_1 + 33, (VOID*)nop4, sizeof(nop4));
			}

			if (hookSpace->scroll_nop_2)
			{
				PatchBlock(hookSpace->scroll_nop_2, (VOID*)nop4, sizeof(nop4));
				PatchBlock(hookSpace->scroll_nop_2 + 33, (VOID*)nop4, sizeof(nop4));
			}

			PatchCall(hookSpace->scroll_speed, hook_0053D185, 1);

			scrollSpeed.multi = 16.0;
			scrollSpeed.offset.y = 16;
			scrollSpeed.offset.x = scrollSpeed.offset.y << 1;

			PatchCall(hookSpace->scroll_hook, hook_0053CA08);

			PatchCall(hookSpace->dblclick_hook, hook_0053A73D);
		}

		// Cold CPU
		{
			// wait dialog
			if (ReadDWord(hookSpace->waitClass + 2, &waitAddress))
				PatchHook(hookSpace->waitHook, hook_004F1464);

			// wait cursor
			RedirectCall(hookSpace->waitShowCursor, hook_0053C095, &sub_0052EE9E);

			if (ReadDWord(hookSpace->waitLoadCursor + 1, &sub_006ACED2))
			{
				PatchHook(hookSpace->waitLoadCursor, hook_005AE161);
				sub_005AE161 = hookSpace->waitLoadCursor + 5 + baseOffset;
				sub_006ACED2 += sub_005AE161;
			}
		}

		// Messages real time
		PatchCall(hookSpace->msgTimeHook, hook_00484CD0);
		config.msgTimeScale.hooked = TRUE;

		// Print text
		if (hookSpace->print_text)
		{
			sub_PrintText = hookSpace->print_text + baseOffset;
			RedirectCall(hookSpace->print_init, hook_00405A1A, &sub_GameObjectInit);
			RedirectCall(hookSpace->print_deinit, hook_0048A93C, &sub_GameObjectDeInit);
		}

		// Png
		{
			pnglib_create_read_struct = (PNG_CREATE_READ_STRUCT)(hookSpace->png_create_read_struct + baseOffset);
			pnglib_create_info_struct = (PNG_CREATE_INFO_STRUCT)(hookSpace->png_create_info_struct + baseOffset);
			pnglib_set_read_fn = (PNG_SET_READ_FN)(hookSpace->png_set_read_fn + baseOffset);
			pnglib_destroy_read_struct = (PNG_DESTROY_READ_STRUCT)(hookSpace->png_destroy_read_struct + baseOffset);
			pnglib_read_info = (PNG_READ_INFO)(hookSpace->png_read_info + baseOffset);
			pnglib_read_image = (PNG_READ_IMAGE)(hookSpace->png_read_image + baseOffset);

			pnglib_create_write_struct = (PNG_CREATE_WRITE_STRUCT)(hookSpace->png_create_write_struct + baseOffset);
			pnglib_set_write_fn = (PNG_SET_WRITE_FN)(hookSpace->png_set_write_fn + baseOffset);
			pnglib_destroy_write_struct = (PNG_DESTROY_WRITE_STRUCT)(hookSpace->png_destroy_write_struct + baseOffset);
			pnglib_write_info = (PNG_WRITE_INFO)(hookSpace->png_write_info + baseOffset);
			pnglib_write_image = (PNG_WRITE_IMAGE)(hookSpace->png_write_image + baseOffset);
			pnglib_write_end = (PNG_WRITE_END)(hookSpace->png_write_end + baseOffset);
			pnglib_set_filter = (PNG_SET_FILTER)(hookSpace->png_set_filter + baseOffset);
			pnglib_set_IHDR = (PNG_SET_IHDR)(hookSpace->png_set_IHDR + baseOffset);
		}

		// Map bit-mask fix
		switch (hookSpace->mapSize)
		{
		case 96:
			PatchHook(hookSpace->bitmaskFix, hook_005BBF85);
			break;

		case 144:
			PatchHook(hookSpace->bitmaskFix, hook_005C848D);
			break;

		default:
			break;
		}

		// Random
		PatchNop(hookSpace->random_nop, 5);

		// AI list fix
		if (hookSpace->ai_list_hook_1)
		{
			RedirectCall(hookSpace->ai_list_hook_1, sub_45287F_Hook, &sub_45287F);
			PatchCall(hookSpace->ai_list_hook_2, sub_45287F_Hook);
			sub_GetType = hookSpace->ai_list_get_type + baseOffset;
		}

		// Skip draw
		RedirectCall(hookSpace->draw_hook_1, hook_00512BF0, &sub_00512BF0);
		PatchCall(hookSpace->draw_hook_2, hook_00512BF0);

		if (config.hd)
		{
			config.bpp32Hooked = TRUE;
			SeedRandom(timeGetTime());
			config.randPos.x = Random();
			config.randPos.y = Random();

			pBinkCopyToBuffer = PatchFunction(file, "_BinkCopyToBuffer@28", BinkCopyToBufferHook);
			Convert565toRGB = config.renderer == RendererGDI ? ConvertToBGR : ConvertToRGB;

			PatchBlock(hookSpace->pixel, (VOID*)pixelFunctions, sizeof(pixelFunctions));

			// =================================================================

			PatchHook(hookSpace->fillColor, hook_0055D283); // Fill color
			PatchCall(hookSpace->minimapGround, DrawMinimapGround); // Minimap ground
			PatchHook(hookSpace->minimapObjects, hook_0055D419); // Draw minimap object
			PatchDWord(hookSpace->clearGround, (DWORD)ClearGround); // Clear ground
			PatchHook(hookSpace->mapGround, hook_005B5660); // Draw map ground
			PatchHook(hookSpace->waterBorders, hook_005B5560); // Draw water borders

			PatchHook(hookSpace->symbol, hook_005280D9); // Draw Symbol
			PatchCall(hookSpace->faces, DrawFaces);
			PatchDWord(hookSpace->buildings, (DWORD)DrawCastleBuildings);

			PatchHook(hookSpace->horLine, hook_0053056A); // Draw Horizontal Line
			PatchHook(hookSpace->verLine, hook_00530603); // Draw Vertical Line

			// =================================================================

			PatchCall(hookSpace->line_1, DrawLine);
			PatchCall(hookSpace->line_2, DrawLine);

			PatchHook(hookSpace->unknown_1, hook_005383A5);
			back_005383AA = hookSpace->unknown_1 + 5 + baseOffset;

			PatchHook(hookSpace->unknown_2, hook_005383F7);
			back_00538413 = hookSpace->unknown_2 + 28 + baseOffset;

			// Increase memory
			{
				PatchByte(hookSpace->memory_1 + 2, 0xC);

				PatchCall(hookSpace->memory_2, hook_005A6311);

				PatchHook(hookSpace->memory_3, hook_005BACE1);
				back_005BACE7 = hookSpace->memory_3 + 6 + baseOffset;

				PatchCall(hookSpace->memory_4, hook_00674400);
				PatchCall(hookSpace->memory_5, hook_00674400);
				PatchCall(hookSpace->memory_6, hook_005A6209);
			}

			// Mirror battle bg
			if (hookSpace->btlLoadBack_1)
			{
				RedirectCall(hookSpace->btlLoadBack_1, hook_005AEF44, &sub_005A9AE9);
				PatchCall(hookSpace->btlLoadBack_2, hook_005AEF44);
				PatchCall(hookSpace->btlLoadBack_3, hook_005AEF44);

				PatchHook(hookSpace->btlBackHeight, hook_00529AEB);
			}

			if (hookSpace->res_hook)
			{
				config.resHooked = TRUE;

				Size size;
				if (!config.isExist)
				{
					size.width = (DWORD)GetSystemMetrics(SM_CXSCREEN);
					size.height = (DWORD)GetSystemMetrics(SM_CYSCREEN);

					Config::CalcZoomed(&size, &size, config.zoom.factor);

					Config::Set(CONFIG_WRAPPER, "DisplayWidth", *(INT*)&size.width);
					Config::Set(CONFIG_WRAPPER, "DisplayHeight", *(INT*)&size.height);
				}
				else
				{
					size.width = (DWORD)Config::Get(CONFIG_WRAPPER, "DisplayWidth", GAME_WIDTH);
					size.height = (DWORD)Config::Get(CONFIG_WRAPPER, "DisplayHeight", GAME_HEIGHT);

					if (size.width < *(DWORD*)&GAME_WIDTH || size.height < *(DWORD*)&GAME_HEIGHT)
					{
						size.width = *(DWORD*)&GAME_WIDTH;
						size.height = *(DWORD*)&GAME_HEIGHT;
					}
				}

				config.mode = &modesList[1];

				*(Size*)config.mode = size;
				config.resolution.width = LOWORD(config.mode->width);
				config.resolution.height = LOWORD(config.mode->height);

				Config::CalcZoomed();

				// Resolution
				{
					PatchHook(hookSpace->res_hook, hook_00611C8B);
					back_00611CEF = hookSpace->res_back + baseOffset;
				}

				// Remove window size restriction
				{
					PatchByte(hookSpace->res_restriction_jmp, 0xEB);
					PatchNop(hookSpace->res_restriction_nop, 2);
					PatchNop(hookSpace->res_restriction_nop + 8, 2);
				}

				// Borders
				{
					if (hookSpace->border_nop)
					{
						PatchNop(hookSpace->border_nop, 2);
						PatchByte(hookSpace->border_nop + 0x16, 0xEB); // remove internal borders

						if (config.mode->width > *(DWORD*)&GAME_WIDTH || config.mode->height > *(DWORD*)&GAME_HEIGHT)
						{
							config.border.inside = TRUE;
							config.border.allowed = TRUE;
							config.zoom.allowed = TRUE;
						}
					}
					else
						config.border.allowed = TRUE;

					if (!config.border.allowed)
						config.border.enabled = FALSE;

					if (!config.zoom.allowed)
						config.zoom.enabled = FALSE;

					if (!hookSpace->border_nop || config.mode->width > *(DWORD*)&GAME_WIDTH || config.mode->height > *(DWORD*)&GAME_HEIGHT)
						PatchCall(hookSpace->border_hook - 3, hook_00538FEB); // detect border
				}

				// Blit count
				{
					BYTE value;
					ReadByte(hookSpace->blit_size + 2, &value);

					DWORD count = DWORD((2048.0f * config.mode->width * config.mode->height) / (GAME_WIDTH_FLOAT * GAME_HEIGHT_FLOAT));
					PatchDWord(hookSpace->blit_patch_1 + 1, count * value);
					PatchDWord(hookSpace->blit_patch_1 + 0xC + 3, count);

					PatchDWord(hookSpace->blit_patch_2 + 1, count * 20);
					PatchDWord(hookSpace->blit_patch_2 + 0x17 + 6, count);
				}

				// Minimap rectangle
				{
					PatchByte(hookSpace->mini_rect_jmp, 0xEB);
					PatchByte(hookSpace->mini_rect_jmp + 23, 0xEB);

					FLOAT w = FLOAT(27 - 7) / (GAME_WIDTH - 160) * (config.mode->width - 160);
					FLOAT h = FLOAT(27 + 7) / GAME_HEIGHT * config.mode->height;

					FLOAT ky = (h - w) * 0.5f;

					INT x = (INT)MathRound(w + ky);
					INT y = (INT)MathRound(ky);

					PatchDWord(hookSpace->mini_rect_patch + 3, *(DWORD*)&x);
					PatchDWord(hookSpace->mini_rect_patch + 8 + 3, *(DWORD*)&y);
				}

				// Fix right side curve
				if (hookSpace->right_curve)
				{
					PatchHook(hookSpace->right_curve, hook_00489124);
					back_00489136 = hookSpace->right_curve + 18 + baseOffset;
				}

				// Fix minimap fill color alpha
				if (hookSpace->minimap_fill)
					PatchNop(hookSpace->minimap_fill, 6);

				if (config.mode->width > 1152)
				{
					PatchDWord(hookSpace->maxSize_1 + 2, (DWORD)&config.mode->width);
					PatchDWord(hookSpace->maxSize_2 + 2, (DWORD)&config.mode->width);
					PatchDWord(hookSpace->maxSize_3 + 6 + 2, (DWORD)&config.mode->width);
				}

				if (config.mode->height > 1152)
				{
					PatchDWord(hookSpace->maxSize_1 + 29 + 2, (DWORD)&config.mode->height);
					PatchDWord(hookSpace->maxSize_2 + 28 + 2, (DWORD)&config.mode->height);
					PatchDWord(hookSpace->maxSize_3 + 2, (DWORD)&config.mode->height);
				}

				// Widescreen Battle
				if (hookSpace->btlClass && config.mode->width >= WIDE_WIDTH && ReadDWord(hookSpace->btlClass + 2, &battleAddress))
				{
					config.wide.hooked = TRUE;

					if (!config.isExist)
					{
						config.wide.allowed = TRUE;
						Config::Set(CONFIG_WRAPPER, "WideBattle", config.wide.allowed);
					}

					PatchNop(hookSpace->btlCentrBack, 2);
					PatchCall(hookSpace->btlCentrBack + 27, hook_0063E6B8); // centre battle background

					PatchHook(hookSpace->btlCentrUnits, hook_00625683); // centre battle units
					back_00625688 = hookSpace->btlCentrUnits + 5 + baseOffset;

					// remove mouse check left group
					RedirectCall(hookSpace->btlMouseCheck, hook_0062891A, &sub_00629FAA);

					// swap groups
					PatchHook(hookSpace->btlSwapGroup, hook_0062F524);
					back_0062F52A = hookSpace->btlSwapGroup + 6 + baseOffset;

					// remove set inactive group
					RedirectCall(hookSpace->btlGroupsInactive, hook_0062F5B8, &sub_00643E80);

					// groups init parameters
					if (hookSpace->version == 2)
					{
						PatchCall(hookSpace->btlGroupsActive, hook_0062521D); // set groups active

						// Reverse groups back
						PatchCall(hookSpace->btlReverseGroup, hook_00624F2F, 1);
						PatchCall(hookSpace->btlInitGroups1_1, hook_00625EAE, 3);
						PatchCall(hookSpace->btlInitGroups1_2, hook_00625EFD, 1);
						PatchCall(hookSpace->btlInitGroups1_3, hook_00625EFD, 1);
						PatchCall(hookSpace->btlInitGroups2_1, hook_0062635D, 3);
						PatchCall(hookSpace->btlInitGroups2_2, hook_006263AC, 1);
						PatchCall(hookSpace->btlInitGroups2_3, hook_006263AC, 1);
					}
					else if (hookSpace->version == 3)
					{
						PatchCall(hookSpace->btlGroupsActive, hook_0062DAA8); // set groups active

						// Reverse groups back
						PatchCall(hookSpace->btlReverseGroup, hook_0062ED48, 1);
						//PatchCall(hookSpace->btlInitGroups1_1, hook_0062E7FB, 1);
						PatchCall(hookSpace->btlInitGroups1_2, hook_0062E848, 1);
						PatchCall(hookSpace->btlInitGroups1_3, hook_0062E848, 1);
						//PatchCall(hookSpace->btlInitGroups2_1, hook_0062ECA9, 1);
						PatchCall(hookSpace->btlInitGroups2_2, hook_0062ECFC, 1);
						PatchCall(hookSpace->btlInitGroups2_3, hook_0062ECFC, 1);
					}

					// change image indices
					RedirectCall(hookSpace->btlImgIndices, hook_00625387, &sub_00625387);

					// set dialog
					PatchCall(hookSpace->btlDialog_1, hook_006244CA, 1);
					PatchCall(hookSpace->btlDialog_2, hook_0062476C, 1);

					if (hookSpace->btlFileGetStr)
						RedirectCall(hookSpace->btlFileGetStr, SteamFileGetStrHook, &sub_fgets);
					else
						PatchFunction(file, "fgets", FileGetStrHook);
				}

				// Debug & message position
				{
					// Calc debug window
					PatchCall(hookSpace->debugPosition, hook_0052EAAE, 1);

					// Calc messages
					PatchCall(hookSpace->msgIconPosition, hook_00484DD1);
					PatchCall(hookSpace->msgTextPosition, hook_00484DD1);
				}
			}
		}
	}

	VOID Load()
	{
		hModule = GetModuleHandle(NULL);
		PIMAGE_NT_HEADERS headNT = (PIMAGE_NT_HEADERS)((BYTE*)hModule + ((PIMAGE_DOS_HEADER)hModule)->e_lfanew);
		baseOffset = (INT)hModule - (INT)headNT->OptionalHeader.ImageBase;

		PatchEntryPoint("DDRAW.dll", FakeEntryPoint);

		MappedFile* file = new MappedFile(hModule);
		{
			{
				PatchFunction(file, "GetDeviceCaps", GetDeviceCapsHook);
				PatchFunction(file, "GetForegroundWindow", GetForegroundWindowHook);

				PatchFunction(file, "CreateWindowExA", CreateWindowExHook);
				PatchFunction(file, "RegisterClassA", RegisterClassHook);

				PatchFunction(file, "SetWindowLongA", SetWindowLongHook);

				PatchFunction(file, "MessageBoxA", MessageBoxHook);

				if (!PatchFunction(file, "RegisterWindowMessageA", RegisterWindowMessageHook))
					PatchFunction(file, "RegisterClipboardFormatA", RegisterWindowMessageHook); // for cracks

				PatchFunction(file, "SetThreadPriority", SetThreadPriorityHook);

				PatchFunction(file, "ShowCursor", ShowCursorHook);
				PatchFunction(file, "ClipCursor", ClipCursorHook);

				PatchFunction(file, "GetClientRect", GetClientRectHook);
				PatchFunction(file, "GetWindowRect", GetWindowRectHook);

				PatchFunction(file, "timeGetTime", timeGetTimeHook);

				PatchFunction(file, "GetOpenFileNameA", GetOpenFileNameHook);
				PatchFunction(file, "GetSaveFileNameA", GetSaveFileNameHook);

				PatchFunction(file, "isalpha", IsAlphaHook);
				PatchFunction(file, "isalnum", IsAlNumHook);
				PatchFunction(file, "isdigit", IsDigitHook);
				PatchFunction(file, "isspace", IsSpaceHook);
				PatchFunction(file, "ispunct", IsPunctHook);
				PatchFunction(file, "iscntrl", IsCntrlHook);
				PatchFunction(file, "isupper", IsUpperHook);
				PatchFunction(file, "toupper", ToUpperHook);
				PatchFunction(file, "tolower", ToLowerHook);
				PatchFunction(file, "strchr", StrCharHook);
				PatchFunction(file, "memchr", MemoryCharHook);

				if (config.locales.current.oem && config.locales.current.ansi)
				{
					PatchFunction(file, "OemToCharA", OemToCharHook);
					PatchFunction(file, "CharToOemA", CharToOemHook);
				}

				PatchFunction(file, "CoCreateInstance", CoCreateInstanceHook);

				if (config.version)
				{
					PatchFunction(file, "PeekMessageA", PeekMessageHook);
					PatchFunction(file, "GetCursorPos", GetCursorPosHookV1);
					PatchFunction(file, "ClientToScreen", ClientToScreenHook);
					PatchFunction(file, "GetDoubleClickTime", GetDoubleClickTimeHook);
				}
				else
				{
					PatchFunction(file, "DirectDrawEnumerateExA", Main::DrawEnumerateEx);
					PatchFunction(file, "DirectDrawCreate", Main::DrawCreate);
					PatchFunction(file, "DirectDrawCreateEx", Main::DrawCreateEx);
					PatchFunction(file, "GetCursorPos", GetCursorPosHookV2);
					PatchFunction(file, "SetCursorPos", SetCursorPosHook);
				}
			}

			if (config.version)
			{
				const AddressSpaceV1* hookSpace = addressArrayV1;
				DWORD hookCount = sizeof(addressArrayV1) / sizeof(AddressSpaceV1);
				do
				{
					DWORD check;
					if (ReadDWord(hookSpace->check + 1, &check) && check == hookSpace->value)
					{
						LoadV1(hookSpace, file);
						break;
					}

					++hookSpace;
				} while (--hookCount);
			}
			else
			{
				const AddressSpaceV2* defaultSpace = NULL;
				const AddressSpaceV2* equalSpace = NULL;

				const AddressSpaceV2* hookSpace = addressArrayV2;
				DWORD hookCount = sizeof(addressArrayV2) / sizeof(AddressSpaceV2);
				do
				{
					DWORD check;
					if (ReadDWord(hookSpace->check_1, &check) && check == hookSpace->value_1)
					{
						if (!hookSpace->check_2)
							defaultSpace = hookSpace;
						else if (ReadDWord(hookSpace->check_2, &check) && check == hookSpace->value_2)
						{
							LoadV2(hookSpace, file);
							defaultSpace = NULL;
							break;
						}
					}

					++hookSpace;
				} while (--hookCount);

				if (defaultSpace)
					LoadV2(defaultSpace, file);
			}
		}
		delete file;
	}
#pragma optimize("", on)
}