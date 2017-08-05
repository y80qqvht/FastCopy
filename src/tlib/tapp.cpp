﻿static char *tapp_id = 
	"@(#)Copyright (C) 1996-2017 H.Shirouzu		tapp.cpp	Ver0.99";
/* ========================================================================
	Project  Name			: Win32 Lightweight  Class Library Test
	Module Name				: Application Frame Class
	Create					: 1996-06-01(Sat)
	Update					: 2017-06-12(Mon)
	Copyright				: H.Shirouzu
	Reference				: 
	======================================================================== */

#include "tlib.h"

TApp *TApp::tapp = NULL;
#define MAX_TAPPWIN_HASH	1009
#define ENGLISH_TEST		0

TApp::TApp(HINSTANCE _hI, LPSTR _cmdLine, int _nCmdShow)
{
	hInstance		= _hI;
	cmdLine			= _cmdLine;
	nCmdShow		= _nCmdShow;
	mainWnd			= NULL;
	preWnd			= NULL;
	defaultClass	= "tapp";
	defaultClassW	= L"tapp";
	tapp			= this;
	hash			= new TWinHashTbl(MAX_TAPPWIN_HASH);
	twinId			= 1;

	InitInstanceForLoadStr(hInstance);

#if ENGLISH_TEST
	TSetDefaultLCID(0x409); // for English Dialog Test
#else
	TSetDefaultLCID();
#endif
	::CoInitialize(NULL);
	::InitCommonControls();
}

TApp::~TApp()
{
	delete mainWnd;
	::CoUninitialize();
}

int TApp::Run(void)
{
	MSG		msg;

	InitApp();
	InitWindow();

	while (::GetMessageW(&msg, NULL, 0, 0))
	{
		if (PreProcMsg(&msg)) {
			continue;
		}

		::TranslateMessage(&msg);
		::DispatchMessageW(&msg);
	}

	return	(int)msg.wParam;
}

BOOL TApp::PreProcMsg(MSG *msg)	// for TranslateAccel & IsDialogMessage
{
	for (auto hWnd=msg->hwnd; hWnd; hWnd=::GetParent(hWnd))
	{
		if (TWin *win = SearchWnd(hWnd)) {
			return	win->PreProcMsg(msg);
		}
	}

	return	FALSE;
}

LRESULT CALLBACK TApp::WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TApp	*app = TApp::GetApp();
	TWin	*win = app->SearchWnd(hWnd);

	if (win) {
		return	win->WinProc(uMsg, wParam, lParam);
	}

	if ((win = app->preWnd))
	{
		app->preWnd = NULL;
		app->AddWinByWnd(win, hWnd);
		return	win->WinProc(uMsg, wParam, lParam);
	}

	return	::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

BOOL TApp::InitApp(void)	// reference kwc
{
	WNDCLASSW wc;

	memset(&wc, 0, sizeof(wc));
	wc.style			= (CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW | CS_DBLCLKS);
	wc.lpfnWndProc		= WinProc;
	wc.cbClsExtra 		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon			= NULL;
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= NULL;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= defaultClassW;

	if (::FindWindowW(defaultClassW, NULL) == NULL)
	{
		if (::RegisterClassW(&wc) == 0)
			return FALSE;
	}

	return	TRUE;
}

void TApp::Idle(DWORD timeout)
{
	TApp	*app = TApp::GetApp();
	DWORD	start = GetTick();
	MSG		msg;

	while (::PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (app->PreProcMsg(&msg))
			continue;

		::TranslateMessage(&msg);
		::DispatchMessageW(&msg);
		if (GetTick() - start >= timeout) break;
	}
}

BOOL TRegisterClass(LPCSTR class_name, UINT style, HICON hIcon, HCURSOR hCursor,
	HBRUSH hbrBackground, int classExtra, int wndExtra, LPCSTR menu_str)
{
	WNDCLASS	wc;

	memset(&wc, 0, sizeof(wc));
	wc.style			= style;
	wc.lpfnWndProc		= TApp::WinProc;
	wc.cbClsExtra 		= classExtra;
	wc.cbWndExtra		= wndExtra;
	wc.hInstance		= TApp::hInst();
	wc.hIcon			= hIcon;
	wc.hCursor			= hCursor;
	wc.hbrBackground	= hbrBackground;
	wc.lpszMenuName		= menu_str;
	wc.lpszClassName	= class_name;

	return	::RegisterClass(&wc);
}

BOOL TRegisterClassW(const WCHAR *class_name, UINT style, HICON hIcon, HCURSOR hCursor,
	HBRUSH hbrBackground, int classExtra, int wndExtra, const WCHAR *menu_str)
{
	WNDCLASSW	wc;

	memset(&wc, 0, sizeof(wc));
	wc.style			= style;
	wc.lpfnWndProc		= TApp::WinProc;
	wc.cbClsExtra 		= classExtra;
	wc.cbWndExtra		= wndExtra;
	wc.hInstance		= TApp::hInst();
	wc.hIcon			= hIcon;
	wc.hCursor			= hCursor;
	wc.hbrBackground	= hbrBackground;
	wc.lpszMenuName		= (LPCWSTR)menu_str;
	wc.lpszClassName	= (LPCWSTR)class_name;

	return	::RegisterClassW(&wc);
}

BOOL TRegisterClassU8(LPCSTR class_name, UINT style, HICON hIcon, HCURSOR hCursor,
	HBRUSH hbrBackground, int classExtra, int wndExtra, LPCSTR menu_str)
{
	Wstr	class_name_w(class_name, BY_UTF8);
	Wstr	menu_str_w(menu_str, BY_UTF8);

	return	TRegisterClassW(class_name_w.s(), style, hIcon, hCursor, hbrBackground, classExtra,
			wndExtra, menu_str_w.s());
}

