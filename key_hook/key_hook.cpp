// key_hook.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif

static HANDLE Event = 0;

struct KeyHook_Struct
{
	WPARAM PrevCode;
	int keys_count;
	int key1;
	int key2;
	BOOL enabled;
};

KeyHook_Struct * kstruct;
HANDLE map_handle;

extern "C"
{
	__declspec(dllexport) LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		if (kstruct->enabled && (nCode >= 0) && ((lParam & (1 << 30)) == 0))
		{
			if (((kstruct->PrevCode == kstruct->key1) && (wParam == kstruct->key2) && (kstruct->keys_count == 2)) || ((wParam == kstruct->key1) && (kstruct->keys_count == 1))) 
			{
				SetEvent(Event);
				kstruct->PrevCode = 0;
			}
			else
			{
				kstruct->PrevCode = wParam;
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
};

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		Event = CreateEvent(NULL, TRUE, FALSE, _T("Hooked!"));
//		MessageBox(0, _T("key_hook.dll attached"), _T("message"), MB_OK);
		map_handle = CreateFileMapping(
			INVALID_HANDLE_VALUE,    // use paging file
			NULL,                    // default security 
			PAGE_READWRITE,          // read/write access
			0,                       // max. object size 
			sizeof(KeyHook_Struct),                // buffer size  
			_T("KeyHook_Struct!"));                 // name of mapping object

		kstruct = (KeyHook_Struct *)MapViewOfFile(map_handle,   // handle to map object
			FILE_MAP_ALL_ACCESS, // read/write permission
			0,                   
			0,                   
			sizeof(KeyHook_Struct));   
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

