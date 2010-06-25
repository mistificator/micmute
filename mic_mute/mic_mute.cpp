// mic_mute.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "mic_mute.h"
#include "ShellAPI.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
DWORD SavedVolume = 0;
BOOL SavedMicState, SavedLineState;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
VOID				Mute(HWND hWnd);

CMixer mixer_mic_in(MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE, CMixer::Record);
CMixer mixer_mic_out(MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE, CMixer::Play);
CMixer mixer_line_out(MIXERLINE_COMPONENTTYPE_SRC_LINE, CMixer::Play);

HANDLE HookEvent = 0;
HANDLE StopEvent = 0;
HANDLE Thread = 0;
HWND AppHWnd = 0;

DWORD WINAPI ThreadProc( LPVOID lpParam ) 
{
	while (1)
	{
		if (WaitForSingleObject(HookEvent, 0) == WAIT_OBJECT_0)
		{
			ResetEvent(HookEvent);
			Mute(HWND(lpParam));
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

	SavedMicState = mixer_mic_out.GetMute();
	SavedLineState = mixer_line_out.GetMute();

	hinstDLL = LoadLibrary(_T("key_hook.dll")); 
	hkprc = (HOOKPROC)GetProcAddress(hinstDLL, "_KeyboardProc@12"); 
	hhook = SetWindowsHookEx(WH_KEYBOARD,hkprc,hinstDLL,NULL); 
	HookEvent = CreateEvent(NULL, TRUE, FALSE, _T("Hooked!"));

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MIC_MUTE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, SW_MINIMIZE))
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

	NOTIFYICONDATA nid;
	nid.cbSize = sizeof(nid);
	nid.hWnd = AppHWnd;
	nid.uID = 1;
	nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MIC_MUTE));
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	TCHAR _tip[] = L"MicMute";
	CopyMemory(nid.szTip, _tip, sizeof(_tip) + 1);
	Shell_NotifyIcon(NIM_ADD, &nid);

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MIC_MUTE));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	Shell_NotifyIcon(NIM_DELETE, &nid);

	SetEvent(StopEvent);
	WaitForSingleObject(Thread, INFINITE);
	CloseHandle(Thread);
	CloseHandle(StopEvent);
	CloseHandle(HookEvent);
	UnhookWindowsHookEx(hhook);
	FreeLibrary(hinstDLL);

	mixer_mic_out.SetMute(SavedMicState);
	mixer_line_out.SetMute(SavedLineState);

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

VOID Mute(HWND hWnd)
{
	HMENU menu = GetMenu(hWnd);
	DWORD mute_state;
	DWORD PrevVolume;

	FLASHWINFO flash;
	flash.cbSize = sizeof(flash);
	flash.hwnd = hWnd;
	flash.dwTimeout = 0;
	flash.uCount = 0xffffffff;

	mute_state = CheckMenuItem(menu, IDM_MUTE, MF_UNCHECKED);
	if (mute_state == MF_UNCHECKED)
	{
		SetWindowText(hWnd, _T("MicMute - Mic Off"));
		CheckMenuItem(menu, IDM_MUTE, mute_state = MF_CHECKED);
		flash.dwFlags = FLASHW_TRAY;
		mixer_mic_out.SetMute(TRUE);
		mixer_line_out.SetMute(TRUE);
	}
	else 
	{
		SetWindowText(hWnd, _T("MicMute"));
		mute_state = MF_UNCHECKED;
		flash.dwFlags = FLASHW_STOP;
		mixer_mic_out.SetMute(FALSE);
		mixer_line_out.SetMute(FALSE);
	}

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
const TCHAR _help_text1[] = _T("Press \"Right Windows + Control\"");
const TCHAR _help_text2[] = _T("to mute or unmute microphone");

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

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
			Mute(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...

		hdc = GetDC(hWnd);
		TextOut(hdc, 5, 5, _help_text1, (sizeof(_help_text1) - 1)/ sizeof(TCHAR));
		TextOut(hdc, 5, 22, _help_text2, (sizeof(_help_text2) - 1)/ sizeof(TCHAR));
		ReleaseDC(hWnd, hdc);

		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
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
