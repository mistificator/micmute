#pragma once

#define MIC_MODE_STANDART 0
#define MIC_MODE_TRANSMITTER 1

extern "C"
{
	typedef  void (__stdcall * _SetShortCut)(int, int, int);
	typedef  void (__stdcall * _GetShortCut)(int *, int *, int *);
	typedef  void (__stdcall * _SetEnabled)(bool);
	typedef  void (__stdcall * _SetMode)(int);
};