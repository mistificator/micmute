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

struct KeyHook_Struct
{
	WPARAM prev_code;
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
	__declspec(dllexport) LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		if (kstruct->enabled && (kstruct->mic_mode == MIC_MODE_STANDART) && (nCode >= 0) && KEY_WAS_UP)
		{
			if (((kstruct->prev_code == kstruct->key1) && (wParam == kstruct->key2) && (kstruct->keys_count == 2)) || ((wParam == kstruct->key1) && (kstruct->keys_count == 1))) 
			{
				SetEvent(Event);
				kstruct->prev_code = 0;
			}
			else
			{
				kstruct->prev_code = wParam;
			}
		}
		if (kstruct->enabled && (kstruct->mic_mode == MIC_MODE_TRANSMITTER) && (nCode >= 0) && (kstruct->keys_count == 1))
		{
			if ((wParam == kstruct->key1) && ((KEY_BEING_PRESSED && KEY_WAS_UP) || (KEY_BEING_RELEASED && KEY_WAS_DOWN)))
			{
				SetEvent(Event);
				kstruct->prev_code = 0;
			}
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

