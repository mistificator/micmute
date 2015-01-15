// key_hook.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

static HANDLE Event = 0;

#define KEY_WAS_UP ((lParam & (1 << 30)) == 0)
#define KEY_WAS_DOWN ((lParam & (1 << 30)) == (1 << 30))
#define KEY_BEING_PRESSED ((lParam & (1 << 31)) == 0)
#define KEY_BEING_RELEASED ((lParam & (1 << 31)) == (1 << 31))
#define KEY_REPEAT_COUNT (lParam & 0xffff)
#define KEY_IS_ALT ((lParam & (1 << 29)) == (1 << 29))

#define KEY_WAS_UP_LL (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
#define KEY_WAS_DOWN_LL (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
#define KEY_BEING_PRESSED_LL (hs->flags & (1 << 7)) == 0)
#define KEY_BEING_RELEASED_LL ((hs->flags & (1 << 7)) == (1 << 7))
#define KEY_IS_ALT_LL ((hs->flags & (1 << 5)) == (1 << 5))

struct KeyHook_Struct
{
	WPARAM prev_code;
	UINT vkCode;
	int keys_count;
	int key1;
	int key2;
	BOOL enabled;
	int mic_mode;
};

KeyHook_Struct * kstruct;
HANDLE map_handle;

extern "C"
{
	__declspec(dllexport) LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		if (nCode == HC_ACTION)
		{
			KBDLLHOOKSTRUCT * hs = (KBDLLHOOKSTRUCT *)lParam;
			kstruct->vkCode = hs->vkCode;
			if (kstruct->enabled && (kstruct->mic_mode == MIC_MODE_STANDART) && KEY_WAS_UP_LL)
			{
				if (((kstruct->prev_code == kstruct->key1) && (hs->vkCode == kstruct->key2) && (kstruct->keys_count == 2)) || ((hs->vkCode == kstruct->key1) && (kstruct->keys_count == 1))) 
				{
					SetEvent(Event);
					kstruct->prev_code = 0;
				}
				else
				{
					kstruct->prev_code = hs->vkCode;
				}
			}
			if (kstruct->enabled && (kstruct->mic_mode == MIC_MODE_TRANSMITTER) && (kstruct->keys_count == 1))
			{
				if (hs->vkCode == kstruct->key1 && ((KEY_WAS_UP_LL && kstruct->prev_code == 0) || (KEY_WAS_DOWN_LL && kstruct->prev_code != 0)))
				{
					SetEvent(Event);
					kstruct->prev_code = KEY_WAS_DOWN_LL ? 0 : hs->vkCode;
				}
			}
			return 0;
		}
		return CallNextHookEx(0, nCode, wParam,lParam);
	}

	__declspec(dllexport) void __stdcall SetShortcut(int _count, int _key1, int _key2)
	{
		if (_count > 2)
		{
			_count = 2;
		}
		if (_count < 1)
		{
			_count = 1;
		}
		kstruct->keys_count = _count;
		kstruct->key1 = _key1;
		kstruct->key2 = _key2; 
	}
	__declspec(dllexport) void __stdcall GetShortcut(int * _count, int * _key1, int * _key2)
	{
		* _count = kstruct->keys_count;
		* _key1 = kstruct->key1;
		* _key2 = kstruct->key2;
	}
	__declspec(dllexport) void __stdcall SetEnabled(bool _state)
	{
		kstruct->enabled = _state;
	}
	__declspec(dllexport) void __stdcall SetMode(int _mode)
	{
		kstruct->mic_mode = _mode;
	}
	__declspec(dllexport) UINT __stdcall GetVkCode()
	{
		return kstruct->vkCode;
	}
};

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		Event = CreateEvent(NULL, TRUE, FALSE, _T("Hooked!"));
		map_handle = CreateFileMapping(
			INVALID_HANDLE_VALUE,    // use paging file
			NULL,                    // default security 
			PAGE_READWRITE,          // read/write access
			0,                       // max. object size 
			sizeof(KeyHook_Struct),                // buffer size  
			_T("KeyHook_Struct!"));                 // name of mapping object

		bool _init_struct = (GetLastError() != ERROR_ALREADY_EXISTS);

		kstruct = (KeyHook_Struct *)MapViewOfFile(map_handle,   // handle to map object
			FILE_MAP_ALL_ACCESS, // read/write permission
			0,                   
			0,                   
			sizeof(KeyHook_Struct));

		if (_init_struct)
		{
			ZeroMemory(kstruct, sizeof(KeyHook_Struct));
			kstruct->enabled = true;
		}
	}
	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		UnmapViewOfFile(kstruct);
		CloseHandle(map_handle);
		CloseHandle(Event);
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

