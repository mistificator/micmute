// mic_mute.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "mic_mute.h"
#include "ShellAPI.h"
#include "./../key_hook/key_hook.h"
#include "strsafe.h"

_SetShortCut	SetShortcut;
_SetEnabled		SetEnabled;
_GetShortCut	GetShortcut;

TCHAR _ini[] = _T(".\\mic_mute.ini");

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
DWORD SavedVolume = 0;
UINT TrayMsg;
HMENU TrayMenu;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	SetupShortcut(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	ShortcutEditProc(HWND, UINT, WPARAM, LPARAM);
VOID				MuteToggle(HWND hWnd);
VOID				StartMutedToggle(HWND hWnd);

CMixer mixer_mic_in(MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE, CMixer::Record);

HANDLE HookEvent = 0;
HANDLE StopEvent = 0;
HANDLE Thread = 0;
HWND AppHWnd = 0;
BOOL StartMuted = FALSE;

DWORD WINAPI ThreadProc( LPVOID lpParam ) 
{
	while (1)
	{
		if (WaitForSingleObject(HookEvent, 0) == WAIT_OBJECT_0)
		{
			ResetEvent(HookEvent);
			MuteToggle(HWND(lpParam));
		}
		if (WaitForSingleObject(StopEvent, 0) == WAIT_OBJECT_0)
		{
			break;
		}
		Sleep(100);
	}
	return 0;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;
	HOOKPROC hkprc; 
	static HINSTANCE hinstDLL; 
	static HHOOK hhook; 

	hinstDLL = LoadLibrary(_T("key_hook.dll")); 
	hkprc = (HOOKPROC)GetProcAddress(hinstDLL, "_KeyboardProc@12"); 
	SetShortcut = (_SetShortCut)GetProcAddress(hinstDLL, "_SetShortcut@12");
	GetShortcut = (_GetShortCut)GetProcAddress(hinstDLL, "_GetShortcut@12");
	SetEnabled = (_SetEnabled)GetProcAddress(hinstDLL, "_SetEnabled@4");
	hhook = SetWindowsHookEx(WH_KEYBOARD,hkprc,hinstDLL,NULL); 
	HookEvent = CreateEvent(NULL, TRUE, FALSE, _T("Hooked!"));

	TCHAR _str[1024];
	int _count, _key1, _key2, _start_muted;
	GetPrivateProfileString(_T("Mic_Mute"), _T("ShortCut_Count"), _T("2"), _str, 1024, _ini);
	_stscanf(_str, _T("%i"), &_count);
	GetPrivateProfileString(_T("Mic_Mute"), _T("ShortCut_Key1"), _T("92"), _str, 1024, _ini);
	_stscanf(_str, _T("%i"), &_key1);
	GetPrivateProfileString(_T("Mic_Mute"), _T("ShortCut_Key2"), _T("17"), _str, 1024, _ini);
	_stscanf(_str, _T("%i"), &_key2);
	GetPrivateProfileString(_T("Mic_Mute"), _T("StartMuted"), _T("0"), _str, 1024, _ini);
	_stscanf(_str, _T("%i"), &_start_muted);
	StartMuted = (_start_muted != 0);

	SetShortcut(_count, _key1, _key2);

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MIC_MUTE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, SW_HIDE))
	{
		return FALSE;
	}

	Thread = CreateThread( 
		NULL,              // default security attributes
		0,                 // use default stack size  
		ThreadProc,        // thread function 
		AppHWnd,             // argument to thread function 
		0,                 // use default creation flags 
		0);   // returns the thread identifier 
	StopEvent = CreateEvent(NULL, TRUE, FALSE, _T("Stop!"));

	TrayMsg = RegisterWindowMessage(_T("Tray!"));

	NOTIFYICONDATA nid;
	nid.cbSize = sizeof(nid);
	nid.hWnd = AppHWnd;
	nid.uID = 1;
	nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MIC_MUTE));
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_INFO;
	nid.uCallbackMessage = TrayMsg;
	nid.uTimeout = 5000;
	nid.dwInfoFlags = NIIF_USER | NIIF_NOSOUND;
	TCHAR _tip[] = L"MicMute";
	StringCchCopy(nid.szTip, 128, _tip);
	StringCchCopy(nid.szInfoTitle, 64, _tip);
	TCHAR _tooltip_text[] = _T("Started");
	StringCchCopy(nid.szInfo, 256, _tooltip_text);
	Shell_NotifyIcon(NIM_ADD, &nid);

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MIC_MUTE));

	HMENU hmenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_TRAY_MIC_MUTE));
	TrayMenu = GetSubMenu(hmenu, 0);
	SetMenuDefaultItem(TrayMenu, IDM_SHOW_MICMUTE, 0);

	if (StartMuted != 0)
	{
		StartMutedToggle(AppHWnd);
		MuteToggle(AppHWnd);
	}

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	_start_muted = (StartMuted == TRUE) ? 1 : 0;
	GetShortcut(&_count, &_key1, &_key2);
	_stprintf(_str, _T("%i"), _count);
	WritePrivateProfileString(_T("Mic_Mute"), _T("ShortCut_Count"), _str, _ini);
	_stprintf(_str, _T("%i"), _key1);
	WritePrivateProfileString(_T("Mic_Mute"), _T("ShortCut_Key1"), _str, _ini);
	_stprintf(_str, _T("%i"), _key2);
	WritePrivateProfileString(_T("Mic_Mute"), _T("ShortCut_Key2"), _str, _ini);
	_stprintf(_str, _T("%i"), _start_muted);
	WritePrivateProfileString(_T("Mic_Mute"), _T("StartMuted"), _str, _ini);

	Shell_NotifyIcon(NIM_DELETE, &nid);

	SetEvent(StopEvent);
	WaitForSingleObject(Thread, INFINITE);
	CloseHandle(Thread);
	CloseHandle(StopEvent);
	CloseHandle(HookEvent);
	UnhookWindowsHookEx(hhook);
	FreeLibrary(hinstDLL);

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MIC_MUTE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MIC_MUTE);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   AppHWnd = CreateWindow(szWindowClass, szTitle, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
      CW_USEDEFAULT, CW_USEDEFAULT, 240, 120, NULL, NULL, hInstance, NULL);

   if (!AppHWnd)
   {
      return FALSE;
   }

   ShowWindow(AppHWnd, nCmdShow);
   UpdateWindow(AppHWnd);

   return TRUE;
}

VOID MuteToggle(HWND hWnd)
{
	HMENU menu = GetMenu(hWnd);
	DWORD mute_state;
	DWORD PrevVolume;

	NOTIFYICONDATA nid;
	nid.cbSize = sizeof(nid);
	nid.hWnd = AppHWnd;
	nid.uID = 1;
	nid.uFlags = NIF_TIP | NIF_INFO;
	nid.uTimeout = 5000;
	nid.dwInfoFlags = NIIF_USER | NIIF_NOSOUND;
	TCHAR _tooltip_title[] = _T("MicMute");
	StringCchCopy(nid.szInfoTitle, 64, _tooltip_title);

	FLASHWINFO flash;
	flash.cbSize = sizeof(flash);
	flash.hwnd = hWnd;
	flash.dwTimeout = 0;
	flash.uCount = 0xffffffff;

	mute_state = CheckMenuItem(menu, IDM_MUTE, MF_UNCHECKED);
	if (mute_state == MF_UNCHECKED)
	{
		TCHAR _tooltip_text[] = _T("Mic Off");
		StringCchCopy(nid.szInfo, 265, _tooltip_text);
		TCHAR _tip[] = _T("MicMute - Mic Off");
		StringCchCopy(nid.szTip, 128, _tip);
		SetWindowText(hWnd, (LPTSTR)_tip);
		CheckMenuItem(menu, IDM_MUTE, mute_state = MF_CHECKED);
		flash.dwFlags = FLASHW_TRAY;
	}
	else 
	{
		TCHAR _tooltip_text[] = _T("Mic On");
		StringCchCopy(nid.szInfo, 256, _tooltip_text);
		TCHAR _tip[] = _T("MicMute");
		StringCchCopy(nid.szTip,128,  _tip);
		SetWindowText(hWnd, (LPTSTR)_tip);
		mute_state = MF_UNCHECKED;
		flash.dwFlags = FLASHW_STOP;
	}
	CheckMenuItem(TrayMenu, IDM_MUTE, mute_state);

	Shell_NotifyIcon(NIM_MODIFY, &nid);
	FlashWindowEx(&flash);

	PrevVolume = mixer_mic_in.GetVolume();
	mixer_mic_in.SetVolume(mute_state == MF_CHECKED ? 0 : SavedVolume);
	if (PrevVolume > 0)
	{
		SavedVolume = PrevVolume;
	}
	if (SavedVolume == 0)
	{
		SavedVolume = 32768;
	}

}

VOID StartMutedToggle(HWND hWnd)
{
	DWORD mute_state;
	HMENU menu = GetMenu(hWnd);
	mute_state = CheckMenuItem(menu, IDM_START_MUTED, MF_UNCHECKED);
	if (mute_state == MF_UNCHECKED)
	{
		CheckMenuItem(menu, IDM_START_MUTED, mute_state = MF_CHECKED);
		StartMuted = TRUE;
	}
	else
	{
		mute_state = MF_UNCHECKED;
		StartMuted = FALSE;
	}
	CheckMenuItem(TrayMenu, IDM_START_MUTED, mute_state);	
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	POINT pt;

	if (message == TrayMsg)
	{
		switch ((UINT)lParam)
		{
		case WM_LBUTTONDBLCLK:
			ShowWindow(hWnd, SW_SHOWNORMAL);
			break;
		case WM_RBUTTONDOWN:
			SetForegroundWindow(hWnd);
			GetCursorPos(&pt);
			TrackPopupMenu(TrayMenu, 
				TPM_LEFTALIGN, 
				pt.x, pt.y, 0, hWnd, NULL); 
			PostMessage(hWnd, WM_NULL, 0, 0);
			break;
		}
		return 0;
	}
	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_MUTE:
			MuteToggle(hWnd);
			break;
		case IDM_START_MUTED:
			StartMutedToggle(hWnd);
			break;
		case IDM_SHOW_MICMUTE:
			ShowWindow(hWnd, SW_SHOWNORMAL);
			break;
		case IDM_HIDE_MICMUTE:
			ShowWindow(hWnd, SW_HIDE);
			break;
		case IDM_SETUP_SHORTCUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_SETUP_SHORTCUT), hWnd, SetupShortcut);
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_CLOSE:
		ShowWindow(hWnd, SW_HIDE);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...

		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

WNDPROC _edit_proc = 0;
UINT _prev_code;
INT_PTR CALLBACK SetupShortcut(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		SetEnabled(false);
		_prev_code = 0;
		_edit_proc = (WNDPROC)GetWindowLongPtr(GetDlgItem(hDlg, IDC_SHORTCUT), GWLP_WNDPROC);
		SetWindowLongPtr(GetDlgItem(hDlg, IDC_SHORTCUT), GWLP_WNDPROC, (LONG_PTR)ShortcutEditProc);
		return (INT_PTR)TRUE;
	case WM_SHOWWINDOW:
		SetFocus(GetDlgItem(hDlg, IDC_SHORTCUT));
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			SetEnabled(true);
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

TCHAR * KeyToName(UINT _code)
{
	static TCHAR _str[32];
	_str[0] = TCHAR(0);
	_str[1] = TCHAR(0);
	switch (_code)
	{
	case VK_BACK: return _T("Back"); 
	case VK_TAB: return _T("Tab");
	case VK_CLEAR: return _T("Clear");
	case VK_RETURN: return _T("Return");

	case VK_SHIFT: return _T("Shift");
	case VK_CONTROL: return _T("Ctrl");
	case VK_MENU: return _T("Menu");
	case VK_PAUSE: return _T("Pause");
	case VK_CAPITAL: return _T("Caps");

	case VK_ESCAPE: return _T("Esc");

	case VK_SPACE: return _T("Space");
	case VK_PRIOR: return _T("Prior");
	case VK_NEXT : return _T("Next");
	case VK_END: return _T("End");
	case VK_HOME: return _T("Home");
	case VK_LEFT: return _T("Left");
	case VK_UP : return _T("Up");
	case VK_RIGHT: return _T("Right");
	case VK_DOWN: return _T("Down");
	case VK_SELECT: return _T("Select");
	case VK_PRINT: return _T("Print");
	case VK_EXECUTE: return _T("Exec");
	case VK_SNAPSHOT: return _T("Snap");
	case VK_INSERT: return _T("Ins");
	case VK_DELETE : return _T("Del");
	case VK_HELP: return _T("Help");

	case VK_LWIN: return _T("Left Win");
	case VK_RWIN: return _T("Right Win");
	case VK_APPS : return _T("Apps");

	case VK_SLEEP: return _T("Sleep");

	case VK_NUMPAD0: return _T("Num 0");
	case VK_NUMPAD1: return _T("Num 1");
	case VK_NUMPAD2: return _T("Num 2");
	case VK_NUMPAD3: return _T("Num 3");
	case VK_NUMPAD4: return _T("Num 4");
	case VK_NUMPAD5: return _T("Num 5");
	case VK_NUMPAD6: return _T("Num 6");
	case VK_NUMPAD7: return _T("Num 7");
	case VK_NUMPAD8 : return _T("Num 8");
	case VK_NUMPAD9 : return _T("Num 9");
	case VK_MULTIPLY : return _T("Num *");
	case VK_ADD     : return _T("Num +");
	case VK_SEPARATOR : return _T("Num .");
	case VK_SUBTRACT: return _T("Num -");
	case VK_DECIMAL : return _T("Num Dec");
	case VK_DIVIDE : return _T("Num /");
	case VK_F1    : return _T("F1");
	case VK_F2    : return _T("F2");
	case VK_F3    : return _T("F3");
	case VK_F4    : return _T("F4");
	case VK_F5    : return _T("F5");
	case VK_F6    : return _T("F6");
	case VK_F7    : return _T("F7");
	case VK_F8     : return _T("F8");
	case VK_F9     : return _T("F9");
	case VK_F10   : return _T("F10");
	case VK_F11   : return _T("F11");
	case VK_F12   : return _T("F12");
	case VK_F13   : return _T("F13");
	case VK_F14   : return _T("F14");
	case VK_F15   : return _T("F15");
	case VK_F16   : return _T("F16");
	case VK_F17   : return _T("F17");
	case VK_F18    : return _T("F18");
	case VK_F19    : return _T("F19");
	case VK_F20    : return _T("F20");
	case VK_F21    : return _T("F21");
	case VK_F22    : return _T("F22");
	case VK_F23    : return _T("F23");
	case VK_F24   : return _T("F24");

	case VK_NUMLOCK  : return _T("NumLock");
	case VK_SCROLL   : return _T("ScrollLock");
	default: _str[0] = (TCHAR)_code;
	}
	return _str;
}

LRESULT CALLBACK ShortcutEditProc(HWND hEdit, UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT _code;
	static TCHAR _str[1024];
	TCHAR _key1[32], _key2[32];
	UINT _count;
	switch (message)
	{
	case WM_PAINT:
		if (_prev_code == 0)
		{
			GetShortcut((int *)&_count, (int *)&_prev_code, (int *)&_code);
			if (_count == 1)
			{
				StringCchCopy(_key1, 32, KeyToName(_prev_code));
				StringCbPrintf(_str, sizeof(_str), _T("%s"), _key1);
			}
			else
			{
				StringCchCopy(_key1, 32, KeyToName(_prev_code));
				StringCchCopy(_key2, 32, KeyToName(_code));
				StringCbPrintf(_str, sizeof(_str), _T("%s + %s"), _key1, _key2);
			}
			SetWindowText(hEdit, _str);
			_prev_code = 0;
		}
		break;
	case WM_KEYDOWN:
		_code = (UINT)wParam;
		if ((_prev_code == 0) || (_prev_code == _code))
		{
			StringCchCopy(_key1, 32, KeyToName(_code));
			StringCbPrintf(_str, sizeof(_str), _T("%s"), _key1);
			SetShortcut(1, _code, 0);
		}
		else
		{
			StringCchCopy(_key1, 32, KeyToName(_prev_code));
			StringCchCopy(_key2, 32, KeyToName(_code));
			StringCbPrintf(_str, sizeof(_str), _T("%s + %s"), _key1, _key2);
			SetShortcut(2, _prev_code, _code);
		}
		SetWindowText(hEdit, _str);
		_prev_code = _code;
		break;
	}
	return CallWindowProc(_edit_proc, hEdit, message, wParam, lParam);
}