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
#include "Hooks.h"
#include "Main.h"
#include "Config.h"
#include "Resource.h"
#include "Window.h"
#include "PngLib.h"

#define CHECKVALUE (WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX)

namespace Hooks
{
	HMODULE hModule;
	INT baseOffset;

	const BYTE nop4[] = { 0x83, 0xC4, 0x04, 0x90, 0x90 };

	const AddressSpaceV1 addressArrayV1[] = {
		// v99.12.15.1
		0x004CD137, WS_POPUP, 0x004DFFA4, 0x00000000, 0x004E06D3,

		// v2000.6.22.1 - Eng
		0x004CD13E, WS_POPUP, 0x004DFF9C, 0x00000000, 0x004E0759,

		// v2000.6.22.1 - Rus1
		0x004CD408, WS_POPUP, 0x004E0323, 0x00000000, 0x004E0AA8,

		// v2000.6.22.1 Editor
		0x00443EDE, WS_POPUP, 0x0045147D, 0x004265E3, 0x00451BE8
	};

	const AddressSpaceV2 addressArrayV2[] = {
#pragma region v1 .10
		// v1.10
		0x00558687, CHECKVALUE, 0x00000000, 0x00000000,

		0x005ADFCC, 0x0059A30B, 0x005AECAF, 0x006622BE, 0x0066242E, 0x0059A203,
		0x006D3728, 0x00552876, 0x0066378C, 0x00552A0C, 0x006CD194, 0x005A98B0,
		0x005A97B0, 0x0051E67B, 0x005A10A1, 0x006CCD00, 0x005261BF, 0x00526258,
		0x0052DBEE, 0x0052DC2D, 0x0052DC36, 0x0052DC88,

		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000,

		0x00532618, 0x00532D00, 0x00000000, 0x00000000, 0x00532DA2, 0x0052FB85,
#pragma endregion

#pragma region v1 .40
		// v1.40
		0x0055EA82, CHECKVALUE, 0x00000000, 0x00000000,

		0x005B4516, 0x005A0826, 0x005B5216, 0x0066BEBB, 0x0066C014, 0x005A071E,
		0x006DFD98, 0x00558982, 0x0066D37F, 0x00558B18, 0x006D950C, 0x005AFD70,
		0x005AFC70, 0x00523979, 0x005A740F, 0x006D9078, 0x0052BD52, 0x0052BDEB,
		0x005339AF, 0x005339EE, 0x005339F7, 0x00533A49,

		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000,

		0x00538701, 0x00538DE6, 0x00000000, 0x00000000, 0x00538E88, 0x00535C7B,
#pragma endregion

#pragma region v2 .01
		// v2.01
		0x005645FD, CHECKVALUE, 0x00000000, 0x00000000,

		0x005BB2A9, 0x005A7434, 0x005BC059, 0x006758C0, 0x00675A27, 0x005A732C,
		0x006EA4E8, 0x0055E55E, 0x00676D96, 0x0055E6F4, 0x006E398C, 0x005B69F0,
		0x005B68F0, 0x00529178, 0x005AE075, 0x006E34C8, 0x005317C9, 0x00531862,
		0x00539521, 0x00539560, 0x00539569, 0x005395BB,

		0x0061331D, 0x00613381, 0x00614CC5, 0x005646BF, 0x0040263B, 0x0053A1BD,
		0x00515A22, 0x00519FF0, 0x00515112, 0x005C829E, 0x005C825F, 0x00488C30,
		0x005A7539, 0x005A7583, 0x0066FB64, 0x0066FCBD, 0x0066F974, 0x00515E2F,
		0x00515F3E, 0x00669434, 0x00546870, 0x0054789E, 0x005476F9, 0x00547344,
		0x00546A5B, 0x005472F7,

		0x0053E350, 0x0053EAC6, 0x00000000, 0x00000000, 0x0053EB68, 0x0053B927,
#pragma endregion

#pragma region v2 .01 - Steam
		// v2.01 - Steam
		0x00564D69, CHECKVALUE, 0x00000000, 0x00000000,

		0x005BB7DE, 0x005A79EA, 0x005BC568, 0x00663E55, 0x00663FBC, 0x005A78E2,
		0x006EA690, 0x0055EC4C, 0x006652F8, 0x0055EDE2, 0x006E39DC, 0x005B6F70,
		0x005B6E70, 0x0052983E, 0x005AE623, 0x006E3518, 0x00531EE5, 0x00531F7E,
		0x00539C09, 0x00539C48, 0x00539C51, 0x00539CA3,

		0x00613712, 0x00613776, 0x00614FAB, 0x00564E2B, 0x00402253, 0x0053A89E,
		0x00516222, 0x0051A760, 0x00515912, 0x005C86BF, 0x005C8680, 0x00488CC7,
		0x005A7AEF, 0x005A7B39, 0x00671EF4, 0x0067204D, 0x00671D04, 0x0051662F,
		0x0051673E, 0x0066B804, 0x00546F70, 0x00547F9E, 0x00547DF9, 0x00547A44,
		0x0054715B, 0x005479F7,

		0x0053EA15, 0x0053F162, 0x00000000, 0x00000000, 0x0053F204, 0x0053C02B,
#pragma endregion

#pragma region v2 .02
		// v2.02
		0x0056339E, CHECKVALUE, 0x00000000, 0x00000000,

		0x005B9F86, 0x005A6311, 0x005BACE1, 0x0067427E, 0x00674400, 0x005A6209,
		0x006E9408, 0x0055D283, 0x00675771, 0x0055D419, 0x006E2894, 0x005B5660,
		0x005B5560, 0x005280D9, 0x005ACF11, 0x006E23D0, 0x0053056A, 0x00530603,
		0x0053835D, 0x0053839C, 0x005383A5, 0x005383F7,

		0x00611C8B, 0x00611CEF, 0x00613523, 0x00563460, 0x00402129, 0x00538FEB,
		0x00514DD2, 0x005193F0, 0x005144C2, 0x005C6F52, 0x005C6F13, 0x00489124,
		0x005A6416, 0x005A6460, 0x0066E3D4, 0x0066E52D, 0x0066E1E4, 0x005151DF,
		0x005152EE, 0x00667DC4, 0x00545670, 0x0054669E, 0x005464F9, 0x00546144,
		0x0054585B, 0x005460F7,

		0x0053D185, 0x0053D8A9, 0x00000000, 0x00000000, 0x0053D94B, 0x0053A73D,
#pragma endregion

#pragma region v2 .02 - Crack
		// v2.02 - Crack
		0x00564C4D, CHECKVALUE, 0x00000000, 0x00000000,

		0x005BB74F, 0x005A7BFA, 0x005BC465, 0x006758E0, 0x00675A47, 0x005A7AF2,
		0x006EA4F0, 0x0055EBAE, 0x00676DB6, 0x0055ED44, 0x006E398C, 0x005B6F90,
		0x005B6E90, 0x00529698, 0x005AE83B, 0x006E34C8, 0x00531C99, 0x00531D32,
		0x005399F1, 0x00539A30, 0x00539A39, 0x00539A8B,

		0x00613530, 0x00613594, 0x00614DD2, 0x00564D0F, 0x0040254B, 0x0053A68D,
		0x00515F52, 0x0051A520, 0x00515642, 0x005C8610, 0x005C85D1, 0x004892E5,
		0x005A7CFF, 0x005A7D49, 0x0066FB84, 0x0066FCDD, 0x0066F994, 0x0051635F,
		0x0051646E, 0x00669454, 0x00546EC0, 0x00547EEE, 0x00547D49, 0x00547994,
		0x005470AB, 0x00547947,

		0x0053E820, 0x0053EF96, 0x00000000, 0x00000000, 0x0053F038, 0x0053BDF7,
#pragma endregion

#pragma region v3 .00 - Steam
		// v3.00 - Steam
		0x005674DF, CHECKVALUE, 0x00000000, 0x00000000,

		0x005BF272, 0x005AB0A4, 0x005C0048, 0x0067E78C, 0x0067E8D5, 0x005AAF9C,
		0x006F4E40, 0x005614BE, 0x0067FC5F, 0x00561654, 0x006EDE44, 0x005BA970,
		0x005BA870, 0x0052CB61, 0x005B1CE2, 0x006ED978, 0x00534FFD, 0x00535096,
		0x0053CD9E, 0x0053CDDD, 0x0053CDE6, 0x0053CE38,

		0x00619B3D, 0x00619BA2, 0x0061B492, 0x005675A1, 0x004021CE, 0x0053DA29,
		0x00519372, 0x0051D940, 0x00518A62, 0x005CC404, 0x005CC3C5, 0x0048BF6E,
		0x005AB1A9, 0x005AB1F3, 0x006787F4, 0x0067894D, 0x00678604, 0x0051977F,
		0x0051988E, 0x006721C4, 0x00549EE0, 0x0054AF0E, 0x0054AD69, 0x0054A9B4,
		0x0054A0CB, 0x0054A967,

		0x00541BA5, 0x005422C9, 0x00000000, 0x00000000, 0x0054236B, 0x0053F1A4,
#pragma endregion

#pragma region v3 .01a
		// v3.01a and Crack
		0x005676DB, CHECKVALUE, 0x00000000, 0x00000000,

		0x005BF7E4, 0x005AB6D8, 0x005C0654, 0x0067F2BE, 0x0067F42A, 0x005AB5D0,
		0x006F5E50, 0x005616BE, 0x006807FE, 0x00561854, 0x006EEE74, 0x005BAFC0,
		0x005BAEC0, 0x0052CAA6, 0x005B22A1, 0x006EE9A8, 0x00535048, 0x005350E1,
		0x0053CE21, 0x0053CE60, 0x0053CE69, 0x0053CEBB,

		0x0061A6C5, 0x0061A72A, 0x0061C023, 0x0056779D, 0x00402444, 0x0053DAAC,
		0x005191B2, 0x0051D6E0, 0x005188A2, 0x005CCA34, 0x005CC9F5, 0x0048BF35,
		0x005AB7DD, 0x005AB827, 0x00679534, 0x0067968D, 0x00679344, 0x005195BF,
		0x005196CE, 0x00672E84, 0x0054A0E0, 0x0054B10E, 0x0054AF69, 0x0054ABB4,
		0x0054A2CB, 0x0054AB67,

		0x00541C2D, 0x0054237C, 0x00000000, 0x00000000, 0x0054241E, 0x0053F237,
#pragma endregion

#pragma region v3 .01b
		// v3.01b
		0x00566E05, CHECKVALUE, 0x00000000, 0x00000000,

		0x005BE82F, 0x005AA960, 0x005BF589, 0x0067DC5A, 0x0067DDDC, 0x005AA858,
		0x006F3E00, 0x00560E5B, 0x0067F172, 0x00560FF1, 0x006ECE14, 0x005B9F70,
		0x005B9E70, 0x0052C03C, 0x005B15A2, 0x006EC948, 0x00534626, 0x005346BF,
		0x0053C4CD, 0x0053C50C, 0x0053C515, 0x0053C567,

		0x006191E3, 0x00619248, 0x0061AB38, 0x00566EC7, 0x0040218A, 0x0053D158,
		0x00518742, 0x0051CCA0, 0x00517E32, 0x005CB950, 0x005CB911, 0x0048BB1E,
		0x005AAA65, 0x005AAAAF, 0x00677D94, 0x00677EED, 0x00677BA4, 0x00518B4F,
		0x00518C5E, 0x00671694, 0x005497F0, 0x0054A81E, 0x0054A679, 0x0054A2C4,
		0x005499DB, 0x0054A277,

		0x005412DD, 0x00541A01, 0x00000000, 0x00000000, 0x00541AA3, 0x0053E89F,
#pragma endregion

#pragma region v1 .10 Editor
		// v1.10 Editor
		0x00472638, CHECKVALUE, 0x00000000, 0x00000480,

		0x0053C64D, 0x0052CE6F, 0x0053D324, 0x0056FC19, 0x0056FD80, 0x0052CD67,
		0x005B8000, 0x0055E8C3, 0x0057114B, 0x0055EA59, 0x005B56C4, 0x00536B10,
		0x00536A10, 0x004A6BE2, 0x005348C6, 0x005B52E0, 0x0047532E, 0x004753C7,
		0x0055FF5C, 0x0055FF9B, 0x0055FFA4, 0x0055FFF6,

		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000,

		0x004813D2, 0x00481AE4, 0x00446339, 0x0045FBD4, 0x00481B86, 0x0047E658,
#pragma endregion

#pragma region v1 .14 Editor
		// v1.14 Editor
		0x004728EF, CHECKVALUE, 0x00000000, 0x00000480,

		0x0053FAC7, 0x0052FBE8, 0x005407DC, 0x005736A4, 0x005737F5, 0x0052FAE0,
		0x005BCDE0, 0x00561ADF, 0x00574BB4, 0x00561C75, 0x005BA60C, 0x00539EE0,
		0x00539DE0, 0x004A82A9, 0x005378A2, 0x005BA070, 0x00475664, 0x004756FD,
		0x00563339, 0x00563378, 0x00563381, 0x005633D3,

		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000,

		0x004823D1, 0x00482AC0, 0x00446667, 0x004601FA, 0x00482B62, 0x0047F6A4,
#pragma endregion

#pragma region v2 .00 Editor - Steam
		// v2.00 Editor - Steam
		0x00489D25, CHECKVALUE, 0x00000000, 0x00000000,

		0x0055C4B6, 0x0054C79A, 0x0055D13A, 0x00591E06, 0x00591F6A, 0x0054C68E,
		0x005DEDE0, 0x0057F12A, 0x005932E9, 0x0057F2C0, 0x005DC574, 0x005568D0,
		0x005567D0, 0x004BFB0E, 0x0055429D, 0x005DBFA8, 0x0048CA22, 0x0048CABB,
		0x0058097B, 0x005809BA, 0x005809C3, 0x00580A15,

		0x0051B64F, 0x0051B6B3, 0x005190D5, 0x00489DE7, 0x00000000, 0x0049435D,
		0x004AF3B2, 0x004B38E0, 0x004AEAA2, 0x004D2597, 0x004D2558, 0x00000000,
		0x0054C89F, 0x0054C8E9, 0x0058B544, 0x0058B69D, 0x0058B34E, 0x004AF7BF,
		0x004AF8CE, 0x00584EE4, 0x0056D0D0, 0x0056E0FE, 0x0056DF59, 0x0056DBA4,
		0x0056D2BB, 0x0056DB57,

		0x00499B8E, 0x0049A2D0, 0x00448D1C, 0x0046320E, 0x0049A372, 0x00496D92,
#pragma endregion

#pragma region v2 .01 Editor
		// v2.01 Editor
		0x004893BD, CHECKVALUE, 0x00000000, 0x00000000,

		0x0055B9F7, 0x0054C08C, 0x0055C66B, 0x00591270, 0x005913E0, 0x0054BF84,
		0x005DDD70, 0x0057E23F, 0x005927BD, 0x0057E3D5, 0x005DB524, 0x00555F10,
		0x00555E10, 0x004BF0E2, 0x005538E4, 0x005DAF58, 0x0048C109, 0x0048C1A2,
		0x0057FA99, 0x0057FAD8, 0x0057FAE1, 0x0057FB33,

		0x0051ADF0, 0x0051AE54, 0x005188BC, 0x0048947F, 0x00000000, 0x00493A1D,
		0x004AEAC2, 0x004B2FE0, 0x004AE1B2, 0x004D1BF1, 0x004D1BB2, 0x00000000,
		0x0054C191, 0x0054C1DB, 0x0058A914, 0x0058AA6D, 0x0058A71E, 0x004AEECF,
		0x004AEFDE, 0x005842E4, 0x0056C1D0, 0x0056D1FE, 0x0056D059, 0x0056CCA4,
		0x0056C3BB, 0x0056CC57,

		0x0049925F, 0x00499A08, 0x00448D8C, 0x0046346A, 0x00499AAA, 0x00496412,
#pragma endregion

#pragma region v2 .02 Editor
		// v2.02 Editor
		0x004893BD, CHECKVALUE, 0x0054E7D3, 0x00000480,

		0x0055BB17, 0x0054C1AC, 0x0055C78B, 0x00591379, 0x005914E9, 0x0054C0A4,
		0x005DDD78, 0x0057E35F, 0x005928C6, 0x0057E4F5, 0x005DB52C, 0x00556030,
		0x00555F30, 0x004BF0E2, 0x00553A04, 0x005DAF60, 0x0048C109, 0x0048C1A2,
		0x0057FBB9, 0x0057FBF8, 0x0057FC01, 0x0057FC53,

		0x0051AE07, 0x0051AE6B, 0x005188BC, 0x0048947F, 0x00000000, 0x00493A1D,
		0x004AEAC2, 0x004B2FE0, 0x004AE1B2, 0x004D1BF1, 0x004D1BB2, 0x00000000,
		0x0054C2B1, 0x0054C2FB, 0x0058AA34, 0x0058AB8D, 0x0058A83E, 0x004AEECF,
		0x004AEFDE, 0x00584404, 0x0056C2F0, 0x0056D31E, 0x0056D179, 0x0056CDC4,
		0x0056C4DB, 0x0056CD77,

		0x0049925F, 0x00499A08, 0x00448D8C, 0x0046346A, 0x00499AAA, 0x00496412,
#pragma endregion

#pragma region v3 .00 Editor - Steam
		// v3.00 Editor - Steam
		0x0048C842, CHECKVALUE, 0x00000000, 0x00000000,

		0x0056234B, 0x005527FA, 0x00562FDC, 0x00597EDA, 0x00598050, 0x005526EE,
		0x005E6700, 0x00585176, 0x005993B6, 0x0058530C, 0x005E3E24, 0x0055C7D0,
		0x0055C6D0, 0x004C264A, 0x0055A1A3, 0x005E3850, 0x0048F5B3, 0x0048F64C,
		0x005869CA, 0x00586A09, 0x00586A12, 0x00586A64,

		0x0052019C, 0x00520201, 0x0051DBBB, 0x0048C904, 0x00000000, 0x00496F6F,
		0x004B1F82, 0x004B64F0, 0x004B1672, 0x004D550E, 0x004D54CF, 0x00000000,
		0x005528FF, 0x00552949, 0x005915F4, 0x0059174D, 0x005913FE, 0x004B238F,
		0x004B249E, 0x0058AF54, 0x00573120, 0x0057414E, 0x00573FA9, 0x00573BF4,
		0x0057330B, 0x00573BA7,

		0x0049C83A, 0x0049CF68, 0x0044AB98, 0x0046555C, 0x0049D00A, 0x004999FB,
#pragma endregion

#pragma region v3 .01 Editor
		// v3.01 Editor
		0x0048BA1B, CHECKVALUE, 0x00000000, 0x00000000,

		0x00561820, 0x00551CA9, 0x005624CD, 0x00597125, 0x0059728C, 0x00551BA1,
		0x005E5660, 0x0058422F, 0x005986BD, 0x005843C5, 0x005E2D9C, 0x0055BCF0,
		0x0055BBF0, 0x004C1729, 0x0055969C, 0x005E27C8, 0x0048E6DF, 0x0048E778,
		0x00585A83, 0x00585AC2, 0x00585ACB, 0x00585B1D,

		0x0051F560, 0x0051F5C5, 0x0051CF95, 0x0048BADD, 0x00000000, 0x0049604C,
		0x004B1002, 0x004B5560, 0x004B06F2, 0x004D4768, 0x004D4729, 0x00000000,
		0x00551DAE, 0x00551DF8, 0x00590904, 0x00590A5D, 0x0059070E, 0x004B140F,
		0x004B151E, 0x0058A1B4, 0x005721C0, 0x005731EE, 0x00573049, 0x00572C94,
		0x005723AB, 0x00572C47,

		0x0049B857, 0x0049BFFF, 0x0044A8E6, 0x00465353, 0x0049C0A1, 0x00498A27
#pragma endregion
	};

#pragma region Hook helpers
#pragma optimize("s", on)
	BOOL __fastcall PatchRedirect(DWORD addr, VOID* hook, BYTE instruction)
	{
		DWORD address = addr + baseOffset;

		DWORD old_prot;
		if (VirtualProtect((VOID*)address, 5, PAGE_EXECUTE_READWRITE, &old_prot))
		{
			BYTE* jump = (BYTE*)address;
			*jump = instruction;
			++jump;
			*(DWORD*)jump = (DWORD)hook - (DWORD)address - 5;

			VirtualProtect((VOID*)address, 5, old_prot, &old_prot);

			return TRUE;
		}
		return FALSE;
	}

	BOOL __fastcall PatchHook(DWORD addr, VOID* hook)
	{
		return PatchRedirect(addr, hook, 0xE9);
	}

	BOOL __fastcall PatchCall(DWORD addr, VOID* hook)
	{
		return PatchRedirect(addr, hook, 0xE8);
	}

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

	DWORD __fastcall PatchFunction(MappedFile* file,
		const CHAR* function,
		VOID* addr)
	{
		DWORD res = NULL;

		DWORD base = (DWORD)file->hModule;
		PIMAGE_NT_HEADERS headNT = (PIMAGE_NT_HEADERS)(
			(BYTE*)base + ((PIMAGE_DOS_HEADER)file->hModule)->e_lfanew);

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
					if (!file->hFile)
					{
						CHAR filePath[MAX_PATH];
						GetModuleFileName(file->hModule, filePath, MAX_PATH);
						file->hFile = CreateFile(filePath, GENERIC_READ, 0, NULL,
							OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
						if (!file->hFile)
							return res;
					}

					if (!file->hMap)
					{
						file->hMap = CreateFileMapping(file->hFile, NULL, PAGE_READONLY, 0, 0, NULL);
						if (!file->hMap)
							return res;
					}

					if (!file->address)
					{
						file->address = MapViewOfFile(file->hMap, FILE_MAP_READ, 0, 0, 0);
						;
						if (!file->address)
							return res;
					}

					headNT = (PIMAGE_NT_HEADERS)((BYTE*)file->address + ((PIMAGE_DOS_HEADER)file->address)->e_lfanew);
					PIMAGE_SECTION_HEADER sh = (PIMAGE_SECTION_HEADER)((DWORD)&headNT->OptionalHeader + headNT->FileHeader.SizeOfOptionalHeader);

					nameThunk = NULL;
					DWORD sCount = headNT->FileHeader.NumberOfSections;
					while (sCount--)
					{
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
						if (ReadDWord((INT)&addressThunk->u1.AddressOfData - baseOffset,
								&res))
							PatchDWord((INT)&addressThunk->u1.AddressOfData - baseOffset,
								(DWORD)addr);

						return res;
					}
				}
			}
		}

		return res;
	}
#pragma optimize("", on)
#pragma endregion

	// ===============================================================
	HWND hWndMain;

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
		INT res;
		ULONG_PTR cookie = NULL;
		if (hActCtx && hActCtx != INVALID_HANDLE_VALUE && !ActivateActCtxC(hActCtx, &cookie))
			cookie = NULL;

		res = MessageBox(hWnd, lpText, lpCaption, uType);

		if (cookie)
			DeactivateActCtxC(0, cookie);

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
		if (GetCursorPos(lpPoint))
		{
			RECT rect;
			if (GetClientRect(hWndMain, &rect) && ClientToScreen(hWndMain, (LPPOINT)&rect))
			{
				if (!config.windowedMode && config.borderlessMode)
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

	BOOL __stdcall GetCursorPosHookV2(LPPOINT lpPoint)
	{
		if (GetCursorPos(lpPoint))
		{
			RECT rect;
			if (GetClientRect(hWndMain, &rect) && ClientToScreen(hWndMain, (LPPOINT)&rect))
			{
				if (!config.windowedMode && config.borderlessMode)
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

				if (!config.zoomImage || !config.isBorder)
				{
					lpPoint->x = LONG(fx * (FLOAT(lpPoint->x - rect.left) - offset.x)) + rect.left;
					lpPoint->y = LONG(fy * (FLOAT(lpPoint->y - rect.top) - offset.y)) + rect.top;
				}
				else
				{
					FLOAT x, y;
					FLOAT k = (FLOAT)config.mode->width / config.mode->height;
					if (k >= 4.0f / 3.0f)
					{
						y = GAME_HEIGHT_FLOAT;
						x = k * y;

						k = GAME_HEIGHT_FLOAT / config.mode->height;
					}
					else
					{
						k = (FLOAT)config.mode->height / config.mode->width;

						x = GAME_WIDTH_FLOAT;
						y = k * x;

						k = GAME_WIDTH_FLOAT / config.mode->width;
					}

					lpPoint->x = LONG(fx * k * (FLOAT(lpPoint->x - rect.left) - offset.x) + ((FLOAT)config.mode->width - x) * 0.5f) + rect.left;
					lpPoint->y = LONG(fy * k * (FLOAT(lpPoint->y - rect.top) - offset.y) + ((FLOAT)config.mode->height - y) * 0.5f) + rect.top;
				}
			}

			return TRUE;
		}

		return FALSE;
	}

	DWORD cursorPos;
	LPPOINT __stdcall GetCursorPosHookV2a(LPPOINT lpPoint)
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
			if (!config.windowedMode && config.borderlessMode)
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

			if (!config.zoomImage || !config.isBorder)
			{
				X = LONG(fx * FLOAT(X - rect.left) + offset.x) + rect.left;
				Y = LONG(fy * FLOAT(Y - rect.top) + offset.y) + rect.top;
			}
			else
			{
				FLOAT x, y;
				FLOAT k = (FLOAT)config.mode->width / config.mode->height;
				if (k >= 4.0f / 3.0f)
				{
					y = GAME_HEIGHT_FLOAT;
					x = k * y;

					k = (FLOAT)config.mode->height / GAME_HEIGHT_FLOAT;
				}
				else
				{
					k = (FLOAT)config.mode->height / config.mode->width;

					x = GAME_WIDTH_FLOAT;
					y = k * x;

					k = (FLOAT)config.mode->width / GAME_WIDTH_FLOAT;
				}

				X = LONG(fx * k * (FLOAT(X - rect.left) - ((FLOAT)config.mode->width - x) * 0.5f) + offset.x) + rect.left;
				Y = LONG(fy * k * (FLOAT(Y - rect.top) - ((FLOAT)config.mode->height - y) * 0.5f) + offset.y) + rect.top;
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

	const IID CLSID_DirectDraw = { 0xD7B70EE0, 0x4340, 0x11CF, 0xB0, 0x63, 0x00, 0x20, 0xAF, 0xC2, 0xCD, 0x35 };

	HRESULT __stdcall CoCreateInstanceHook(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID* ppv)
	{
		if (!MemoryCompare((VOID*)&rclsid, &CLSID_DirectDraw, sizeof(IID)))
			return Main::DrawCreateEx(NULL, ppv, riid, NULL);

		return CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
	}

#pragma region 32 BPP
#define COLORKEY_AND 0x00F8FCF8
#define COLORKEY_CHECK 0x00F800F8

	DWORD pBinkCopyToBuffer;
	INT __stdcall BinkCopyToBufferHook(VOID* hBnk, VOID* dest, INT pitch, DWORD height, DWORD x, DWORD y, DWORD flags)
	{
		if (config.bpp32Hooked)
		{
			pitch <<= 1;
			flags = 0x80000004;
		}

		return ((INT(__stdcall*)(VOID*, VOID*, INT, DWORD, DWORD, DWORD, DWORD))pBinkCopyToBuffer)(hBnk, dest, pitch, height, x, y, flags);
	}

	BlendData blendList[32];

#pragma region Memory
	VOID __declspec(naked) hook_005A6311()
	{
		__asm {
			SHL EAX, 1
			IMUL EAX, DWORD PTR DS : [ESI + 0x4]
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
			SHL EAX, 2
			IMUL EAX, DWORD PTR SS : [EBP - 0x14]
			JMP back_005BACE7
		}
	}

	VOID __declspec(naked) hook_00674400()
	{
		__asm {
			SHL EAX, 1
			IMUL EAX, DWORD PTR SS : [EDI + 0x4]
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
			PUSH 0
			PUSH EDI
			XOR EDI, EDI
			SHL EAX, 2
			RETN
		}
	}
#pragma endregion

#pragma region PIXEL
	VOID __stdcall Pixel_Blit_Indexed_to_565(BYTE* srcData, DWORD srcPitch, DWORD* palette, DWORD* dstData, DWORD dstPitch, RECT* rect)
	{
		dstPitch >>= 1;

		BYTE* lpSrc = srcData + rect->top * srcPitch + rect->left;
		DWORD* lpDst = dstData + rect->top * dstPitch + rect->left;

		DWORD height = rect->bottom - rect->top;
		do
		{
			BYTE* src = lpSrc;
			DWORD* dst = lpDst;

			DWORD width = rect->right - rect->left;
			do
				*dst++ = palette[*src++] & 0x00FFFFFF | 0xFF000000;
			while (--width);

			lpSrc += srcPitch;
			lpDst += dstPitch;
		} while (--height);
	}

	DWORD __stdcall Pixel_ConvertPixel_565(BYTE red, BYTE green, BYTE blue)
	{
		return (red >> 3) | (green >> 3 << 6) | (blue >> 3 << 11);
	}

	VOID __stdcall Pixel_Blit_By_Masks(DWORD* srcData, LONG srcPitch, DWORD redMask, DWORD greenMask, DWORD blueMask, DWORD alphaMask, BYTE* dstData, LONG dstPitch, RECT* rect)
	{
		srcPitch >>= 1;

		if (dstPitch & 3)
			dstPitch = (dstPitch & 0xFFFFFFFC) + 4;

		srcData += rect->top * srcPitch + rect->left;

		if (!config.zoomImage || !config.isBorder)
			dstData += rect->top * dstPitch + rect->left * 3;

		LONG height = rect->bottom - rect->top;
		do
		{
			DWORD* src = srcData;
			BYTE* dst = dstData;

			LONG width = rect->right - rect->left;
			do
			{
				DWORD color = _byteswap_ulong(*src++);
				BYTE* px = (BYTE*)&color;

				*dst++ = *++px;
				*dst++ = *++px;
				*dst++ = *++px;
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
		/*DWORD height = size->cy;
  while (height--)
  {
          BYTE* px = data;

          DWORD width = size->cx;
          while (width--)
          {
                  BYTE temp = *(px + 2);
                  *(px + 2) = *px;
                  *px = temp;

                  px += 3;
          }

          data += pitch;
  }*/
	}

	DWORD __stdcall Pixel_Blend(DWORD dstData, DWORD srcData, BYTE msk)
	{
		DWORD res;

		if (msk == 255)
			res = srcData;
		else if (msk)
		{
			DWORD src = srcData & 0x000000FF;
			DWORD dst = dstData & 0x000000FF;
			DWORD r = (((dst * 255) + (src - dst) * msk) / 255) & 0x000000FF;

			src = srcData & 0x0000FF00;
			dst = dstData & 0x0000FF00;
			DWORD g = (((dst * 255) + (src - dst) * msk) / 255) & 0x0000FF00;

			src = srcData & 0x00FF0000;
			dst = dstData & 0x00FF0000;
			DWORD b = (((dst * 255) + (src - dst) * msk) / 255) & 0x00FF0000;

			res = r | g | b;
		}
		else
			res = dstData;

		return res | 0xFF000000;
	}

	DWORD __stdcall Pixel_BlendSome(DWORD pix, BYTE b, BYTE g, BYTE r, BYTE msk)
	{
		DWORD color = r | (g << 8) | (b << 16);
		return Pixel_Blend(pix, color, msk);
	}

	VOID __stdcall Pixel_BlitBlend(DWORD* srcData, DWORD* dstData, DWORD count, BYTE* mskData)
	{
		while (count--)
		{
			*dstData = Pixel_Blend(*dstData, *srcData, *(BYTE*)mskData);

			++srcData;
			++dstData;
			++mskData;
		}
	}

	VOID __stdcall Pixel_BlitBlendWithColorKey(BlendData* blendItem, DWORD count, DWORD colorKey)
	{
		colorKey = ((colorKey & 0x001F) << 3) | ((colorKey & 0x07E0) << 5) | ((colorKey & 0xF800) << 8);
		while (count--)
		{
			DWORD* srcData = blendItem->srcData;
			DWORD* dstData = blendItem->dstData;
			BYTE* mskData = (BYTE*)blendItem->mskData;

			DWORD length = blendItem->length;
			while (length--)
			{
				if ((*dstData & COLORKEY_AND) != colorKey)
					*dstData = Pixel_Blend(*dstData, *srcData, *(BYTE*)mskData);

				++srcData;
				++dstData;
				++mskData;
			}

			++blendItem;
		}
	}

	VOID __stdcall Pixel_BlitBlendAvarage(DWORD* srcData, LONG srcPitch, POINT* srcPos, DWORD* dstData, LONG dstPitch, POINT* dstPos, SIZE* size, BYTE flag, DWORD colorKey)
	{
		if (!flag || flag == 0xFF)
			return;

		colorKey = ((colorKey & 0x001F) << 3) | ((colorKey & 0x07E0) << 5) | ((colorKey & 0xF800) << 8);
		srcPitch >>= 1;
		dstPitch >>= 1;

		srcData += srcPitch * srcPos->y + srcPos->x;
		dstData += dstPitch * dstPos->y + dstPos->x;

		DWORD height = size->cy;
		while (height--)
		{
			DWORD* src = srcData;
			DWORD* dst = dstData;

			DWORD width = size->cx;
			while (width--)
			{
				if ((*src & COLORKEY_AND) != colorKey)
					*dst = Pixel_Blend(*dst, *src, 128);

				++src;
				++dst;
			}

			srcData += srcPitch;
			dstData += dstPitch;
		}
	}

	VOID __stdcall Pixel_Add(DWORD* srcData, LONG srcPitch, POINT* srcPos, DWORD* dstData, LONG dstPitch, POINT* dstPos, SIZE* size)
	{
		srcPitch >>= 1;
		dstPitch >>= 1;

		srcData += srcPitch * srcPos->y + srcPos->x;
		dstData += dstPitch * dstPos->y + dstPos->x;

		DWORD height = size->cy;
		while (height--)
		{
			DWORD* src = srcData;
			DWORD* dst = dstData;

			DWORD width = size->cx;
			while (width--)
			{
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

				*dst++ = r | g | b | 0xFF000000;
			}

			srcData += srcPitch;
			dstData += dstPitch;
		}
	}

	VOID __stdcall Pixel_Sub(DWORD* srcData, LONG srcPitch, POINT* srcPos, DWORD* dstData, LONG dstPitch, POINT* dstPos, SIZE* size)
	{
		srcPitch >>= 1;
		dstPitch >>= 1;

		srcData += srcPitch * srcPos->y + srcPos->x;
		dstData += dstPitch * dstPos->y + dstPos->x;

		DWORD height = size->cy;
		while (height--)
		{
			DWORD* src = srcData;
			DWORD* dst = dstData;

			DWORD width = size->cx;
			while (width--)
			{
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

				*dst++ = r | g | b | 0xFF000000;
			}

			srcData += srcPitch;
			dstData += dstPitch;
		}
	}

	VOID __stdcall Pixel_BlitColorKey(DWORD* srcData, LONG srcPitch, POINT* srcPos, DWORD* dstData, LONG dstPitch, POINT* dstPos, SIZE* size, BYTE flag, DWORD colorKey)
	{
		if (!flag || flag == 0xFF)
			return;

		colorKey = ((colorKey & 0x001F) << 3) | ((colorKey & 0x07E0) << 5) | ((colorKey & 0xF800) << 8);
		srcPitch >>= 1;
		dstPitch >>= 1;

		srcData += srcPitch * srcPos->y + srcPos->x;
		dstData += dstPitch * dstPos->y + dstPos->x;

		BOOL check = (~dstPos->y ^ dstPos->x) & 1;

		DWORD height = size->cy;
		while (height--)
		{
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
			while (width--)
			{
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
		srcPitch >>= 1;
		dstPitch >>= 1;

		srcData += srcPitch * srcPos->y + srcPos->x;
		dstData += dstPitch * dstPos->y + dstPos->x;

		BOOL check = (~dstPos->y ^ dstPos->x) & 1;

		DWORD height = size->cy;
		while (height--)
		{
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
			while (width--)
			{
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
		colorKey = ((colorKey & 0x001F) << 3) | ((colorKey & 0x07E0) << 5) | ((colorKey & 0xF800) << 8);
		pitch >>= 1;

		DWORD height = size->cy;
		while (height--)
		{
			DWORD* ptr = data;

			DWORD width = size->cx;
			while (width--)
			{
				if ((*ptr & COLORKEY_AND) == colorKey)
					*ptr = 0;
				else if (flag)
				{
					DWORD r = *ptr & 0xFF;
					DWORD g = (*ptr >> 8) & 0xFF;
					DWORD b = (*ptr >> 16) & 0xFF;
					DWORD a = (*ptr >> 24) & 0xFF;

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

						a <<= 1;
						if (a > 0xFF)
							a = 0xFF;

						*ptr = r | (g << 8) | (b << 16) | (a << 24);
					}
					else
						*ptr = 0;
				}

				++ptr;
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
						colorKey = ((colorKey & 0x001F) << 3) | ((colorKey & 0x07E0) << 5) | ((colorKey & 0xF800) << 8);

						DWORD srcPitch = pitch >> 1;
						DWORD dstPitch = obj->pitch >> 1;

						DWORD* srcData = (DWORD*)data + shiftY * srcPitch + shiftX;
						DWORD* dstData = (DWORD*)obj->data + top * dstPitch + left;

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
						colorKey = ((colorKey & 0x001F) << 3) | ((colorKey & 0x07E0) << 5) | ((colorKey & 0xF800) << 8);

						DWORD srcPitch = pitch >> 1;
						DWORD dstPitch = obj->pitch >> 1;

						DWORD* srcData = (DWORD*)data + shiftY * srcPitch + shiftX;
						DWORD* dstData = (DWORD*)obj->data + top * dstPitch + left;

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
			color = ((color & 0x001F) << 3) | ((color & 0x07E0) << 5) | ((color & 0xF800) << 8) | 0xFF000000;

			DWORD* data = (DWORD*)obj->data + rect->top * pitch + rect->left;
			DWORD height = rect->bottom - rect->top;
			while (height--)
			{
				DWORD* ptr = data;
				DWORD width = rect->right - rect->left;
				while (width--)
					*ptr++ = color;

				data += pitch;
			}
		}
		else
		{
			DWORD pitch = obj->pitch;

			BYTE* data = (BYTE*)obj->data + rect->top * pitch + rect->left;
			DWORD height = rect->bottom - rect->top;
			while (height--)
			{
				BYTE* ptr = data;
				DWORD width = rect->right - rect->left;
				while (width--)
					*ptr++ = LOBYTE(color);

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
						*dst++ = 0xFF000000;
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
				colorFill = ((colorFill & 0x001F) << 3) | ((colorFill & 0x07E0) << 5) | ((colorFill & 0xF800) << 8) | 0xFF000000;
				colorShadow = ((colorShadow & 0x001F) << 3) | ((colorShadow & 0x07E0) << 5) | ((colorShadow & 0xF800) << 8) | 0xFF000000;

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
					while (count--)
						srcVal <<= 1;
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
			colorFill = ((colorFill & 0x001F) << 3) | ((colorFill & 0x07E0) << 5) | ((colorFill & 0xF800) << 8) | 0xFF000000;

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
			colorFill = ((colorFill & 0x001F) << 3) | ((colorFill & 0x07E0) << 5) | ((colorFill & 0xF800) << 8) | 0xFF000000;

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
				blend = 0xC40000;
				break;

			case 2: // unavailable yet
				isBlend = TRUE;
				blend = 0x0000FF;
				break;

			case 3: // other line
				isBlend = TRUE;
				blend = 0x000000;
				break;

			default:
				isBlend = FALSE;
				break;
			}

			LONG srcPitch = width;
			LONG dstPitch = obj->pitch >> 1;

			DWORD* srcData = *((DWORD**)thisObj[11] + 1);
			DWORD* dstData = (DWORD*)obj->data;

			while (height--)
			{
				DWORD* src = srcData;
				DWORD* dst = dstData;

				INT count = width;
				while (count--)
				{
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
		while (height--)
		{
			DWORD* src = source;
			DWORD* msk = mask;
			DWORD* dst = destination;

			LONG count = width;
			while (count--)
			{
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

	VOID __stdcall DrawLine(BlitObject* obj, POINT* loc, DWORD count, DWORD color)
	{
		DWORD pitch = obj->pitch >> 1;
		color = ((color & 0x001F) << 3) | ((color & 0x07E0) << 5) | ((color & 0xF800) << 8) | 0xFF000000;

		DWORD* data = (DWORD*)obj->data + loc->y * pitch + loc->x;
		while (count--)
			*data++ = color;
	}

	DWORD __stdcall Color565toRGB(DWORD color)
	{
		return ((color & 0x001F) << 3) | ((color & 0x07E0) << 5) | ((color & 0xF800) << 8) | 0xFF000000;
	}

	DWORD back_005383AA;
	VOID __declspec(naked) hook_005383A5()
	{
		__asm {
			MOV EAX, DWORD PTR SS : [EBP - 0x1C]
			PUSH EAX
			CALL Color565toRGB
			MOV DWORD PTR SS : [EBP - 0x1C], EAX

			MOV EAX, DWORD PTR SS : [EBP - 0x14]
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
			MOV EAX, DWORD PTR DS : [ECX + 0x4]
			IMUL EAX, EBX
			ADD ESI, EAX
			MOV EAX, DWORD PTR DS : [ECX]
			MOV ECX, DWORD PTR SS : [EBP - 0x1C]
			MOV DWORD PTR DS : [EAX * 0x4 + ESI], ECX
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

	DWORD back_00539010;
	VOID __declspec(naked) hook_00538FEB()
	{
		__asm
		{
			MOV BYTE PTR config.isBorder, AL
			JMP back_00539010
		}
	}

	VOID __stdcall GetScreenshotSize(SIZE* size)
	{
		if (!config.zoomImage || !config.isBorder)
		{
			size->cx = config.mode->width;
			size->cy = config.mode->height;
		}
		else
		{
			FLOAT k = (FLOAT)config.mode->width / config.mode->height;
			if (k >= 4.0f / 3.0f)
			{
				size->cx = LONG(GAME_HEIGHT_FLOAT * k);
				size->cy = GAME_HEIGHT;
			}
			else
			{
				size->cx = GAME_WIDTH;
				size->cy = LONG(GAME_WIDTH_FLOAT * config.mode->height / config.mode->width);
			}
		}
	}

	VOID __stdcall GetScreenshotRectangle(RECT* rect, POINT* point, SIZE* size)
	{
		if (!config.zoomImage || !config.isBorder)
		{
			rect->left = 0;
			rect->top = 0;
		}
		else
		{
			rect->left = (config.mode->width - size->cx) >> 1;
			rect->top = (config.mode->height - size->cy) >> 1;
		}

		rect->right = rect->left + size->cx;
		rect->bottom = rect->top + size->cy;
	}

	VOID __declspec(naked) hook_005A6460()
	{
		__asm
		{
			POP EAX
			PUSH ECX
			PUSH EAX
			JMP GetScreenshotRectangle
		}
	}

	DWORD back_0066E533;
	VOID __declspec(naked) hook_0066E52D()
	{
		__asm
		{
			MOV EAX, [EBX + 0x8]
			SUB EAX, [EBX]
			MOV ECX, [EBX + 0xC]
			SUB ECX, [EBX + 0x4]

			JMP back_0066E533
		}
	}

	DWORD back_00489136;
	VOID __declspec(naked) hook_00489124()
	{
		__asm {
			CMP DWORD PTR DS : [ESI + 0x68] , EBX
			JE lbl_success
			CMP config.isBorder, EBX
			JE lbl_back

			lbl_success : MOV EAX, DWORD PTR DS : [ECX]
						  ADD ESI, 0x64

						  MOV EDX, [ESI + 0x4]
						  CMP config.isBorder, EBX
						  JE lbl_continue
						  SHR EDX, 0x1

						  lbl_continue : PUSH EDX
										 MOV EDX, [ESI]
										 CMP config.isBorder, EBX
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

	DWORD realTime, virtualTime, lastTime, lastVirtual;
	DOUBLE lastSpeed = 1.0;
	DWORD __stdcall timeGetTimeHook()
	{
		DOUBLE speed = config.speed.enabled ? config.speed.value : 1.0;

		if (speed != lastSpeed)
		{
			lastTime = realTime;
			lastVirtual = virtualTime;

			lastSpeed = speed;
		}

		realTime = timeGetTime();
		virtualTime = lastVirtual + DWORD(lastSpeed * (realTime - lastTime));

		return virtualTime;
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
			MOV ECX, DWORD PTR DS : [ECX]
			PUSH DWORD PTR DS : [ECX + 0x44]
			PUSH EAX
			JMP GetScrollTime
		}
	}

	VOID __declspec(naked) hook_0053CA08()
	{
		__asm {
			POP EAX
			MOV ECX, DWORD PTR DS : [ECX]
			PUSH DWORD PTR DS : [ECX + 0x40]
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

	DWORD back_0053D18B = 0x0053D18B;
	VOID __declspec(naked) hook_0053D185()
	{
		__asm {
			PUSH EAX
			PUSH EAX
			MOV ECX, ESP
			PUSH EAX

			PUSH ECX
			CALL GetScrollOffset

			POP EAX
			MOV EDI, DWORD PTR DS : [EAX]
			
			JMP back_0053D18B
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

		if (config.coldCPU)
			Sleep(1);
		else if (glVersion)
			Sleep(0);

		return FALSE;
	}
#pragma endregion

#pragma optimize("s", on)
	BOOL Load()
	{
		hModule = GetModuleHandle(NULL);
		PIMAGE_NT_HEADERS headNT = (PIMAGE_NT_HEADERS)(
			(BYTE*)hModule + ((PIMAGE_DOS_HEADER)hModule)->e_lfanew);
		baseOffset = (INT)hModule - (INT)headNT->OptionalHeader.ImageBase;

		{
			MappedFile file = { hModule, NULL, NULL, NULL };
			{
				PatchFunction(&file, "GetDeviceCaps", GetDeviceCapsHook);
				PatchFunction(&file, "GetForegroundWindow", GetForegroundWindowHook);

				PatchFunction(&file, "CreateWindowExA", CreateWindowExHook);
				PatchFunction(&file, "RegisterClassA", RegisterClassHook);

				PatchFunction(&file, "SetWindowLongA", SetWindowLongHook);

				PatchFunction(&file, "MessageBoxA", MessageBoxHook);
				PatchFunction(&file, "PeekMessageA", PeekMessageHook);

				PatchFunction(&file, "ShowCursor", ShowCursorHook);
				PatchFunction(&file, "ClipCursor", ClipCursorHook);

				PatchFunction(&file, "GetClientRect", GetClientRectHook);
				PatchFunction(&file, "GetWindowRect", GetWindowRectHook);

				PatchFunction(&file, "timeGetTime", timeGetTimeHook);

				PatchFunction(&file, "GetOpenFileNameA", GetOpenFileNameHook);
				PatchFunction(&file, "GetSaveFileNameA", GetSaveFileNameHook);

				if (config.version)
				{
					PatchFunction(&file, "CoCreateInstance", CoCreateInstanceHook);
					PatchFunction(&file, "GetCursorPos", GetCursorPosHookV1);
					PatchFunction(&file, "ClientToScreen", ClientToScreenHook);
					PatchFunction(&file, "GetDoubleClickTime", GetDoubleClickTimeHook);
				}
				else
				{
					PatchFunction(&file, "DirectDrawEnumerateExA", Main::DrawEnumerateEx);
					PatchFunction(&file, "DirectDrawCreate", Main::DrawCreate);
					PatchFunction(&file, "DirectDrawCreateEx", Main::DrawCreateEx);
					PatchFunction(&file, "GetCursorPos", GetCursorPosHookV2);
					PatchFunction(&file, "SetCursorPos", SetCursorPosHook);

					pBinkCopyToBuffer = PatchFunction(&file, "_BinkCopyToBuffer@28", BinkCopyToBufferHook);
				}
			}

			if (file.address)
				UnmapViewOfFile(file.address);

			if (file.hMap)
				CloseHandle(file.hMap);

			if (file.hFile)
				CloseHandle(file.hFile);
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
						equalSpace = hookSpace;
						break;
					}
				}

				++hookSpace;
			} while (--hookCount);

			hookSpace = equalSpace ? equalSpace : defaultSpace;
			if (hookSpace)
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

					PatchHook(hookSpace->scroll_speed, hook_0053D185);
					back_0053D18B = hookSpace->scroll_speed + 6 + baseOffset;
					scrollSpeed.multi = 16.0;
					scrollSpeed.offset.y = 16;
					scrollSpeed.offset.x = scrollSpeed.offset.y << 1;

					PatchCall(hookSpace->scroll_hook, hook_0053CA08);

					PatchCall(hookSpace->dblclick_hook, hook_0053A73D);
				}

				if (config.hd)
				{
					config.bpp32Hooked = TRUE;

					PatchBlock(hookSpace->pixel, (VOID*)pixelFunctions,
						sizeof(pixelFunctions));

					// GOOD
					// =================================================================

					PatchHook(hookSpace->fillColor, hook_0055D283); // Fill color
					PatchCall(hookSpace->minimapGround,
						DrawMinimapGround); // Minimap ground
					PatchHook(hookSpace->minimapObjects,
						hook_0055D419); // Draw minimap object
					PatchDWord(hookSpace->clearGround, (DWORD)ClearGround); // Clear ground
					PatchHook(hookSpace->mapGround, hook_005B5660); // Draw map ground
					PatchHook(hookSpace->waterBorders,
						hook_005B5560); // Draw water borders

					PatchHook(hookSpace->symbol, hook_005280D9); // Draw Symbol
					PatchCall(hookSpace->faces, DrawFaces);
					PatchDWord(hookSpace->buildings, (DWORD)DrawCastleBuildings);

					PatchHook(hookSpace->horLine, hook_0053056A); // Draw Horizontal Line
					PatchHook(hookSpace->verLine, hook_00530603); // Draw Vertical Line

					// GOOD
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

					if (hookSpace->res_hook)
					{
						config.resHooked = TRUE;

						INT cx, cy;
						if (!config.isExist)
						{
							cx = GetSystemMetrics(SM_CXSCREEN);
							cy = GetSystemMetrics(SM_CYSCREEN);

							FLOAT k = (FLOAT)cx / cy;
							if (k >= 4.0f / 3.0f)
							{
								cx = INT(GAME_HEIGHT_FLOAT * k);
								cy = GAME_HEIGHT;
							}
							else
							{
								cy = INT(GAME_WIDTH_FLOAT * cy / cx);
								cx = GAME_WIDTH;
							}

							Config::Set(CONFIG_WRAPPER, "DisplayWidth", cx);
							Config::Set(CONFIG_WRAPPER, "DisplayHeight", cy);

							config.menuZoomImage = TRUE;
							config.zoomImage = cx > GAME_WIDTH && cy > GAME_HEIGHT;
							Config::Set(CONFIG_DISCIPLE, "EnableZoom", TRUE);
						}
						else
						{
							cx = Config::Get(CONFIG_WRAPPER, "DisplayWidth", GAME_WIDTH);
							cy = Config::Get(CONFIG_WRAPPER, "DisplayHeight", GAME_HEIGHT);

							if (cx < GAME_WIDTH || cy < GAME_HEIGHT)
							{
								cx = GAME_WIDTH;
								cy = GAME_HEIGHT;
							}

							config.menuZoomImage = Config::Get(CONFIG_DISCIPLE, "EnableZoom", TRUE);
							if (cx > GAME_WIDTH && cy > GAME_HEIGHT)
								config.zoomImage = config.menuZoomImage;
						}

						config.mode = &modesList[1];
						config.mode->width = *(DWORD*)&cx;
						config.mode->height = *(DWORD*)&cy;
						config.resolution.width = LOWORD(config.mode->width);
						config.resolution.height = LOWORD(config.mode->height);

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
								config.zoomable = TRUE;
								config.gameBorders = TRUE;
							}
							else
							{
								config.zoomable = FALSE;
								config.gameBorders = FALSE;
								config.menuZoomImage = FALSE;
								config.zoomImage = FALSE;
							}

							PatchByte(hookSpace->border_nop + 0x16,
								0xEB); // remove internal borders
							PatchHook(hookSpace->border_hook, hook_00538FEB); // ini
							back_00539010 = hookSpace->border_hook + 0x25 + baseOffset;
						}

						// Blit count
						{
							BYTE value;
							ReadByte(hookSpace->blit_size + 2, &value);

							DWORD count = DWORD((3072.0f * config.mode->width * config.mode->height) / (1280.0f * 960.0f));
							PatchDWord(hookSpace->blit_patch_1 + 1, count * value);
							PatchDWord(hookSpace->blit_patch_1 + 0xC + 3, count);

							PatchDWord(hookSpace->blit_patch_2 + 1, count * 20);
							PatchDWord(hookSpace->blit_patch_2 + 0x17 + 6, count);
						}

						// Minimap  rectangle
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

						// Screenshot zoomed size
						{
							PatchCall(hookSpace->snapshot_size, GetScreenshotSize);
							PatchCall(hookSpace->snapshot_rect, hook_005A6460);
							PatchNop(hookSpace->snapshot_nop1, 6);

							PatchHook(hookSpace->snapshot_hook, hook_0066E52D);
							back_0066E533 = hookSpace->snapshot_hook + 6 + baseOffset;

							PatchNop(hookSpace->snapshot_nop2, 0x2E);
						}

						if (config.mode->width > 1152)
						{
							PatchDWord(hookSpace->maxSize_1 + 2, (DWORD)&config.mode->width);
							PatchDWord(hookSpace->maxSize_2 + 2, (DWORD)&config.mode->width);
							PatchDWord(hookSpace->maxSize_3 + 6 + 2,
								(DWORD)&config.mode->width);
						}

						if (config.mode->height > 1152)
						{
							PatchDWord(hookSpace->maxSize_1 + 29 + 2,
								(DWORD)&config.mode->height);
							PatchDWord(hookSpace->maxSize_2 + 28 + 2,
								(DWORD)&config.mode->height);
							PatchDWord(hookSpace->maxSize_3 + 2, (DWORD)&config.mode->height);
						}

						// Png
						{
							pnglib_create_read_struct = (PNG_CREATE_READ_STRUCT)(
								hookSpace->png_create_read_struct + baseOffset);
							pnglib_create_info_struct = (PNG_CREATE_INFO_STRUCT)(
								hookSpace->png_create_info_struct + baseOffset);
							pnglib_set_read_fn = (PNG_SET_READ_FN)(hookSpace->png_set_read_fn + baseOffset);
							pnglib_destroy_read_struct = (PNG_DESTROY_READ_STRUCT)(
								hookSpace->png_destroy_read_struct + baseOffset);
							pnglib_read_info = (PNG_READ_INFO)(hookSpace->png_read_info + baseOffset);
							pnglib_read_image = (PNG_READ_IMAGE)(hookSpace->png_read_image + baseOffset);
						}
					}
				}
			}

			return TRUE;
		}

		return FALSE;
	}
#pragma optimize("", on)
}