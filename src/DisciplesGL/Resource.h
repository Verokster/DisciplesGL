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

//{{NO_DEPENDENCIES}}
// Microsoft Visual C++ generated include file.
// Used by module.rc
//

#pragma region Commands
#define IDC_BTN_OK 1
#define IDC_TITLE 2
#define IDC_VERSION 3
#define IDC_COPYRIGHT 4
#define IDC_LNK_EMAIL 5
#define IDC_LNK_WEB 6
#define IDC_LNK_PATRON 7

#define IDC_BTN_CANCEL 2
#define IDC_BTN_RESET 3
#define IDC_BTN_AUTO 4
#define IDC_CANVAS 5
#define IDC_LBL_HUE 6
#define IDC_TRK_HUE 7
#define IDC_LBL_SAT 8
#define IDC_TRK_SAT 9
#define IDC_RAD_RGB 10
#define IDC_RAD_RED 11
#define IDC_RAD_GREEN 12
#define IDC_RAD_BLUE 13
#define IDC_LBL_IN_LEFT 14
#define IDC_TRK_IN_LEFT 15
#define IDC_LBL_IN_RIGHT 16
#define IDC_TRK_IN_RIGHT 17
#define IDC_LBL_GAMMA 18
#define IDC_TRK_GAMMA 19
#define IDC_LBL_OUT_LEFT 20
#define IDC_TRK_OUT_LEFT 21
#define IDC_LBL_OUT_RIGHT 22
#define IDC_TRK_OUT_RIGHT 23
#pragma endregion

#pragma region Resources
#define IDR_MANIFEST 2

#define IDR_BACK 4
#define IDR_BORDER 5
#define IDR_BORDER_WIDE 6
#define IDR_ALT 7
#define IDR_ALT_WIDE 8

#define IDR_DLG_BATTLE_B 10
#define IDR_DLG_ISO_NEW 11
#define IDR_DLG_ISO_VIEW_NEW 12
#define IDR_DLG_STRATEGIC_OLD 13
#define IDR_DLG_STRATEGIC_NEW 14
#define IDR_DLG_STRAT_SPELL_NEW 15
#define IDR_DLG_STRAT_WAIT_NEW 16
#define IDR_DLG_ISO_EDIT 17
#define IDR_DLG_MAP_EDIT 18
#define IDR_DLG_LANDMARK_EDIT 19
#define IDR_DLG_EXCEPTIONAL_EDIT 20

#define IDR_LINEAR_VERTEX 30
#define IDR_LINEAR_FRAGMENT 31

#define IDR_HERMITE_VERTEX IDR_LINEAR_VERTEX
#define IDR_HERMITE_FRAGMENT 33

#define IDR_CUBIC_VERTEX IDR_LINEAR_VERTEX
#define IDR_CUBIC_FRAGMENT 35

#define IDR_LANCZOS_VERTEX IDR_LINEAR_VERTEX
#define IDR_LANCZOS_FRAGMENT 37

#define IDR_XBRZ_VERTEX IDR_LINEAR_VERTEX
#define IDR_XBRZ_FRAGMENT_2X 41
#define IDR_XBRZ_FRAGMENT_3X 42
#define IDR_XBRZ_FRAGMENT_4X 43
#define IDR_XBRZ_FRAGMENT_5X 44
#define IDR_XBRZ_FRAGMENT_6X 45

#define IDR_SCALEHQ_VERTEX_2X IDR_LINEAR_VERTEX
#define IDR_SCALEHQ_FRAGMENT_2X 51
#define IDR_SCALEHQ_VERTEX_4X IDR_LINEAR_VERTEX
#define IDR_SCALEHQ_FRAGMENT_4X 53

#define IDR_XSAL_VERTEX IDR_LINEAR_VERTEX
#define IDR_XSAL_FRAGMENT 61

#define IDR_EAGLE_VERTEX IDR_LINEAR_VERTEX
#define IDR_EAGLE_FRAGMENT 71

#define IDR_SCALENX_VERTEX_2X IDR_LINEAR_VERTEX
#define IDR_SCALENX_FRAGMENT_2X 81
#define IDR_SCALENX_VERTEX_3X IDR_LINEAR_VERTEX
#define IDR_SCALENX_FRAGMENT_3X 83

#define IDR_MENU 100
#pragma endregion

#pragma region Dialogs
#define IDD_ABOUT_APPLICATION 20
#define IDD_ABOUT_APPLICATION_OLD 21
#define IDD_ABOUT_WRAPPER 22
#define IDD_ABOUT_WRAPPER_OLD 23
#define IDD_COLOR_ADJUSTMENT 24
#pragma endregion

#pragma region Menu
#define IDM_REND_AUTO 1
#define IDM_REND_GL1 2
#define IDM_REND_GL2 3
#define IDM_REND_GL3 4
#define IDM_REND_GDI 5

#define IDM_FILT_OFF 10
#define IDM_FILT_LINEAR 11
#define IDM_FILT_HERMITE 12
#define IDM_FILT_CUBIC 13
#define IDM_FILT_LANCZOS 14

#define IDM_FILT_NONE 20

#define IDM_FILT_XRBZ_2X 22
#define IDM_FILT_XRBZ_3X 23
#define IDM_FILT_XRBZ_4X 24
#define IDM_FILT_XRBZ_5X 25
#define IDM_FILT_XRBZ_6X 26

#define IDM_FILT_SCALEHQ_2X 32
#define IDM_FILT_SCALEHQ_4X 34

#define IDM_FILT_XSAL_2X 42

#define IDM_FILT_EAGLE_2X 52

#define IDM_FILT_SCALENX_2X 62
#define IDM_FILT_SCALENX_3X 63

#define IDM_ASPECT_RATIO 70
#define IDM_VSYNC 71

#define IDM_RES_FULL_SCREEN 80
#define IDM_HELP_ABOUT_APPLICATION 81
#define IDM_HELP_ABOUT_WRAPPER 82
#define IDM_FILE_RESET 83
#define IDM_FILE_QUIT 84

#define IDM_BORDERS_OFF 85
#define IDM_BORDERS_CLASSIC 86
#define IDM_BORDERS_ALTERNATIVE 87

#define IDM_BACKGROUND 88
#define IDM_COLOR_ADJUST 89

#define IDM_FAST_AI 90
#define IDM_PATCH_CPU 91
#define IDM_ALWAYS_ACTIVE 92

#define IDM_MODE_BORDERLESS 93
#define IDM_MODE_EXCLUSIVE 94

#define IDM_MODE_WIDEBATTLE 95

#define IDM_SCENE_SORT_NAME 100
#define IDM_SCENE_SORT_FILE 101
#define IDM_SCENE_SORT_SIZE_ASC 102
#define IDM_SCENE_SORT_SIZE_DESC 103

#define IDM_SCENE_ALL 105

#define IDM_EDITOR_SCENE 106
#define IDM_EDITOR_CAMP 107

#define IDM_MAP_LMB 110
#define IDM_MAP_MMB 111
#define IDM_MAP_EDGE 112

#define IDM_RES_640_480 200
#define IDM_RES_800_600 (IDM_RES_640_480 + 1)
#define IDM_RES_960_768 (IDM_RES_640_480 + 2)
#define IDM_RES_1024_600 (IDM_RES_640_480 + 3)
#define IDM_RES_1024_768 (IDM_RES_640_480 + 4)
#define IDM_RES_1152_864 (IDM_RES_640_480 + 5)
#define IDM_RES_1280_720 (IDM_RES_640_480 + 6)
#define IDM_RES_1280_768 (IDM_RES_640_480 + 7)
#define IDM_RES_1280_800 (IDM_RES_640_480 + 8)
#define IDM_RES_1280_960 (IDM_RES_640_480 + 9)
#define IDM_RES_1280_1024 (IDM_RES_640_480 + 10)
#define IDM_RES_1360_768 (IDM_RES_640_480 + 11)
#define IDM_RES_1366_768 (IDM_RES_640_480 + 12)
#define IDM_RES_1400_1050 (IDM_RES_640_480 + 13)
#define IDM_RES_1440_900 (IDM_RES_640_480 + 14)
#define IDM_RES_1440_1080 (IDM_RES_640_480 + 15)
#define IDM_RES_1536_864 (IDM_RES_640_480 + 16)
#define IDM_RES_1600_900 (IDM_RES_640_480 + 17)
#define IDM_RES_1600_1200 (IDM_RES_640_480 + 18)
#define IDM_RES_1680_1050 (IDM_RES_640_480 + 19)
#define IDM_RES_1920_1080 (IDM_RES_640_480 + 20)
#define IDM_RES_1920_1200 (IDM_RES_640_480 + 21)
#define IDM_RES_1920_1440 (IDM_RES_640_480 + 22)
#define IDM_RES_2048_1536 (IDM_RES_640_480 + 23)
#define IDM_RES_2560_1440 (IDM_RES_640_480 + 24)
#define IDM_RES_2560_1600 (IDM_RES_640_480 + 25)
#define IDM_RES_3840_2160 (IDM_RES_640_480 + 26)
#define IDM_RES_7680_4320 (IDM_RES_640_480 + 27)

#define IDM_SPEED_1_0 230
#define IDM_SPEED_1_1 (IDM_SPEED_1_0 + 1)
#define IDM_SPEED_1_2 (IDM_SPEED_1_0 + 2)
#define IDM_SPEED_1_3 (IDM_SPEED_1_0 + 3)
#define IDM_SPEED_1_4 (IDM_SPEED_1_0 + 4)
#define IDM_SPEED_1_5 (IDM_SPEED_1_0 + 5)
#define IDM_SPEED_1_6 (IDM_SPEED_1_0 + 6)
#define IDM_SPEED_1_7 (IDM_SPEED_1_0 + 7)
#define IDM_SPEED_1_8 (IDM_SPEED_1_0 + 8)
#define IDM_SPEED_1_9 (IDM_SPEED_1_0 + 9)
#define IDM_SPEED_2_0 (IDM_SPEED_1_0 + 10)
#define IDM_SPEED_2_1 (IDM_SPEED_1_0 + 11)
#define IDM_SPEED_2_2 (IDM_SPEED_1_0 + 12)
#define IDM_SPEED_2_3 (IDM_SPEED_1_0 + 13)
#define IDM_SPEED_2_4 (IDM_SPEED_1_0 + 14)
#define IDM_SPEED_2_5 (IDM_SPEED_1_0 + 15)
#define IDM_SPEED_2_6 (IDM_SPEED_1_0 + 16)
#define IDM_SPEED_2_7 (IDM_SPEED_1_0 + 17)
#define IDM_SPEED_2_8 (IDM_SPEED_1_0 + 18)
#define IDM_SPEED_2_9 (IDM_SPEED_1_0 + 19)
#define IDM_SPEED_3_0 (IDM_SPEED_1_0 + 20)

#define IDM_SCR_BMP 259
#define IDM_SCR_PNG 260
#define IDM_SCR_1 (IDM_SCR_PNG + 1)
#define IDM_SCR_2 (IDM_SCR_PNG + 2)
#define IDM_SCR_3 (IDM_SCR_PNG + 3)
#define IDM_SCR_4 (IDM_SCR_PNG + 4)
#define IDM_SCR_5 (IDM_SCR_PNG + 5)
#define IDM_SCR_6 (IDM_SCR_PNG + 6)
#define IDM_SCR_7 (IDM_SCR_PNG + 7)
#define IDM_SCR_8 (IDM_SCR_PNG + 8)
#define IDM_SCR_9 (IDM_SCR_PNG + 9)

#define IDM_MSG_0 270
#define IDM_MSG_5 (IDM_MSG_0 + 5)
#define IDM_MSG_10 (IDM_MSG_0 + 10)
#define IDM_MSG_15 (IDM_MSG_0 + 15)
#define IDM_MSG_20 (IDM_MSG_0 + 20)
#define IDM_MSG_25 (IDM_MSG_0 + 25)
#define IDM_MSG_30 (IDM_MSG_0 + 30)

#define IDM_STRETCH_OFF 400
#define IDM_STRETCH_10 (IDM_STRETCH_OFF + 10)
#define IDM_STRETCH_20 (IDM_STRETCH_OFF + 20)
#define IDM_STRETCH_30 (IDM_STRETCH_OFF + 30)
#define IDM_STRETCH_40 (IDM_STRETCH_OFF + 40)
#define IDM_STRETCH_50 (IDM_STRETCH_OFF + 50)
#define IDM_STRETCH_60 (IDM_STRETCH_OFF + 60)
#define IDM_STRETCH_70 (IDM_STRETCH_OFF + 70)
#define IDM_STRETCH_80 (IDM_STRETCH_OFF + 80)
#define IDM_STRETCH_90 (IDM_STRETCH_OFF + 90)
#define IDM_STRETCH_100 (IDM_STRETCH_OFF + 100)

#define IDM_LOCALE_DEFAULT 5000

#define IDM_MODS 10000
#pragma endregion

#pragma region Strings
#define IDS_ERROR_CHOOSE_PF 300
#define IDS_ERROR_SET_PF 301
#define IDS_ERROR_DESCRIBE_PF 302
#define IDS_ERROR_NEED_PALETTE 303
#define IDS_ERROR_BAD_PF 304
#define IDS_ERROR_ARB_VERSION 305
#define IDS_ERROR_ARB_PROFILE 306
#define IDS_ERROR_LOAD_RESOURCE 307
#define IDS_ERROR_COMPILE_SHADER 308
#define IDS_INFO_RESTART 309
#define IDS_INFO_NEXT_BATTLE 310
#define IDS_WARN_RESET 311
#define IDS_INFO_RESET 312
#define IDS_WARN_FAST_AI 314

#define IDS_TEXT_INTERPOLATIN 320
#define IDS_TEXT_UPSCALING 321

#define IDS_TEXT_FILT_OFF 340
#define IDS_TEXT_FILT_LINEAR 341
#define IDS_TEXT_FILT_HERMITE 342
#define IDS_TEXT_FILT_CUBIC 343
#define IDS_TEXT_FILT_LANCZOS 344
#define IDS_TEXT_FILT_NONE 345
#define IDS_TEXT_FILT_SCALENX 346
#define IDS_TEXT_FILT_EAGLE 347
#define IDS_TEXT_FILT_XSAL 348
#define IDS_TEXT_FILT_SCALEHQ 349
#define IDS_TEXT_FILT_XBRZ 350

#define IDS_TEXT_RENDERER 351

#define IDS_ON 352
#define IDS_OFF 353
#define IDS_RES_FULL_SCREEN 354
#define IDS_MODE_BORDERLESS 355
#define IDS_MODE_EXCLUSIVE 356
#define IDS_VSYNC 357
#define IDS_ASPECT_RATIO 358
#define IDS_ASPECT_KEEP 359
#define IDS_ASPECT_STRETCH 360
#define IDS_STRETCH 361
#define IDS_BORDERS 362
#define IDS_BORDERS_CLASSIC 363
#define IDS_BORDERS_ALTERNATIVE 364
#define IDS_BACKGROUND 365
#define IDS_SPEED 366
#define IDS_MSG 367
#define IDS_MSG_SECONDS 368
#define IDS_FAST_AI 369
#define IDS_ALWAYS_ACTIVE 370
#define IDS_PATCH_CPU 371
#define IDS_LOCALE 372
#define IDS_LOCALE_DEFAULT 373
#define IDS_SCR_TYPE 374
#define IDS_SCR_BMP 375
#define IDS_SCR_PNG 376
#define IDS_SCR_FILE 377

#define IDS_REND_AUTO 378
#define IDS_REND_GL1 379
#define IDS_REND_GL2 380
#define IDS_REND_GL3 381
#define IDS_REND_GDI 382
#pragma endregion

// Next default values for new objects
//
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE 101
#define _APS_NEXT_COMMAND_VALUE 40001
#define _APS_NEXT_CONTROL_VALUE 1001
#define _APS_NEXT_SYMED_VALUE 101
#endif
#endif
