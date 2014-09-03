﻿
/*
Copyright (c) 2009-2014 Maximus5
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define HIDE_USE_EXCEPTION_INFO

#define SHOWDEBUGSTR

#define DEBUGSTRSTAT(s) //DEBUGSTR(s)
#define DEBUGSTRSIZE(s) DEBUGSTR(s)

#include <windows.h>
#include <commctrl.h>

#include "header.h"

#include "ConEmu.h"
#include "Inside.h"
#include "Menu.h"
#include "Options.h"
#include "RealConsole.h"
#include "Status.h"
#include "TabBar.h"
#include "VConGroup.h"
#include "VirtualConsole.h"


#define STATUS_PAINT_DELAY 500

#undef DUMP_STATUS_IMG
#ifdef _DEBUG
	#include "ScreenDump.h"

	#define DUMP_STATUS_IMG
#endif

const wchar_t gsReady[] = L"Waiting...";

//Информация по колонкам
//{
//	CEStatusItems nID;
//	LPCWSTR sSettingName;
//	LPCWSTR sName; // Shown in menu (select active columns)
//	LPCWSTR sHelp; // Shown in Info, when hover over item
//}
static StatusColInfo gStatusCols[] =
{
	// строго первая, НЕ отключаемая
	{csi_Info,  NULL,   L"Show status bar",
						L"Hide status bar, you may restore it later from 'Settings...'"},
	// Далее - по настройкам
	{csi_ConsoleTitle,	L"StatusBar.Hide.Title",
						L"Console title",
						L"Active console title"},
	{csi_ActiveProcess,	L"StatusBar.Hide.Proc",
						L"Active process",
						L"Active process"},
	{csi_ActiveVCon,	L"StatusBar.Hide.VCon",
						L"Active VCon",
						L"ActiveCon/TotalCount, left click to change"},
	{csi_NewVCon,		L"StatusBar.Hide.New",
						L"Create new console",
						L"Show create new console popup menu"},

	{csi_SyncInside,	L"StatusBar.Hide.Sync",
						L"Synchronize cur dir",
						L"Synchronize current directory in the 'ConEmu Inside' mode"},

	{csi_CapsLock,		L"StatusBar.Hide.CapsL",
						L"Caps Lock state",
						L"Caps Lock state, left click to change"},

	{csi_NumLock,		L"StatusBar.Hide.NumL",
						L"Num Lock state",
						L"Num Lock state, left click to change"},

	{csi_ScrollLock,	L"StatusBar.Hide.ScrL",
						L"Scroll Lock state",
						L"Scroll Lock state, left click to change"},

	{csi_KeyHooks,		L"StatusBar.Hide.KeyHooks",
						L"Keyboard hooks",
						L"Install keyboard hooks status"},

	{csi_InputLocale,	L"StatusBar.Hide.Lang",
						L"Current input HKL",
						L"Active input locale identifier - GetKeyboardLayout"},

	{csi_WindowPos,		L"StatusBar.Hide.WPos",
						L"ConEmu window rectangle",
						L"(X1, Y1)-(X2, Y2): ConEmu window rectangle"},

	{csi_WindowSize,	L"StatusBar.Hide.WSize",
						L"ConEmu window size",
						L"Width x Height: ConEmu window size"},

	{csi_WindowClient,	L"StatusBar.Hide.WClient",
						L"ConEmu window client area size",
						L"Width x Height: ConEmu client area size (without caption and frame)"},

	{csi_WindowWork,	L"StatusBar.Hide.WWork",
						L"ConEmu window work area size",
						L"Width x Height: ConEmu work area size (virtual consoles place)"},

	{csi_WindowStyle,	L"StatusBar.Hide.Style",
						L"ConEmu window style",
						L"GWL_STYLE: ConEmu window style"},
	{csi_WindowStyleEx,	L"StatusBar.Hide.StyleEx",
						L"ConEmu window extended styles",
						L"GWL_EXSTYLE: ConEmu window extended styles"},

	{csi_HwndFore,		L"StatusBar.Hide.hFore",
						L"Foreground HWND",
						L"Foreground HWND"},
	{csi_HwndFocus,		L"StatusBar.Hide.hFocus",
						L"Focus HWND",
						L"Focus HWND"},

	{csi_ActiveBuffer,	L"StatusBar.Hide.ABuf",
						L"Active console buffer",
						L"PRI/ALT/SEL/FND/DMP: Active console buffer"},

	{csi_Zoom,			L"StatusBar.Hide.Zoom",
						L"Current zoom value",
						L"Ctrl+WheelUp/WheelDown change main font zoom"},
	{csi_DPI,			L"StatusBar.Hide.Dpi",
						L"Current DPI value",
						L"Show DPI for monitor with main window"},

	{csi_ConsolePos,	L"StatusBar.Hide.CPos",
						L"Console visible rectangle",
						L"(Left, Top)-(Right,Bottom): Console visible rect, 0-based"},

	{csi_ConsoleSize,	L"StatusBar.Hide.CSize",
						L"Console visible size",
						L"Width x Height: Console visible rect"},

	{csi_BufferSize,	L"StatusBar.Hide.BSize",
						L"Console buffer size",
						L"Width x Height: Console buffer size"},

	{csi_CursorX,		L"StatusBar.Hide.CurX",
						L"Cursor column",
						L"Col: Console cursor pos, 0-based"},

	{csi_CursorY,		L"StatusBar.Hide.CurY",
						L"Cursor row",
						L"Row: Console cursor pos, 0-based"},

	{csi_CursorSize,	L"StatusBar.Hide.CurS",
						L"Cursor size / visibility",
						L"Height (V|H): Console cursor size and visibility"},

	{csi_CursorInfo,	L"StatusBar.Hide.CurI",
						L"Cursor information",
						L"Col, Row, Height (visible|hidden): Console cursor, 0-based"},

	{csi_ConEmuPID,		L"StatusBar.Hide.ConEmuPID",
						L"ConEmu GUI PID",
						L"ConEmu GUI PID"},

	{csi_ConEmuHWND,	L"StatusBar.Hide.ConEmuHWND",
						L"ConEmu GUI HWND",
						L"ConEmu GUI HWND"},

	{csi_ConEmuView,	L"StatusBar.Hide.ConEmuView",
						L"ConEmu GUI View HWND",
						L"ConEmu GUI View HWND"},

	{csi_Server,		L"StatusBar.Hide.Srv",
						L"Console server PID",
						L"Server PID / AltServer PID"},

	{csi_ServerHWND,	L"StatusBar.Hide.SrvHWND",
						L"Console server HWND",
						L"Server HWND"},

	{csi_Transparency,	L"StatusBar.Hide.Transparency",
						L"Transparency",
						L"Transparency, left click to change"},

	{csi_Time,			L"StatusBar.Hide.Time",
						L"System time",
						L"System time"},

	{csi_SizeGrip,		L"StatusBar.Hide.Resize",
						L"Size grip",
						L"Click and drag size grip to resize ConEmu window"},
};


static CStatus::StatusMenuOptions gTranspOpt[] = {
	{1, L"Transparent, 40%",       40, CStatus::Transparent_IsMenuChecked},
	{2, L"Transparent, 50%",       50, CStatus::Transparent_IsMenuChecked},
	{3, L"Transparent, 60%",       60, CStatus::Transparent_IsMenuChecked},
	{4, L"Transparent, 70%",       70, CStatus::Transparent_IsMenuChecked},
	{5, L"Transparent, 80%",       80, CStatus::Transparent_IsMenuChecked},
	{6, L"Transparent, 90%",       90, CStatus::Transparent_IsMenuChecked},
	{0},
	{7, L"UserScreen transparency", 1, CStatus::Transparent_IsMenuChecked},
	{8, L"ColorKey transparency",   2, CStatus::Transparent_IsMenuChecked},
	{9, L"Opaque, 100%",          100, CStatus::Transparent_IsMenuChecked},
};

static CStatus::StatusMenuOptions gZoomOpt[] = {
	{1, L"Zoom 300%", 300, CStatus::Zoom_IsMenuChecked},
	{2, L"Zoom 250%", 250, CStatus::Zoom_IsMenuChecked},
	{3, L"Zoom 200%", 200, CStatus::Zoom_IsMenuChecked},
	{4, L"Zoom 175%", 175, CStatus::Zoom_IsMenuChecked},
	{5, L"Zoom 150%", 150, CStatus::Zoom_IsMenuChecked},
	{6, L"Zoom 125%", 125, CStatus::Zoom_IsMenuChecked},
	{7, L"Zoom 100%", 100, CStatus::Zoom_IsMenuChecked},
	{8, L"Zoom 75%",   75, CStatus::Zoom_IsMenuChecked},
	{9, L"Zoom 50%",   50, CStatus::Zoom_IsMenuChecked},
};


CStatus::CStatus()
{
	//mb_WasClick = false;
	mb_InPopupMenu = false;

	mb_DataChanged = true;
	//mb_Invalidated = false;
	mn_LastPaintTick = 0;

	m_ClickedItemDesc = csi_Last;
	mb_InSetupMenu = false;

	mn_Style = mn_ExStyle = 0;
	mn_Zoom = mn_Dpi = 0;
	mh_Fore = mh_Focus = NULL;
	mn_ForePID = mn_FocusPID = 0;
	ms_ForeInfo[0] = ms_FocusInfo[0] = 0;

	mb_Caps = mb_Num = mb_Scroll = false;
	mb_KeyHooks = false;
	mhk_Locale = 0;

	ms_Status[0] = 0;

	ZeroStruct(m_Items);
	ZeroStruct(m_Values);
	ZeroStruct(mrc_LastStatus);
	ZeroStruct(mrc_LastResizeCol);
	ZeroStruct(mt_LastTime);
	mb_StatusResizing = false;

	//mn_BmpSize; -- не важно
	mb_OldBmp = mh_Bmp = NULL; mh_MemDC = NULL;
	ZeroStruct(mn_BmpSize);

	for (int i = csi_Info; i < csi_Last; i++)
	{
		for (size_t j = 0; j < countof(gStatusCols); j++)
		{
			if (i == gStatusCols[j].nID)
			{
				m_Values[i].sName = gStatusCols[j].sName;
				m_Values[i].sHelp = gStatusCols[j].sHelp ? gStatusCols[j].sHelp : gStatusCols[j].sName;
				m_Values[i].sSettingName = gStatusCols[j].sSettingName;
				wcscpy_c(m_Values[i].sText, L" ");
				wcscpy_c(m_Values[i].szFormat, L" ");
				break;
			}
		}
	}

	#ifdef _DEBUG
	for (int j = csi_Info+1; j < csi_Last; j++)
	{
		// Проверяем, все csi_xxx элементы должны быть описаны в gStatusCols
		_ASSERTE(m_Values[j].sHelp && m_Values[j].sName && m_Values[j].sSettingName);
	}
	#endif

	_wsprintf(m_Values[csi_ConEmuPID].sText, SKIPLEN(countof(m_Values[csi_ConEmuPID].sText)) L"%u", GetCurrentProcessId());

	// Init some values
	OnTransparency();

	// Fixed and self-draw
	wcscpy_c(m_Values[csi_SizeGrip].sText, L"//");
	wcscpy_c(m_Values[csi_SizeGrip].szFormat, L"//");
	wcscpy_c(m_Values[csi_Time].sText, L"00:00:00");
	wcscpy_c(m_Values[csi_Time].szFormat, L"00:00:00");

	_ASSERTE(gpConEmu && *gpConEmu->ms_ConEmuBuild);
	_wsprintf(ms_ConEmuBuild, SKIPLEN(countof(ms_ConEmuBuild)) L" %c %s[%u%s]",
		0x00AB/* « */, gpConEmu->ms_ConEmuBuild, WIN3264TEST(32,64), RELEASEDEBUGTEST(L"","D"));
}

CStatus::~CStatus()
{
	if (mh_MemDC)
	{
		SelectObject(mh_MemDC, mb_OldBmp);
		DeleteObject(mh_Bmp);
		DeleteDC(mh_MemDC);
	}
}

// static
size_t CStatus::GetAllStatusCols(StatusColInfo** ppColumns)
{
	if (ppColumns)
		*ppColumns = gStatusCols;
	return countof(gStatusCols);
}

// true = text changed
bool CStatus::FillItems()
{
	bool lbChanged = false;

	TODO("CStatus::FillItems()");

	return lbChanged;
}

bool CStatus::LoadActiveProcess(CRealConsole* pRCon, wchar_t* pszText, int cchMax)
{
	bool lbRc = false;
	DWORD nPID = 0;

	if (!pRCon || ((nPID = pRCon->GetActivePID()) == 0))
	{
		//lstrcpyn(pszText, gsReady, cchMax);
	}
	else
	{
		LPCWSTR pszName = pRCon->GetActiveProcessName();
		TODO("Показать все дерево запущенных процессов");
		wchar_t* psz = pszText;
		int nCchLeft = cchMax;
		lstrcpyn(psz, (pszName && *pszName) ? pszName : L"???", 64);
		int nCurLen = lstrlen(psz);
		_wsprintf(psz+nCurLen, SKIPLEN(nCchLeft-nCurLen) _T(":%u"), nPID);
		//_wsprintf(psz+nCurLen, SKIPLEN(nCchLeft-nCurLen) _T(":%u « %s%s"),
		//	nPID, gpConEmu->ms_ConEmuBuild, WIN3264TEST(L"x86",L"x64"));
		UNREFERENCED_PARAMETER(nCchLeft);
		lbRc = true;
	}

	return lbRc;
}

void CStatus::PaintStatus(HDC hPaint, LPRECT prcStatus /*= NULL*/)
{
	#ifdef DUMP_STATUS_IMG
	static bool bNeedDumpImage;
	LPCWSTR pszDumpImgName = L"C:\\ConEmu\\StatusMem.png";
	LPCWSTR pszDumpImgDstName = L"C:\\ConEmu\\StatusDst.png";
	#endif

	_ASSERTE(isMainThread());

	RECT rcStatus = {0,0};
	if (prcStatus)
	{
		rcStatus = *prcStatus;
	}
	else
	{
		GetStatusBarClientRect(&rcStatus);
	}

	// Сразу сбросим
	//mb_Invalidated = false;

	#ifdef _DEBUG
	bool bDataChanged = mb_DataChanged;
	#endif
	mb_DataChanged = false;

	int nStatusWidth = rcStatus.right - rcStatus.left + 1;
	int nStatusHeight = rcStatus.bottom - rcStatus.top + ((gpSet->isStatusBarFlags & csf_NoVerticalPad) ? 0 : 1);

	// Проверить клавиатуру
	IsKeyboardChanged();
	// И статусы окна
	IsWindowChanged();
	// Время, если показано
	IsTimeChanged();

	if (!mh_MemDC || (nStatusWidth != mn_BmpSize.cx) || (nStatusHeight != mn_BmpSize.cy))
	{
		if (mh_MemDC)
		{
			SelectObject(mh_MemDC, mb_OldBmp);
			DeleteObject(mh_Bmp);
			DeleteDC(mh_MemDC);
		}

		mh_MemDC = CreateCompatibleDC(hPaint);
		mh_Bmp = CreateCompatibleBitmap(hPaint, nStatusWidth, nStatusHeight);
		mb_OldBmp = (HBITMAP)SelectObject(mh_MemDC, mh_Bmp);

		mn_BmpSize.cx = nStatusWidth; mn_BmpSize.cy = nStatusHeight;
	}

	HDC hDrawDC = mh_MemDC;
	POINT ptUL = {0,0};

	bool lbSysColor = (gpSet->isStatusBarFlags & csf_SystemColors) == csf_SystemColors;
	bool lbFade = lbSysColor ? false : gpSet->isFadeInactive && !gpConEmu->isMeForeground(true);

	COLORREF crBack = lbSysColor ? GetSysColor(COLOR_3DFACE) : lbFade ? gpSet->GetFadeColor(gpSet->nStatusBarBack) : gpSet->nStatusBarBack;
	COLORREF crText = lbSysColor ? GetSysColor(COLOR_BTNTEXT) : lbFade ? gpSet->GetFadeColor(gpSet->nStatusBarLight) : gpSet->nStatusBarLight;
	COLORREF crDash = lbSysColor ? GetSysColor(COLOR_3DSHADOW) : lbFade ? gpSet->GetFadeColor(gpSet->nStatusBarDark) : gpSet->nStatusBarDark;

	HBRUSH hBr = CreateSolidBrush(crBack);
	//HPEN hLight = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
	//HPEN hShadow = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DDKSHADOW));
	HPEN hDash = CreatePen(PS_SOLID, 1, crDash);
	HPEN hOldP = (HPEN)SelectObject(hDrawDC, hDash);

	LOGFONT lf = {};
	lf.lfWeight = FW_DONTCARE;
	lf.lfCharSet = gpSet->nStatusFontCharSet;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
	wcscpy_c(lf.lfFaceName, gpSet->sStatusFontFace);
	// Do not use gpSetCls->EvalLogfontSizes, because we does not need to take zoom into account here
	lf.lfHeight = gpSetCls->EvalSize(gpSet->StatusBarFontHeight(), esf_Vertical|esf_CanUseUnits|esf_CanUseDpi);

	HFONT hFont = CreateFontIndirect(&lf);
	HFONT hOldF = (HFONT)SelectObject(hDrawDC, hFont);

	RECT rcFill = {ptUL.x, ptUL.y, ptUL.x+nStatusWidth, ptUL.y+nStatusHeight};

	#ifdef DUMP_STATUS_IMG
	if (bNeedDumpImage)
	{
		FillRect(hDrawDC, &rcFill, (HBRUSH)GetStockObject(BLACK_BRUSH));
	}
	#endif

	FillRect(hDrawDC, &rcFill, hBr);


	if (gpSet->isStatusBarFlags & csf_HorzDelim)
	{
		MoveToEx(hDrawDC, rcFill.left, rcFill.top, NULL);
		LineTo(hDrawDC, rcFill.right+1, rcFill.top);
	}


	CVConGuard VCon;
	CRealConsole* pRCon = NULL;
	CVirtualConsole* pVCon = (gpConEmu->GetActiveVCon(&VCon) >= 0) ? VCon.VCon() : NULL;
	if (pVCon)
		pRCon = pVCon->RCon();

	// Сформировать элементы
	size_t nDrawCount = 0;
	//size_t nID = 1;
	LPCWSTR pszTmp;
	CONSOLE_CURSOR_INFO ci = {};
	CONSOLE_SCREEN_BUFFER_INFO sbi = {};
	DEBUGTEST(RealBufferType tBuffer = rbt_Undefined);
	SIZE szTemp;
	int x = 0;

	if (pRCon)
	{
		pRCon->GetConsoleCursorInfo(&ci);
		pRCon->GetConsoleScreenBufferInfo(&sbi);
		DEBUGTEST(tBuffer = pRCon->GetActiveBufferType());
	}

	int nGapWidth = 2;
	int nDashWidth = 1;
	int nMinInfoWidth = 80;
	int nTotalWidth = nGapWidth;
	int iInfoID = -1;
	SIZE szVerSize = {};

	GetTextExtentPoint32(hDrawDC, ms_ConEmuBuild, lstrlen(ms_ConEmuBuild), &szVerSize);

	//while (nID <= ces_Last)
	for (size_t ii = 0; ii < countof(gStatusCols); ii++)
	{
		CEStatusItems nID = gStatusCols[ii].nID;
		_ASSERTE(nID < countof(m_Values));

		// Info - показываем всегда!
		if (nID != csi_Info)
		{
			_ASSERTE(nID<countof(gpSet->isStatusColumnHidden));
			if (gpSet->isStatusColumnHidden[nID])
				continue; // просили не показывать
			if (nID == csi_ConsoleTitle)
				continue; // если выбрано - то показывается как csi_Info
			if ((nID == csi_Transparency) && !gpSet->isTransparentAllowed())
				continue; // смысла не имеет
			if ((nID == csi_SyncInside) && !gpConEmu->mp_Inside)
				continue; // смысла не имеет
		}

		m_Items[nDrawCount].nID = nID;

		switch (nID)
		{
			case csi_Info:
			{
				pszTmp =
					(m_ClickedItemDesc == csi_Info && !mb_InSetupMenu) ? L"Right click to show System Menu" :
					((mb_InSetupMenu && m_ClickedItemDesc == csi_Info)
						|| (m_ClickedItemDesc > csi_Info && m_ClickedItemDesc < csi_Last))
					? m_Values[m_ClickedItemDesc].sHelp :
					ms_Status[0] ? ms_Status :
					pRCon ? pRCon->GetConStatus() : NULL;

				if (pszTmp && *pszTmp)
				{
					lstrcpyn(m_Items[nDrawCount].sText, pszTmp, countof(m_Items[nDrawCount].sText));
				}
				else if (pRCon)
				{
					if (!gpSet->isStatusColumnHidden[csi_ConsoleTitle])
					{
						lstrcpyn(m_Items[nDrawCount].sText, pRCon->GetTitle(), countof(m_Items[nDrawCount].sText));
					}
					else if (!gpSet->isStatusColumnHidden[csi_ActiveProcess])
					{
						if (!LoadActiveProcess(pRCon, m_Items[nDrawCount].sText, countof(m_Items[nDrawCount].sText)))
							lstrcpyn(m_Items[nDrawCount].sText, gsReady, countof(m_Items[nDrawCount].sText));
					}
					else
					{
						lstrcpyn(m_Items[nDrawCount].sText, L"Ready", countof(m_Items[nDrawCount].sText));
					}
				}
				else
				{
					wcscpy_c(m_Items[nDrawCount].sText, L" ");
				}
				wcscpy_c(m_Items[nDrawCount].szFormat, L"Ready");

				break;
			} // csi_Info

			case csi_ConsoleTitle:
				_ASSERTE(FALSE && "Must be processed as first column - csi_Info");
				break;

			case csi_ActiveProcess:
				if (gpSet->isStatusColumnHidden[csi_ConsoleTitle])
					m_Items[nDrawCount].sText[0] = 0; // Уже показано в первой колонке
				else
					LoadActiveProcess(pRCon, m_Items[nDrawCount].sText, countof(m_Items[nDrawCount].sText));
				break;

			case csi_NewVCon:
				wcscpy_c(m_Items[nDrawCount].sText, L"[+]");
				wcscpy_c(m_Items[nDrawCount].szFormat, L"[+]");
				break;

			case csi_SyncInside:
				wcscpy_c(m_Items[nDrawCount].sText, L"Sync");
				wcscpy_c(m_Items[nDrawCount].szFormat, L"Sync");
				break;

			case csi_CapsLock:
				wcscpy_c(m_Items[nDrawCount].sText, L"CAPS");
				wcscpy_c(m_Items[nDrawCount].szFormat, L"CAPS");
				break;
			case csi_NumLock:
				wcscpy_c(m_Items[nDrawCount].sText, L"NUM");
				wcscpy_c(m_Items[nDrawCount].szFormat, L"NUM");
				break;
			case csi_ScrollLock:
				wcscpy_c(m_Items[nDrawCount].sText, L"SCRL");
				wcscpy_c(m_Items[nDrawCount].szFormat, L"SCRL");
				break;
			case csi_InputLocale:
				// чтобы не задавали вопросов, нафига дублируется.
				if (LOWORD((DWORD)mhk_Locale) == HIWORD((DWORD)mhk_Locale))
				{
					_wsprintf(m_Items[nDrawCount].sText, SKIPLEN(countof(m_Items[nDrawCount].sText)-1) L"%04X", LOWORD((DWORD)mhk_Locale));
					wcscpy_c(m_Items[nDrawCount].szFormat, L"FFFF");
				}
				else
				{
					_wsprintf(m_Items[nDrawCount].sText, SKIPLEN(countof(m_Items[nDrawCount].sText)-1) L"%08X", (DWORD)mhk_Locale);
					wcscpy_c(m_Items[nDrawCount].szFormat, L"FFFFFFFF");
				}
				break;
			case csi_KeyHooks:
				wcscpy_c(m_Items[nDrawCount].sText,
					(gpConEmu->IsKeyboardHookRegistered() || gpSet->isKeyboardHooks(false, true))
					? L"KH" : L"––");
				wcscpy_c(m_Items[nDrawCount].szFormat, L"XX");
				break;

			case csi_WindowStyle:
				_wsprintf(m_Items[nDrawCount].sText, SKIPLEN(countof(m_Items[nDrawCount].sText)-1) L"%08X", mn_Style);
				wcscpy_c(m_Items[nDrawCount].szFormat, L"FFFFFFFF");
				break;
			case csi_WindowStyleEx:
				_wsprintf(m_Items[nDrawCount].sText, SKIPLEN(countof(m_Items[nDrawCount].sText)-1) L"%08X", mn_ExStyle);
				wcscpy_c(m_Items[nDrawCount].szFormat, L"FFFFFFFF");
				break;

			case csi_Zoom:
				_wsprintf(m_Items[nDrawCount].sText, SKIPLEN(countof(m_Items[nDrawCount].sText)-1) L"%i%%", gpSetCls->GetZoom());
				wcscpy_c(m_Items[nDrawCount].szFormat, L"200%");
				break;
			case csi_DPI:
				_wsprintf(m_Items[nDrawCount].sText, SKIPLEN(countof(m_Items[nDrawCount].sText)-1) L"%i", gpSetCls->QueryDpi());
				wcscpy_c(m_Items[nDrawCount].szFormat, L"999");
				break;

			case csi_HwndFore:
				_wsprintf(m_Items[nDrawCount].sText, SKIPLEN(countof(m_Items[nDrawCount].sText)-1) L"x%08X[%u]", (DWORD)mh_Fore, mn_ForePID);
				wcscpy_c(m_Items[nDrawCount].szFormat, L"xFFFFFFFF[99999]");
				m_Values[nID].sHelp = ms_ForeInfo;
				break;
			case csi_HwndFocus:
				_wsprintf(m_Items[nDrawCount].sText, SKIPLEN(countof(m_Items[nDrawCount].sText)-1) L"x%08X[%u]", (DWORD)mh_Focus, mn_FocusPID);
				wcscpy_c(m_Items[nDrawCount].szFormat, L"xFFFFFFFF[99999]");
				m_Values[nID].sHelp = ms_FocusInfo;
				break;

			default:
				wcscpy_c(m_Items[nDrawCount].sText, m_Values[nID].sText);
				wcscpy_c(m_Items[nDrawCount].szFormat, m_Values[nID].szFormat);
		}

		m_Items[nDrawCount].nTextLen = lstrlen(m_Items[nDrawCount].sText);

		if ((nID != csi_Info) && !m_Items[nDrawCount].nTextLen)
		{
			continue;
		}

		if (!GetTextExtentPoint32(hDrawDC, m_Items[nDrawCount].sText, m_Items[nDrawCount].nTextLen, &m_Items[nDrawCount].TextSize) || (m_Items[nDrawCount].TextSize.cx <= 0))
		{
			if (nID != csi_Info)
				continue;
		}
		if (*m_Items[nDrawCount].szFormat && GetTextExtentPoint32(hDrawDC, m_Items[nDrawCount].szFormat, lstrlen(m_Items[nDrawCount].szFormat), &szTemp))
		{
			m_Items[nDrawCount].TextSize.cx = max(m_Items[nDrawCount].TextSize.cx, szTemp.cx);
		}

		if (nID == csi_Info)
		{
			m_Items[nDrawCount].TextSize.cx += szVerSize.cx; // Информация о версии
			if (m_Items[nDrawCount].TextSize.cx < nMinInfoWidth)
				m_Items[nDrawCount].TextSize.cx = nMinInfoWidth;
		}

		//if (nDrawCount) // перед вторым и далее - добавить разделитель
		//	nTotalWidth += 2*nGapWidth + nDashWidth;

		if (nID == csi_Info) // csi_Info добавим потом
			iInfoID = (int)nDrawCount;
		//else
		//	nTotalWidth += m_Items[nDrawCount].TextSize.cx;

		m_Items[nDrawCount++].bShow = TRUE;

		//Next:
		//	nID = nID<<1;
	}

	if (iInfoID != 0)
	{
		_ASSERTE(iInfoID == 0);
		goto wrap;
	}

	//nTotalWidth += nGapWidth;

	// Сброс неиспользуемых ячеек
	for (size_t i = nDrawCount; i < countof(m_Items); i++)
	{
		m_Items[i].bShow = FALSE;
	}



	if (nDrawCount == 1)
	{
		m_Items[0].TextSize.cx = min((nStatusWidth - 2*nGapWidth - 1),m_Items[0].TextSize.cx);
	}
	else
	{
		// Подсчет допустимой ширины
		nTotalWidth = nMinInfoWidth + 2*nGapWidth + nDashWidth;

		for (size_t i = 1; i < nDrawCount; i++)
		{
			if (m_Items[i].bShow)
			{
				if ((nTotalWidth+m_Items[i].TextSize.cx+2*nGapWidth) > nStatusWidth)
				{
					m_Items[i].bShow = FALSE;
					// Но продолжим, может следующая ячейки будет уже и "влезет"
				}
				else
				{
					// Раз этот элемент показан
					nTotalWidth += (m_Items[i].TextSize.cx + 2*nGapWidth + nDashWidth);
				}
			}
			// -- don't break, may be further columns!
			//else
			//{
			//	break; // дальше смысла нет - ячейки кончились
			//}
		}


		//if (nMinInfoWidth < (nStatusWidth - nTotalWidth))
		//{
		//	//int nMaxInfoWidth = nStatusWidth - nTotalWidth - nGapWidth - 1;
		//	//m_Items[0].TextSize.cx = max(nMinInfoWidth,nMaxInfoWidth);
		//	m_Items[0].TextSize.cx = nStatusWidth - nTotalWidth;
		//}

		m_Items[0].TextSize.cx = max(nMinInfoWidth,(nStatusWidth - nTotalWidth + nMinInfoWidth));
	}



	SetTextColor(hDrawDC, crText);
	SetBkColor(hDrawDC, crBack);
	SetBkMode(hDrawDC, TRANSPARENT);

	x = ptUL.x;
	for (size_t i = 0; i < nDrawCount; i++)
	{
		BOOL bDrawRC = FALSE;
		DWORD nDrawErr = 0;

		if (!m_Items[i].bShow || (m_Items[i].nID >= csi_Last) || (i && !m_Items[i].nID))
			continue;

		RECT rcField = {x, ptUL.y, x+m_Items[i].TextSize.cx+2*nGapWidth, ptUL.y+nStatusHeight};
		if (rcField.right > nStatusWidth)
		{
			_ASSERTE(rcField.right <= nStatusWidth); // должно было отсечься на предыдущем цикле!
			break;
		}
		if ((i+1) == nDrawCount)
			rcField.right = nStatusWidth;


		// Запомнить, чтобы клики обрабатывать...
		m_Items[i].rcClient = MakeRect(rcField.left+rcStatus.left, rcStatus.top, rcField.right+rcStatus.left, rcStatus.bottom);

		// "Resize mark" отрисовывается вручную
		if (m_Items[i].nID == csi_SizeGrip)
		{
			int nW = (rcField.bottom - rcField.top);
			if (nW > 0)
			{
				int nShift = (nW - 1) / 3;
				int nX = rcField.right - nW - 1;
				int nY = rcField.top-1;
				int nR = rcField.right;
				MoveToEx(hDrawDC, nX, rcField.bottom, NULL);
				LineTo(hDrawDC, nR, nY);
				nX += nShift; nY += nShift;
				MoveToEx(hDrawDC, nX, rcField.bottom, NULL);
				LineTo(hDrawDC, nR, nY);
				nX += nShift; nY += nShift;
				MoveToEx(hDrawDC, nX, rcField.bottom, NULL);
				LineTo(hDrawDC, nR, nY);
			}
		}
		else
		{
			switch (m_Items[i].nID)
			{
			case csi_SyncInside:
				SetTextColor(hDrawDC, (gpConEmu->mp_Inside && gpConEmu->mp_Inside->mb_InsideSynchronizeCurDir) ? crText : crDash);
				break;
			case csi_CapsLock:
				SetTextColor(hDrawDC, mb_Caps ? crText : crDash);
				break;
			case csi_NumLock:
				SetTextColor(hDrawDC, mb_Num ? crText : crDash);
				break;
			case csi_ScrollLock:
				SetTextColor(hDrawDC, mb_Scroll ? crText : crDash);
				break;
			case csi_KeyHooks:
				SetTextColor(hDrawDC, mb_KeyHooks ? crText : crDash);
				break;
			default:
				;
			}

			int iTopPad = (gpSet->isStatusBarFlags & csf_HorzDelim) ? 1 : 0;
			int iBottomPad = ((gpSet->isStatusBarFlags & csf_NoVerticalPad) && !(gpSet->isStatusBarFlags & csf_HorzDelim)) ? 0 : 1;
			RECT rcText = {rcField.left+1, rcField.top+iTopPad, rcField.right-1, rcField.bottom-iBottomPad};

			#ifdef DUMP_STATUS_IMG
			if (bNeedDumpImage)
			{
				if (hDrawDC != hPaint)
				{
					FillRect(hDrawDC, &rcText, (HBRUSH)GetStockObject(BLACK_BRUSH));
					DumpImage(mh_MemDC, mh_Bmp, nStatusWidth, nStatusHeight, pszDumpImgName);
					FillRect(hDrawDC, &rcText, hBr);

					SelectObject(hDrawDC, hFont);
					SetTextColor(hDrawDC, crText);
					SetBkColor(hDrawDC, crBack);
					SetBkMode(hDrawDC, OPAQUE);
				}
			}
			#endif

			if (!i && szVerSize.cx > 0)
			{
				bDrawRC = DrawText(hDrawDC, ms_ConEmuBuild, lstrlen(ms_ConEmuBuild), &rcText,
					DT_RIGHT|DT_NOPREFIX|DT_SINGLELINE|/*DT_VCENTER|*/DT_END_ELLIPSIS);
				rcText.right -= szVerSize.cx;
			}

			bDrawRC = DrawText(hDrawDC, m_Items[i].sText, m_Items[i].nTextLen, &rcText,
				((m_Items[i].nID == csi_Info) ? DT_LEFT : DT_CENTER)
				|DT_NOPREFIX|DT_SINGLELINE|/*DT_VCENTER|*/DT_END_ELLIPSIS);

			if (bDrawRC <= 0)
				nDrawErr = GetLastError();

			#ifdef DUMP_STATUS_IMG
			if (bNeedDumpImage)
			{
				if (hDrawDC != hPaint)
				{
					DumpImage(mh_MemDC, mh_Bmp, nStatusWidth, nStatusHeight, pszDumpImgName);
				}
			}
			#endif

			switch (m_Items[i].nID)
			{
			case csi_SyncInside:
			case csi_CapsLock:
			case csi_NumLock:
			case csi_ScrollLock:
			case csi_KeyHooks:
				SetTextColor(hDrawDC, crText);
				break;
			default:
				;
			}

			if ((gpSet->isStatusBarFlags & csf_VertDelim) && ((i+1) < nDrawCount))
			{
				MoveToEx(hDrawDC, rcField.right, rcField.top, NULL);
				LineTo(hDrawDC, rcField.right, rcField.bottom+1);
			}

			UNREFERENCED_PARAMETER(nDrawErr);
		}

		#ifdef DUMP_STATUS_IMG
		if (hDrawDC == hPaint)
		{
			GdiFlush();
		}
		#endif

		x = rcField.right + nDashWidth;
	}


wrap:
	#ifdef DUMP_STATUS_IMG
	if (bNeedDumpImage)
	{
		if (hDrawDC != hPaint)
			DumpImage(mh_MemDC, mh_Bmp, nStatusWidth, nStatusHeight, pszDumpImgName);
		DumpImage(hPaint, NULL, rcStatus.left, rcStatus.top, nStatusWidth, nStatusHeight, pszDumpImgDstName);
	}
	#endif

	if (hDrawDC != hPaint)
	{
		DWORD nErr = 0;
		BOOL bPaintOK;
		#if 0 //def _DEBUG
		HBRUSH hOldB = (HBRUSH)SelectObject(hPaint, (HBRUSH)GetStockObject(DKGRAY_BRUSH));
		HPEN hOldP = (HPEN)SelectObject(hPaint, (HPEN)GetStockObject(WHITE_PEN));
		Rectangle(hPaint, rcStatus.left, rcStatus.top, rcStatus.left+nStatusWidth, rcStatus.top+nStatusHeight);
		SelectObject(hPaint, hOldB);
		SelectObject(hPaint, hOldP);
		bPaintOK = TRUE;
		#else
		bPaintOK = BitBlt(hPaint, rcStatus.left, rcStatus.top, nStatusWidth, nStatusHeight, hDrawDC, 0,0, SRCCOPY);
		#endif
		if (!bPaintOK)
		{
			#ifdef _DEBUG
			nErr = GetLastError();
			// ассерт может возникнуть в RemoteDesktop пока был запрос на UAC
			_ASSERTE(bPaintOK || !gpConEmu->session.Connected());
			bPaintOK = FALSE;
			#endif
		}
	}


	SelectObject(hDrawDC, hOldF);
	SelectObject(hDrawDC, hOldP);

	DeleteObject(hBr);
	DeleteObject(hDash);
	DeleteObject(hFont);

	if (gpSet->isStatusColumnHidden[csi_SizeGrip]
		|| !GetStatusBarItemRect(csi_SizeGrip, &mrc_LastResizeCol))
	{
		ZeroStruct(mrc_LastResizeCol);
	}

	mrc_LastStatus = rcStatus;
	mn_LastPaintTick = GetTickCount();
}

void CStatus::SetStatus(LPCWSTR asStatus)
{
	lstrcpyn(ms_Status, asStatus ? asStatus : L"", countof(ms_Status));

	if (ms_Status[0] && gpSetCls->isAdvLogging)
	{
		gpConEmu->LogString(ms_Status);
	}

	UpdateStatusBar(true, false);
}

// true = обновлять строго, сменился шрифт, размер, или еще что...
void CStatus::UpdateStatusBar(bool abForce /*= false*/, bool abRepaintNow /*= false*/)
{
	if (!gpSet->isStatusBarShow || !ghWnd)
		return;

	mb_DataChanged	= mb_DataChanged | abForce;

	//if (mb_Invalidated)
	//	return; // уже дернули

	if (!abForce)
	{
		// Чтобы не было слишком частой отрисовки и нагрузки на систему
		DWORD nDelta = (GetTickCount() - mn_LastPaintTick);
		if (nDelta < STATUS_PAINT_DELAY)
			return; // обновится по следующему таймеру
	}

	RECT rcInvalidated = {};

	InvalidateStatusBar(&rcInvalidated);

	if (abRepaintNow)
	{
		RedrawWindow(ghWnd, &rcInvalidated, NULL, RDW_INTERNALPAINT|RDW_NOERASE|RDW_NOFRAME|RDW_UPDATENOW|RDW_VALIDATE);
	}
}

void CStatus::InvalidateStatusBar(LPRECT rcInvalidated /*= NULL*/)
{
	if (gpConEmu->isIconic())
		return;

	RECT rcClient = {};
	if (!GetStatusBarClientRect(&rcClient))
		return;

	// Invalidate вызывается не только при изменениях, но
	// и по таймеру, чтобы гарантировать актуальность данных
	// (не все данные хранятся в объекте, например CAPS/NUM/SCRL нужно проверять)

	gpConEmu->Invalidate(&rcClient, FALSE);

	if (rcInvalidated)
	{
		*rcInvalidated = rcClient;
	}
}

void CStatus::OnTimer()
{
	if (!gpSet->isStatusBarShow)
		return;

	// Если сейчас нет меню, и был показан hint для колонки -
	// проверить позицию мышки
	if ((m_ClickedItemDesc != csi_Last) && !mb_InPopupMenu)
	{
		if (gpConEmu->isSizing())
		{
			SetClickedItemDesc(csi_Last);
		}
		else
		{
			POINT ptCurClient = {}; GetCursorPos(&ptCurClient);
			MapWindowPoints(NULL, ghWnd, &ptCurClient, 1);
			LRESULT l;
			ProcessStatusMessage(ghWnd, WM_MOUSEMOVE, 0, 0, ptCurClient, l);
		}
	}

	if (/*!mb_Invalidated &&*/ !mb_DataChanged && IsKeyboardChanged())
		mb_DataChanged = true; // Изменения в клавиатуре

	if (!mb_DataChanged && IsWindowChanged())
		mb_DataChanged = true; // измения в стилях или текущем фокусе

	if (!mb_DataChanged && IsTimeChanged())
		mb_DataChanged = true; // Обновить время на статусе

	if (mb_DataChanged /*&& !mb_Invalidated*/)
		UpdateStatusBar(true);
}

void CStatus::SetClickedItemDesc(CEStatusItems anID)
{
	if (m_ClickedItemDesc != anID)
	{
		if (((anID >= csi_Info && anID < csi_Last) /*|| (mb_InSetupMenu && anID == csi_Info)*/)
			&& m_Values[anID].sHelp)
			m_ClickedItemDesc = anID;
		else
			m_ClickedItemDesc = csi_Last;

		UpdateStatusBar(true);
	}
}

bool CStatus::IsResizeAllowed()
{
	if (gpConEmu->mp_Inside)
		return false;

	if (!gpSet->isStatusBarShow
		|| gpSet->isStatusColumnHidden[csi_SizeGrip])
	{
		return false;
	}

	if (gpConEmu->GetWindowMode() != wmNormal)
	{
		return false;
	}

	return true;
}

bool CStatus::IsCursorOverResizeMark(POINT ptCurClient)
{
	_ASSERTE(this);

	if (!IsResizeAllowed())
		return false;

	// Cursor over "resize mark"?
	if (!PtInRect(&mrc_LastResizeCol, ptCurClient))
		return false;

	return true;
}

bool CStatus::IsStatusResizing()
{
	_ASSERTE(this);
	if (!gpSet->isStatusBarShow)
		return false; // Нет статуса - нет ресайза

	return mb_StatusResizing;
}

bool CStatus::ProcessStatusMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, POINT ptCurClient, LRESULT& lResult)
{
	if (!gpSet->isStatusBarShow)
		return false; // Разрешить дальнейшую обработку

	lResult = 0;

	// Cursor over "resize mark"?
	if (IsResizeAllowed()
		&& (mb_StatusResizing || PtInRect(&mrc_LastResizeCol, ptCurClient)))
	{
		POINT ptScr = {};

		switch (uMsg)
		{
		case WM_LBUTTONDOWN:
			DEBUGSTRSIZE(L"Starting resize from status bar grip");
			GetCursorPos(&mpt_StatusResizePt);
			GetWindowRect(ghWnd, &mrc_StatusResizeRect);
			mb_StatusResizing = true;
			SetCapture(ghWnd);
			gpConEmu->BeginSizing(true);
			break;
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			if (mb_StatusResizing && GetCursorPos(&ptScr))
			{
				DEBUGSTRSIZE(L"Change size from status bar grip");

				int nWidth = (mrc_StatusResizeRect.right - mrc_StatusResizeRect.left) + (ptScr.x - mpt_StatusResizePt.x);
				int nHeight = (mrc_StatusResizeRect.bottom - mrc_StatusResizeRect.top) + (ptScr.y - mpt_StatusResizePt.y);

				RECT rcNew = {mrc_StatusResizeRect.left, mrc_StatusResizeRect.top, mrc_StatusResizeRect.left + nWidth, mrc_StatusResizeRect.top + nHeight};
				gpConEmu->OnSizing(WMSZ_BOTTOMRIGHT, (LPARAM)&rcNew);

				SetWindowPos(ghWnd, NULL,
					rcNew.left, rcNew.top, rcNew.right - rcNew.left, rcNew.bottom - rcNew.top,
					SWP_NOZORDER|SWP_NOCOPYBITS);
			}

			if (uMsg == WM_LBUTTONUP)
			{
				SetCapture(NULL);
				gpConEmu->EndSizing();
				mb_StatusResizing = false;
				DEBUGSTRSIZE(L"Resize from status bar grip finished");
			}
			break;
		case WM_SETCURSOR: // не приходит
			// Stop further processing
			SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
			lResult = TRUE;
			return true;
		}
	}

	if (gpConEmu->isSizing() || !PtInRect(&mrc_LastStatus, ptCurClient))
	{
		//bool bWasClick = mb_WasClick;
		//mb_WasClick = false;
		if (m_ClickedItemDesc != csi_Last)
		{
			m_ClickedItemDesc = csi_Last;
			UpdateStatusBar(true);
		}
		return false; //bWasClick;
	}

	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_MOUSEMOVE:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		{
			RECT rcClient = {};
			bool bFound = false;
			for (size_t i = 0; i < countof(m_Items); i++)
			{
				if (m_Items[i].bShow && PtInRect(&m_Items[i].rcClient, ptCurClient))
				{
					bFound = true;

					// правый клик был на колонке "Info"
					if (i == 0)
					{
						// и координата не правее середины колонки
						// (ну так, на всякий случай, чтобы и статусное меню можно было показать)
						if (ptCurClient.x <= min(80,(m_Items[0].rcClient.right/2)))
						{
							_ASSERTE(m_Items[i].nID == csi_Info);
							SetClickedItemDesc(csi_Info);
						}
						else
						{
							SetClickedItemDesc(csi_Last);
						}
					}
					else
					{
						SetClickedItemDesc(m_Items[i].nID);
					}
					rcClient = m_Items[i].rcClient;
					break;
				}
			} // for (size_t i = 0; i < countof(m_Items); i++)

			if (!bFound)
			{
				SetClickedItemDesc(csi_Last);
			}

			bool bNeedUpdate = true;

			#ifdef _DEBUG
			static POINT ptLast = {};
			if (uMsg == WM_MOUSEMOVE)
			{
				POINT ptCur = {}; GetCursorPos(&ptCur);
				if (ptCur.x != ptLast.x || ptCur.y != ptLast.y)
				{
					ptLast = ptCur;
				}
				else
				{
					bNeedUpdate = false;
				}
			}
			#endif

			if (bNeedUpdate)
			{
				UpdateStatusBar(true);
			}

			if ((uMsg == WM_LBUTTONDOWN) || (uMsg == WM_LBUTTONDBLCLK))
			{
				MapWindowPoints(ghWnd, NULL, (LPPOINT)&rcClient, 2);

				switch (m_ClickedItemDesc)
				{
				case csi_ActiveVCon:
					if (uMsg == WM_LBUTTONDOWN)
						ShowVConMenu(MakePoint(rcClient.left, rcClient.top));
					break;
				case csi_NewVCon:
					if (uMsg == WM_LBUTTONDOWN)
						gpConEmu->mp_Menu->OnNewConPopupMenu((POINT*)&rcClient, TPM_LEFTALIGN|TPM_BOTTOMALIGN, isPressed(VK_SHIFT));
					break;
				case csi_Transparency:
					if (uMsg == WM_LBUTTONDOWN)
						ShowTransparencyMenu(MakePoint(rcClient.right, rcClient.top));
					break;
				case csi_Zoom:
					if (uMsg == WM_LBUTTONDOWN)
						ShowZoomMenu(MakePoint(rcClient.right, rcClient.top));
					break;
				case csi_SyncInside:
					if (gpConEmu->mp_Inside)
					{
						gpConEmu->mp_Inside->mb_InsideSynchronizeCurDir = !gpConEmu->mp_Inside->mb_InsideSynchronizeCurDir;
					}
					break;
				case csi_CapsLock:
					keybd_event(VK_CAPITAL, 0, 0, 0);
					keybd_event(VK_CAPITAL, 0, KEYEVENTF_KEYUP, 0);
					break;
				case csi_NumLock:
					keybd_event(VK_NUMLOCK, 0, 0, 0);
					keybd_event(VK_NUMLOCK, 0, KEYEVENTF_KEYUP, 0);
					break;
				case csi_ScrollLock:
					keybd_event(VK_SCROLL, 0, 0, 0);
					keybd_event(VK_SCROLL, 0, KEYEVENTF_KEYUP, 0);
					break;
				default:
					;
				}
			}
			else if (uMsg == WM_RBUTTONUP)
			{
				MapWindowPoints(ghWnd, NULL, (LPPOINT)&rcClient, 2);

				switch (m_ClickedItemDesc)
				{
				case csi_ActiveVCon:
				case csi_NewVCon:
					//gpConEmu->mp_TabBar->OnNewConPopupMenu((POINT*)&rcClient, TPM_LEFTALIGN|TPM_BOTTOMALIGN);
					{
						CVConGuard VCon;
						if (CVConGroup::GetActiveVCon(&VCon) >= 0)
						{
							gpConEmu->mp_Menu->ShowPopupMenu(VCon.VCon(), MakePoint(rcClient.left,rcClient.top), TPM_LEFTALIGN|TPM_BOTTOMALIGN);
						}
					}
					break;
				default:
					ShowStatusSetupMenu();
				}
			}
		}
		break; // WM_LBUTTONDOWN

	//case WM_LBUTTONUP:
	//	if (mb_MouseCaptured)
	//	{
	//		ReleaseCapture();
	//		mb_WasClick = false;
	//		if (ms_ClickedItemDesc[0])
	//		{
	//			ms_ClickedItemDesc[0] = 0;
	//			UpdateStatusBar(true);
	//		}
	//		return true;
	//	}
	//	break;

	}

	// Значит курсор над статусом
	return true;
}

void CStatus::ShowStatusSetupMenu()
{
	POINT ptCur = {}; ::GetCursorPos(&ptCur);
	POINT ptClient = ptCur;
	MapWindowPoints(NULL, ghWnd, &ptClient, 1);

	int nClickedID = -1;

	// m_Items содержит только видимые элементы, поэтому индексы не совпадают, надо искать
	for (size_t j = 0; j < countof(m_Items); j++)
	{
		if (m_Items[j].bShow && PtInRect(&m_Items[j].rcClient, ptClient))
		{
			nClickedID = m_Items[j].nID;
			break;
		}
	}

	// Если правый клик был на колонке "Info"
	if ((nClickedID == 0)
		// и координата не правее середины колонки
		// (ну так, на всякий случай, чтобы и статусное меню можно было показать)
		&& (ptClient.x <= min(80,(m_Items[0].rcClient.right/2))))
	{
		gpConEmu->mp_Menu->ShowSysmenu(ptCur.x, ptCur.y, TPM_BOTTOMALIGN);
		return;
	}


	if (isSettingsOpened(IDD_SPG_STATUSBAR))
		return;

	mb_InPopupMenu = true;

	HMENU hPopup = CreatePopupMenu();

	int nBreak = (countof(m_Items) + 1) / 2;

	for (int i = 2; i <= (int)countof(gStatusCols); i++)
	{
		DWORD nBreakFlag = 0;
		if ((nBreak--) < 0)
		{
			nBreak = (countof(m_Items) + 1) / 2;
			nBreakFlag = MF_MENUBREAK;
		}

		//if (i == 1)
		//{
		//	AppendMenu(hPopup, MF_STRING|(gpSet->isStatusBarShow ? MF_CHECKED : MF_UNCHECKED), i, gStatusCols[i-1].sName);
		//	AppendMenu(hPopup, MF_SEPARATOR, 0, L"");
		//}
		//else
		{
			AppendMenu(hPopup, MF_STRING|nBreakFlag|((gpSet->isStatusColumnHidden[gStatusCols[i-1].nID]) ? MF_UNCHECKED : MF_CHECKED), i, gStatusCols[i-1].sName);
		}

		// m_Items содержит только видимые элементы, поэтому индексы не совпадают
		if (nClickedID == gStatusCols[i-1].nID)
		{
			MENUITEMINFO mi = {sizeof(mi), MIIM_STATE|MIIM_ID};
			mi.wID = i;
			GetMenuItemInfo(hPopup, i, FALSE, &mi);
			mi.fState |= MF_DEFAULT;
			SetMenuItemInfo(hPopup, i, FALSE, &mi);
		}
	}

	AppendMenu(hPopup, MF_SEPARATOR, 0, L"");
	AppendMenu(hPopup, MF_STRING|(gpSet->isStatusBarShow ? MF_CHECKED : MF_UNCHECKED), 1, gStatusCols[0].sName);
	AppendMenu(hPopup, MF_STRING, (int)countof(gStatusCols)+1, L"Setup...");

	while (true)
	{
		mb_InSetupMenu = true;
		// Popup menu with columns list
		int nCmd = gpConEmu->mp_Menu->trackPopupMenu(tmp_StatusBarCols, hPopup, TPM_BOTTOMALIGN|TPM_LEFTALIGN|TPM_RETURNCMD, ptCur.x, ptCur.y, ghWnd);
		mb_InSetupMenu = false;

		if (nCmd == ((int)countof(gStatusCols)+1))
		{
			CSettings::Dialog(IDD_SPG_STATUSBAR);
			break;
		}
		else if ((nCmd >= 1) && (nCmd <= (int)countof(gStatusCols)))
		{
			if (nCmd == 1)
			{
				gpConEmu->StatusCommand(csc_ShowHide);
			}
			else
			{
				int nIdx = gStatusCols[nCmd-1].nID;
				if (nIdx >= 0 && nIdx < countof(gpSet->isStatusColumnHidden))
				{
					gpSet->isStatusColumnHidden[nIdx] = !gpSet->isStatusColumnHidden[nIdx];

					MENUITEMINFO mi = {sizeof(mi), MIIM_STATE|MIIM_ID};
					mi.wID = nCmd;
					GetMenuItemInfo(hPopup, nCmd, FALSE, &mi);
					if (gpSet->isStatusColumnHidden[nIdx])
						mi.fState &= ~MF_CHECKED;
					else
						mi.fState |= MF_CHECKED;
					SetMenuItemInfo(hPopup, nCmd, FALSE, &mi);

					UpdateStatusBar(true);
				}
			}

			if (ghOpWnd && IsWindow(ghOpWnd))
			{
				_ASSERTE(L"Refresh status page!");
			}
			else
			{
				gpSet->mb_StatusSettingsWasChanged = true;
			}
		}

		if (nCmd <= 1)
		{
			break;
		}
	}

	// Done
	DestroyMenu(hPopup);
	mb_InPopupMenu = false;
}

bool CStatus::OnDataChanged(CEStatusItems *ChangedID, size_t Count, bool abForceOnChange /*= false*/)
{
	//if (!mb_Invalidated && IsKeyboardChanged())
	//{
	//	// Изменения в клавиатуре - отрисовать сразу
	//	UpdateStatusBar(true);
	//}

	for (size_t k = 0; k < Count; k++)
	{
		for (size_t i = 0; i < countof(m_Items); i++)
		{
			if ((m_Items[i].nID == ChangedID[k]) && m_Items[i].bShow)
			{
				if (lstrcmp(m_Items[i].sText, m_Values[ChangedID[k]].sText) != 0)
				{
					UpdateStatusBar(abForceOnChange);
					return true;
				}
			}
		}
	}

	return false;
}

void CStatus::OnWindowReposition(const RECT *prcNew)
{
	if (!gpSet->isStatusBarShow)
		return;

	RECT rcTmp = {};
	if (prcNew)
	{
		rcTmp = *prcNew;
	}
	else
	{
		if (gpConEmu->isIconic())
			return; // все равно ничего не видно

		//-- GetWindowRect может вернуть еще минимизированные размеры
		//GetWindowRect(ghWnd, &rcTmp);
		rcTmp = gpConEmu->CalcRect(CER_MAIN);
	}

	// csi_ConEmuHWND
	_wsprintf(m_Values[csi_ConEmuHWND].sText, SKIPLEN(countof(m_Values[csi_ConEmuHWND].sText)-1)
		L"x%08X(%u)", (DWORD)ghWnd, (DWORD)ghWnd);
	wcscpy_c(m_Values[csi_ConEmuHWND].szFormat, m_Values[csi_ConEmuHWND].sText);

	// csi_WindowPos
	_wsprintf(m_Values[csi_WindowPos].sText, SKIPLEN(countof(m_Values[csi_WindowPos].sText)-1)
		L"(%i,%i)-(%i,%i)", rcTmp.left, rcTmp.top, rcTmp.right, rcTmp.bottom);
	wcscpy_c(m_Values[csi_WindowPos].szFormat, m_Values[csi_WindowPos].sText /*L"(999,999)-(9999,9999)"*/);

	// csi_WindowSize:
	_wsprintf(m_Values[csi_WindowSize].sText, SKIPLEN(countof(m_Values[csi_WindowSize].sText)-1)
		L"%ix%i", (rcTmp.right-rcTmp.left), (rcTmp.bottom-rcTmp.top));
	wcscpy_c(m_Values[csi_WindowSize].szFormat, m_Values[csi_WindowSize].sText/*L"9999x9999"*/);

	// csi_WindowClient
	RECT rcClient = gpConEmu->CalcRect(CER_MAINCLIENT, rcTmp, CER_MAIN);
	_wsprintf(m_Values[csi_WindowClient].sText, SKIPLEN(countof(m_Values[csi_WindowClient].sText)-1)
		L"%ix%i", (rcClient.right-rcClient.left), (rcClient.bottom-rcClient.top));
	wcscpy_c(m_Values[csi_WindowClient].szFormat, m_Values[csi_WindowClient].sText/*L"9999x9999"*/);

	// csi_WindowClient
	RECT rcWork = gpConEmu->CalcRect(CER_WORKSPACE, rcTmp, CER_MAIN);
	_wsprintf(m_Values[csi_WindowWork].sText, SKIPLEN(countof(m_Values[csi_WindowWork].sText)-1)
		L"%ix%i", (rcWork.right-rcWork.left), (rcWork.bottom-rcWork.top));
	wcscpy_c(m_Values[csi_WindowWork].szFormat, m_Values[csi_WindowWork].sText/*L"9999x9999"*/);

	CEStatusItems Changed[] = {csi_WindowPos, csi_WindowSize, csi_WindowClient, csi_WindowWork};
	// -- обновляем сразу, иначе получаются странные эффекты "отставания" статуса от размера окна...
	if (!OnDataChanged(Changed, countof(Changed), true))
	{
		// Размер окна изменился - даже если колонки не менялись, все равно нужно отрисоваться
		InvalidateStatusBar();
	}
}

// bForceUpdate надо ставить в true, после изменения размеров консоли! Чтобы при ресайзе
// актуальные данные показывать. Другие значения (скролл и курсор) можно и с задержкой
// отображать, чтобы лишнюю нагрузку не создавать.
void CStatus::OnConsoleChanged(const CONSOLE_SCREEN_BUFFER_INFO* psbi, const CONSOLE_CURSOR_INFO* pci, bool bForceUpdate)
{
	bool bValid = (psbi != NULL);

	// csi_ConsolePos:
	if (bValid)
	{
		_wsprintf(m_Values[csi_ConsolePos].sText, SKIPLEN(countof(m_Values[csi_ConsolePos].sText)-1) L"(%i,%i)-(%i,%i)",
			(int)psbi->srWindow.Left+1, (int)psbi->srWindow.Top+1, (int)psbi->srWindow.Right+1, (int)psbi->srWindow.Bottom+1);
		_wsprintf(m_Values[csi_ConsolePos].szFormat, SKIPLEN(countof(m_Values[csi_ConsolePos].szFormat)) L" (%i,%i)-(%i,%i) ",
			(int)psbi->srWindow.Left+1, (int)psbi->srWindow.Top+1, (int)psbi->dwSize.X, (int)psbi->dwSize.Y);
	}
	else
	{
		wcscpy_c(m_Values[csi_ConsolePos].sText, L" ");
		wcscpy_c(m_Values[csi_ConsolePos].szFormat, L"(0,0)-(199,199)"); // на самом деле может быть до 32766, но для уменьшения ширины - по умолчанию так
	}

	// csi_ConsoleSize:
	if (bValid)
		_wsprintf(m_Values[csi_ConsoleSize].sText, SKIPLEN(countof(m_Values[csi_ConsoleSize].sText)-1) L"%ix%i", (int)psbi->srWindow.Right-psbi->srWindow.Left+1, (int)psbi->srWindow.Bottom-psbi->srWindow.Top+1);
	else
		wcscpy_c(m_Values[csi_ConsoleSize].sText, L" ");
	//m_Values[csi_ConsoleSize].sFormat = L"999x999";
	//_wsprintf(m_Values[csi_ConsoleSize].szFormat, SKIPLEN(countof(m_Values[csi_ConsoleSize].szFormat)) L" %ix%i",
	//	(int)psbi->dwSize.X, max((int)psbi->dwSize.Y,(int)gpSet->DefaultBufferHeight));
	wcscpy_c(m_Values[csi_ConsoleSize].szFormat, m_Values[csi_ConsoleSize].sText);

	// csi_BufferSize:
	if (bValid)
	{
		_wsprintf(m_Values[csi_BufferSize].sText, SKIPLEN(countof(m_Values[csi_BufferSize].sText)-1) L"%ix%i", (int)psbi->dwSize.X, (int)psbi->dwSize.Y);
		_wsprintf(m_Values[csi_BufferSize].szFormat, SKIPLEN(countof(m_Values[csi_BufferSize].szFormat)) L" %ix%i",
			(int)psbi->dwSize.X, max((int)psbi->dwSize.Y,(int)gpSet->DefaultBufferHeight));
	}
	else
	{
		wcscpy_c(m_Values[csi_BufferSize].sText, L" ");
		//m_Values[csi_BufferSize].sFormat = L"999x9999"; // на самом деле может быть до 9999, но для уменьшения ширины - по умолчанию так
		wcscpy_c(m_Values[csi_BufferSize].szFormat, m_Values[csi_BufferSize].sText);
	}

	if (psbi && pci)
	{
		OnCursorChanged(&psbi->dwCursorPosition, pci, psbi->dwSize.X, psbi->dwSize.Y);
	}

	CEStatusItems Changed[] = {csi_ConsolePos, csi_ConsoleSize, csi_BufferSize};
	OnDataChanged(Changed, countof(Changed), bForceUpdate);
}

void CStatus::OnCursorChanged(const COORD* pcr, const CONSOLE_CURSOR_INFO* pci, int nMaxX /*= 0*/, int nMaxY /*= 0*/)
{
	bool bValid = (pcr != NULL) && (pci != NULL);

	// csi_CursorX:
	if (bValid)
	{
		_wsprintf(m_Values[csi_CursorX].sText, SKIPLEN(countof(m_Values[csi_CursorX].sText)-1) _T("%i"), (int)pcr->X+1);
		if (nMaxX)
			_wsprintf(m_Values[csi_CursorX].szFormat, SKIPLEN(countof(m_Values[csi_CursorX].szFormat)) L" %i", nMaxX);
	}
	else
	{
		wcscpy_c(m_Values[csi_CursorX].sText, L" ");
		wcscpy_c(m_Values[csi_CursorX].szFormat, L"99");
	}

	// csi_CursorY:
	if (bValid)
	{
		_wsprintf(m_Values[csi_CursorY].sText, SKIPLEN(countof(m_Values[csi_CursorY].sText)-1) _T("%i"), (int)pcr->Y+1);
		if (nMaxY)
			_wsprintf(m_Values[csi_CursorY].szFormat, SKIPLEN(countof(m_Values[csi_CursorY].szFormat)) L" %i", nMaxY);
	}
	else
	{
		wcscpy_c(m_Values[csi_CursorY].sText, L" ");
		wcscpy_c(m_Values[csi_CursorY].szFormat, L"99");
	}

	// csi_CursorSize:
	if (bValid)
	{
		_wsprintf(m_Values[csi_CursorSize].sText, SKIPLEN(countof(m_Values[csi_CursorSize].sText)-1) _T("%i%s"), pci->dwSize, pci->bVisible ? L"V" : L"H");
		wcscpy_c(m_Values[csi_CursorSize].szFormat, L"100V");
	}
	else
	{
		wcscpy_c(m_Values[csi_CursorSize].sText, L" ");
		wcscpy_c(m_Values[csi_CursorSize].szFormat, L"(99,199) 99V");
	}

	// csi_CursorInfo:
	if (bValid)
	{
		_wsprintf(m_Values[csi_CursorInfo].sText, SKIPLEN(countof(m_Values[csi_CursorInfo].sText)-1) _T("(%i,%i) %i%s"), (int)pcr->X+1, (int)pcr->Y+1, pci->dwSize, pci->bVisible ? L"V" : L"H");
		if (nMaxX && nMaxY)
			_wsprintf(m_Values[csi_CursorInfo].szFormat, SKIPLEN(countof(m_Values[csi_CursorInfo].szFormat)) L" (%i,%i) 100V", nMaxX, nMaxY);
	}
	else
	{
		wcscpy_c(m_Values[csi_CursorInfo].sText, L" ");
		wcscpy_c(m_Values[csi_CursorInfo].szFormat, L"(99,199) 99V");
	}

	CEStatusItems Changed[] = {csi_CursorX, csi_CursorY, csi_CursorSize, csi_CursorInfo};
	OnDataChanged(Changed, countof(Changed));
}

//void CStatus::OnConsoleTitleChanged(CRealConsole* pRCon)
//{
//	lstrcpyn(m_Values[csi_ConsoleTitle]
//}

void CStatus::OnConsoleBufferChanged(CRealConsole* pRCon)
{
	RealBufferType tBuffer = pRCon ? pRCon->GetActiveBufferType() : rbt_Undefined;
	TODO("isBufferWidth");
	bool bIsScroll = pRCon ? pRCon->isBufferHeight() : false;
	wchar_t cScroll = (gnOsVer>=0x0601/*Windows7*/) ? 0x2195 : L']'/*WinXP has not arrows*/; // Up/Down arrow

	wcscpy_c(m_Values[csi_ActiveBuffer].sText,
			(tBuffer == rbt_Primary) ? L"PRI" :
			(tBuffer == rbt_Alternative) ? L"ALT" :
			(tBuffer == rbt_Selection) ? L"SEL" :
			(tBuffer == rbt_Find) ? L"FND" :
			(tBuffer == rbt_DumpScreen) ? L"DMP" :
			L" ");
	if (bIsScroll)
	{
		m_Values[csi_ActiveBuffer].sText[4] = 0;
		m_Values[csi_ActiveBuffer].sText[3] = cScroll;
	}

	wcscpy_c(m_Values[csi_ActiveBuffer].szFormat, L"DUMP");

	CEStatusItems Changed[] = {csi_ActiveBuffer};
	// -- обновляем сразу, иначе получаются странные эффекты "отставания" статуса от окна...
	OnDataChanged(Changed, countof(Changed), true);
}

void CStatus::OnActiveVConChanged(int nIndex/*0-based*/, CRealConsole* pRCon)
{
	//int nActive = gpConEmu->GetActiveVCon();
	int nCount = gpConEmu->GetConCount();

	if ((nCount > 0) && (nIndex >= 0))
	{
		_wsprintf(m_Values[csi_ActiveVCon].sText, SKIPLEN(countof(m_Items[csi_ActiveVCon].sText)) L"%i/%i",
			nIndex+1, nCount);
	}
	else
	{
		wcscpy_c(m_Values[csi_ActiveVCon].sText, L" ");
	}
	wcscpy_c(m_Values[csi_ActiveVCon].szFormat, L"99/99");

	//CEStatusItems Changed[] = {csi_ActiveVCon};
	//OnDataChanged(Changed, countof(Changed));

	OnConsoleBufferChanged(pRCon);

	HWND hView = NULL, hCon = NULL;

	if (pRCon)
	{
		DWORD nMainServerPID = pRCon->isServerCreated() ? pRCon->GetServerPID(true) : 0;
		DWORD nAltServerPID = nMainServerPID ? pRCon->GetServerPID(false) : 0;
		OnServerChanged(nMainServerPID, nAltServerPID);

		CONSOLE_SCREEN_BUFFER_INFO sbi = {};
		CONSOLE_CURSOR_INFO ci = {};
		pRCon->GetConsoleScreenBufferInfo(&sbi);
		pRCon->GetConsoleCursorInfo(&ci);
		OnConsoleChanged(&sbi, &ci, false);

		hView = pRCon->GetView();
		hCon = pRCon->ConWnd();
	}
	else
	{
		OnServerChanged(0, 0);
		OnConsoleChanged(NULL, NULL, false);
	}

	// csi_ConEmuView
	_wsprintf(m_Values[csi_ConEmuView].sText, SKIPLEN(countof(m_Values[csi_ConEmuView].sText)-1)
		L"x%08X(%u)", (DWORD)hView, (DWORD)hView);
	wcscpy_c(m_Values[csi_ConEmuView].szFormat, m_Values[csi_ConEmuView].sText);

	// csi_ServerHWND
	_wsprintf(m_Values[csi_ServerHWND].sText, SKIPLEN(countof(m_Values[csi_ServerHWND].sText)-1)
		L"x%08X(%u)", (DWORD)hCon, (DWORD)hCon);
	wcscpy_c(m_Values[csi_ServerHWND].szFormat, m_Values[csi_ServerHWND].sText);

	// Чтобы уж точно обновилось - как минимум, меняется PID активного процесса
	UpdateStatusBar(true);
}

void CStatus::OnServerChanged(DWORD nMainServerPID, DWORD nAltServerPID)
{
	if (nMainServerPID)
	{
		if ((nMainServerPID == nAltServerPID) || !nAltServerPID)
		{
			_wsprintf(m_Values[csi_Server].sText, SKIPLEN(countof(m_Items[csi_Server].sText)) _T("%u"), nMainServerPID);
			wcscpy_c(m_Values[csi_Server].szFormat, L"99999");
		}
		else
		{
			_wsprintf(m_Values[csi_Server].sText, SKIPLEN(countof(m_Values[csi_Server].sText)) _T("%u/%u"), nMainServerPID, nAltServerPID);
			wcscpy_c(m_Values[csi_Server].szFormat, L"99999/99999");
		}
	}
	else
	{
		wcscpy_c(m_Values[csi_Server].sText, L" ");
		wcscpy_c(m_Values[csi_Server].szFormat, L"99999");
	}

	CEStatusItems Changed[] = {csi_Server};
	OnDataChanged(Changed, countof(Changed));
}

LPCWSTR CStatus::GetSettingName(CEStatusItems nID)
{
	_ASSERTE(this);
	if (nID <= csi_Info || nID >= csi_Last)
		return NULL;

	_ASSERTE(m_Values[nID].sSettingName!=NULL && m_Values[nID].sSettingName[0]!=0);
	return m_Values[nID].sSettingName;
}

bool CStatus::IsTimeChanged()
{
	if (gpSet->isStatusColumnHidden[csi_Time])
		return false; // Не показан

	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	if (st.wHour != mt_LastTime.wHour || st.wMinute != mt_LastTime.wMinute || st.wSecond != mt_LastTime.wSecond)
	{
		mt_LastTime = st;
		_wsprintf(m_Values[csi_Time].sText, SKIPLEN(countof(m_Values[csi_Time].sText)) L"%u:%02u:%02u", (uint)st.wHour, (uint)st.wMinute, (uint)st.wSecond);
		return true;
	}

	return false;
}

bool CStatus::IsKeyboardChanged()
{
	if (gpSet->isStatusColumnHidden[csi_CapsLock]
		&& gpSet->isStatusColumnHidden[csi_NumLock]
		&& gpSet->isStatusColumnHidden[csi_ScrollLock]
		&& gpSet->isStatusColumnHidden[csi_KeyHooks]
		&& gpSet->isStatusColumnHidden[csi_InputLocale])
	{
		// Если ни одна из клавиатурных "кнопок" не показана - то и делать нечего
		return false;
	}

	bool bChanged = false;

	bool bCaps = (GetKeyState(VK_CAPITAL) & 1) == 1;
	bool bNum = (GetKeyState(VK_NUMLOCK) & 1) == 1;
	bool bScroll = (GetKeyState(VK_SCROLL) & 1) == 1;
	bool bKeyHooks = gpConEmu->IsKeyboardHookRegistered();
	DWORD_PTR hkl = gpConEmu->GetActiveKeyboardLayout();

	if (bCaps != mb_Caps)
	{
		mb_Caps = bCaps; bChanged = true;
	}

	if (bNum != mb_Num)
	{
		mb_Num = bNum; bChanged = true;
	}

	if (bScroll != mb_Scroll)
	{
		mb_Scroll = bScroll; bChanged = true;
	}

	if (bKeyHooks != mb_KeyHooks)
	{
		mb_KeyHooks = bKeyHooks; bChanged = true;
	}

	if (hkl != mhk_Locale)
	{
		mhk_Locale = hkl; bChanged = true;
	}

	return bChanged;
}

bool CStatus::IsWindowChanged()
{
	if (gpSet->isStatusColumnHidden[csi_WindowStyle]
		&& gpSet->isStatusColumnHidden[csi_WindowStyleEx]
		&& gpSet->isStatusColumnHidden[csi_Zoom]
		&& gpSet->isStatusColumnHidden[csi_DPI]
		&& gpSet->isStatusColumnHidden[csi_HwndFore]
		&& gpSet->isStatusColumnHidden[csi_HwndFocus])
	{
		// Если ни одна позиция не показана - то и делать нечего
		return false;
	}
	_ASSERTE(isMainThread());

	bool bChanged = false;
	DWORD n; HWND h;
	LONG l;

	if (!gpSet->isStatusColumnHidden[csi_WindowStyle])
	{
		n = GetWindowLong(ghWnd, GWL_STYLE);
		if (n != mn_Style)
		{
			mn_Style = n; bChanged = true;
		}
	}

	if (!gpSet->isStatusColumnHidden[csi_WindowStyleEx])
	{
		n = GetWindowLong(ghWnd, GWL_EXSTYLE);
		if (n != mn_ExStyle)
		{
			mn_ExStyle = n; bChanged = true;
		}
	}

	if (!gpSet->isStatusColumnHidden[csi_Zoom])
	{
		l = gpSetCls->GetZoom();
		if (l != mn_Zoom)
		{
			mn_Zoom = l; bChanged = true;
		}
	}

	if (!gpSet->isStatusColumnHidden[csi_DPI])
	{
		l = gpSetCls->QueryDpi();
		if (l != mn_Dpi)
		{
			mn_Dpi = l; bChanged = true;
		}
	}

	if (!gpSet->isStatusColumnHidden[csi_HwndFore])
	{
		h = getForegroundWindow();
		if (h != mh_Fore)
		{
			mh_Fore = h; bChanged = true;
			::getWindowInfo(h, ms_ForeInfo, true, &mn_ForePID);
		}
	}

	if (!gpSet->isStatusColumnHidden[csi_HwndFocus])
	{
		h = ::GetFocus();
		if (h != mh_Focus)
		{
			mh_Focus = h; bChanged = true;
			::getWindowInfo(h, ms_FocusInfo, true, &mn_FocusPID);
		}
	}

	return bChanged;
}

void CStatus::OnKeyboardChanged()
{
	if (/*!mb_Invalidated &&*/ IsKeyboardChanged())
	{
		// Изменения в клавиатуре - отрисовать сразу
		UpdateStatusBar(true);
	}
}

void CStatus::OnTransparency()
{
	_wsprintf(m_Values[csi_Transparency].sText, SKIPLEN(countof(m_Items[csi_Transparency].sText))
		_T("%u%%%s%s"), (UINT)(gpConEmu->mn_LastTransparentValue >= 255) ? 100 : (gpConEmu->mn_LastTransparentValue * 100 / 255),
		gpSet->isUserScreenTransparent ? L",USR" : L"",
		L""); // TODO: ColorKey transparency
	wcscpy_c(m_Values[csi_Transparency].szFormat, L"100%");

	CEStatusItems Changed[] = {csi_Transparency};
	OnDataChanged(Changed, countof(Changed), true);
}

void CStatus::ProcessMenuHighlight(HMENU hMenu, WORD nID, WORD nFlags)
{
	if (mb_InSetupMenu)
	{
		if ((nID > 0) && (nID <= countof(gStatusCols)))
			SetClickedItemDesc(gStatusCols[nID-1].nID);
		else
			SetClickedItemDesc(csi_Last);
	}
	else switch (m_ClickedItemDesc)
	{
		case csi_Transparency:
			{
				if (ProcessTransparentMenuId(nID, true))
					gpConEmu->OnTransparent();
			}
			break;
		case csi_Zoom:
			ProcessZoomMenuId(nID);
			break;
		default:
			;
	}
}

void CStatus::ShowVConMenu(POINT pt)
{
	mb_InPopupMenu = true;

	_ASSERTE(m_ClickedItemDesc == csi_ActiveVCon);
	m_ClickedItemDesc = csi_ActiveVCon;

	gpConEmu->ChooseTabFromMenu(FALSE, pt, TPM_LEFTALIGN|TPM_BOTTOMALIGN);

	mb_InPopupMenu = false;
}

bool CStatus::ProcessTransparentMenuId(WORD nCmd, bool abAlphaOnly)
{
	bool bSelected = false;

	StatusMenuOptions* p;
	if ((p = GetStatusMenuItem(nCmd, gTranspOpt, countof(gTranspOpt))) != NULL)
	{
		// Change TEMPORARILY, without saving settings
		if (p->nValue >= 40)
		{
			if ((p->nValue < 100) || !abAlphaOnly)
			{
				gpSet->nTransparent = min(255,((p->nValue*255/100)+1));
				bSelected = true;
			}
		}
		else if (!abAlphaOnly)
		{
			switch (p->nValue)
			{
				case 1:
					gpSet->isUserScreenTransparent = !gpSet->isUserScreenTransparent;
					gpConEmu->OnHideCaption(); // при прозрачности - обязательно скрытие заголовка + кнопки
					gpConEmu->UpdateWindowRgn();
					// Отразить изменения в статусе
					OnTransparency();
					break;
				case 2:
					gpSet->isColorKeyTransparent = !gpSet->isColorKeyTransparent;
					bSelected = true;
					break;
			}
		}
	}

	if (bSelected)
	{
		// Отразить изменения в статусе
		OnTransparency();
	}

	return bSelected;
}

bool CStatus::ProcessZoomMenuId(WORD nCmd)
{
	StatusMenuOptions* p;
	if ((p = GetStatusMenuItem(nCmd, gZoomOpt, countof(gZoomOpt))) != NULL)
	{
		gpSetCls->MacroFontSetSize(2, p->nValue);
		return true;
	}
	return false;
}

bool CStatus::isSettingsOpened(UINT nOpenPageID)
{
	if (ghOpWnd && IsWindow(ghOpWnd))
	{
		gpSetCls->Dialog(nOpenPageID);
		if (!nOpenPageID)
			MessageBox(ghOpWnd, L"Close settings dialog first, please.", gpConEmu->GetDefaultTitle(), MB_ICONEXCLAMATION);
		return true;
	}

	return false;
}

void CStatus::ShowTransparencyMenu(POINT pt)
{
	if (isSettingsOpened(IDD_SPG_TRANSPARENT))
		return;

	mb_InPopupMenu = true;
	HMENU hPopup = CreateStatusMenu(gTranspOpt, countof(gTranspOpt));

	// меню было расчитано на min=40
	_ASSERTE(MIN_ALPHA_VALUE==40);

	u8 nPrevAlpha = gpSet->nTransparent;
	//bool bPrevUserScreen = gpSet->isUserScreenTransparent; -- он идет через WindowRgn, а не Transparency
	bool bPrevColorKey = gpSet->isColorKeyTransparent;

	_ASSERTE(m_ClickedItemDesc == csi_Transparency);
	int nCmd = ShowStatusBarMenu(pt, hPopup, csi_Transparency);

	bool bSelected = ProcessTransparentMenuId(nCmd, false);

	if (!bSelected)
	{
		// Если поменяли альфу при Hover мышкой...
		if ((nPrevAlpha != gpSet->nTransparent)
			//|| (bPrevUserScreen != gpSet->isUserScreenTransparent)
			|| (bPrevColorKey != gpSet->isColorKeyTransparent))
		{
			gpSet->nTransparent = nPrevAlpha;
			//gpSet->isUserScreenTransparent = bPrevUserScreen;
			gpSet->isColorKeyTransparent = bPrevColorKey;
			bSelected = true;
		}
	}

	if (bSelected)
	{
		gpConEmu->OnTransparent();
	}

	// Done
	DestroyMenu(hPopup);
	mb_InPopupMenu = false;

	// Отразить изменения в статусе
	OnTransparency();
}

void CStatus::ShowZoomMenu(POINT pt)
{
	mb_InPopupMenu = true;
	HMENU hPopup = CreateStatusMenu(gZoomOpt, countof(gZoomOpt));

	_ASSERTE(m_ClickedItemDesc == csi_Zoom);
	int nPrevZoom = gpSetCls->GetZoom(true);
	int nCmd = ShowStatusBarMenu(pt, hPopup, csi_Zoom);

	bool bSelected = ProcessZoomMenuId(nCmd);

	if (!bSelected)
	{
		if (nPrevZoom != gpSetCls->GetZoom(true))
		{
			gpSetCls->MacroFontSetSize(3, nPrevZoom);
		}
	}

	// Done
	DestroyMenu(hPopup);
	mb_InPopupMenu = false;

	// Отразить изменения в статусе
	IsWindowChanged();
}

// Прямоугольник в клиентских координатах ghWnd!
bool CStatus::GetStatusBarClientRect(RECT* rc)
{
	if (!gpSet->isStatusBarShow)
		return false;

	RECT rcClient = gpConEmu->GetGuiClientRect();

	int nHeight = gpSet->StatusBarHeight();
	if (nHeight >= (rcClient.bottom - rcClient.top))
		return false;

	rcClient.top = rcClient.bottom /*- 1*/ - nHeight;

	*rc = rcClient;
	return true;
}

// Прямоугольник в клиентских координатах ghWnd!
bool CStatus::GetStatusBarItemRect(CEStatusItems nID, RECT* rc)
{
	if (!gpSet->isStatusBarShow)
		return false;

	for (size_t i = 0; i < countof(m_Items); i++)
	{
		if (m_Items[i].bShow && (m_Items[i].nID == nID))
		{
			if (rc)
				*rc = m_Items[i].rcClient;
			return true;
		}
	}

	if (rc)
		GetStatusBarClientRect(rc);

	return false;
}

HMENU CStatus::CreateStatusMenu(StatusMenuOptions* pItems, size_t nCount)
{
	HMENU hPopup = CreatePopupMenu();

	// меню было расчитано на min=40
	_ASSERTE(MIN_ALPHA_VALUE==40);

	int nCurrent = -1;

	for (size_t i = 0; i < nCount; i++)
	{
		int iChecked = 0;
		if (!pItems[i].nMenuID)
		{
			AppendMenu(hPopup, MF_SEPARATOR, 0, L"");
		}
		else
		{
			if (pItems[i].pfnChecked)
			{
				iChecked = pItems[i].pfnChecked(pItems[i].nValue, ((i+1) < nCount) ? &pItems[i+1].nValue : NULL);
				if (iChecked == 1)
					nCurrent = pItems[i].nMenuID;
			}

			AppendMenu(hPopup, MF_STRING|(iChecked ? MF_CHECKED : MF_UNCHECKED), pItems[i].nMenuID, pItems[i].sText);
		}
	}

	if (nCurrent != -1)
	{
		MENUITEMINFO mi = {sizeof(mi), MIIM_STATE|MIIM_ID};
		mi.wID = nCurrent;
		GetMenuItemInfo(hPopup, nCurrent, FALSE, &mi);
		mi.fState |= MF_DEFAULT;
		SetMenuItemInfo(hPopup, nCurrent, FALSE, &mi);
	}

	return hPopup;
}

CStatus::StatusMenuOptions* CStatus::GetStatusMenuItem(WORD nMenuID, StatusMenuOptions* pItems, size_t nCount)
{
	if (!nMenuID)
		return NULL;

	CStatus::StatusMenuOptions* p = NULL;

	for (size_t i = 0; i < nCount; i++)
	{
		if (pItems[i].nMenuID == nMenuID)
		{
			p = (pItems+i);
			break;
		}
	}

	return p;
}

int CStatus::Transparent_IsMenuChecked(int nValue, int* pnNextValue)
{
	if (nValue == 1)
	{
		return gpSet->isUserScreenTransparent ? 2 : 0;
	}
	else if (nValue == 2)
	{
		return gpSet->isColorKeyTransparent ? 2 : 0;
	}
	else if (nValue == 100)
	{
		return (gpSet->nTransparent == 255);
	}
	else if (pnNextValue)
	{
		UINT nVal = nValue * 255 / 100;
		UINT nNextVal = ((*pnNextValue < 10) ? 255 : (*pnNextValue * 255 / 100));
		return ((nVal <= gpSet->nTransparent) && (gpSet->nTransparent < nNextVal));
	}
	else
	{
		_ASSERTE(pnNextValue!=NULL);
	}

	return false;
}

int CStatus::Zoom_IsMenuChecked(int nValue, int* pnNextValue)
{
	int nZoom = gpSetCls->GetZoom();
	int iChecked = 0;
	if (nValue == nZoom)
		iChecked = true;
	else if (pnNextValue)
		iChecked = (nValue < nZoom && nZoom < *pnNextValue);
	else
		iChecked = (nValue > nZoom);
	return iChecked;
}

int CStatus::ShowStatusBarMenu(POINT pt, HMENU hPopup, CEStatusItems csi)
{
	m_ClickedItemDesc = csi;

	RECT rcExcl;
	if (!GetStatusBarItemRect(csi, &rcExcl))
		rcExcl = MakeRect(pt.x-1, pt.y-1, pt.x+1, pt.y+1);
	else
		MapWindowPoints(ghWnd, NULL, (LPPOINT)&rcExcl, 2);

	int nCmd = gpConEmu->mp_Menu->trackPopupMenu(tmp_StatusBarCols, hPopup, TPM_BOTTOMALIGN|TPM_RIGHTALIGN|TPM_RETURNCMD, pt.x, pt.y, ghWnd, &rcExcl);
	return nCmd;
}
