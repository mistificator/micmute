// key_hook.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif

HANDLE Event = 0;
WPARAM PrevCode = 0;

extern "C"
{
	__declspec(dllexport) LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		if ((nCode >= 0) && ((lParam & (1 << 30)) == (1 << 30)))
		{
			if ((PrevCode == VK_RWIN) && (wParam == VK_CONTROL)) 
			{
				SetEvent(Event);
				PrevCode = 0;
			}
			else
			{
				PrevCode = wParam;
			}
		}
		return CallNextHookEx(0, nCode, wParam,lParam);
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
	}
	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		CloseHandle(Event);
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

