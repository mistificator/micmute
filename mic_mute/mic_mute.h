#pragma once

#include "resource.h"
#include <windows.h>
#include <MMsystem.h>

class CMixer
{
private:
	DWORD m_dwVolControlID, m_dwMuteControlID, m_dwChannels;
	bool m_bOK;
	UINT m_dwRecId, m_dwPlayId;
	UINT devcount;
public:
	enum DestKind {Play, Record};
	CMixer::CMixer(DWORD ComponentType, DestKind dkKind): m_dwVolControlID(-1),  
		m_bOK(false), m_dwChannels(0)  

	{  
		devcount = mixerGetNumDevs();
		if (devcount < 1) 
		{
			return;
		}

		for (UINT _id = 0; _id < devcount; _id++)
		{
			HMIXER hMixer;  
			HRESULT hr;  

			hr = mixerOpen(&hMixer, _id, 0, 0, 0);  
			if (FAILED(hr)) continue;  

			MIXERCAPS caps;
			hr = mixerGetDevCaps((UINT_PTR)hMixer, &caps, sizeof(MIXERCAPS));
			if (FAILED(hr)) continue;
			MIXERLINE mxl;  
			MIXERCONTROL mc;  
			MIXERLINECONTROLS mxlc;  
			DWORD kind, count;  
			int item=-1;  
			if (dkKind == Play)  
			{  
				mxl.cbStruct = sizeof(mxl);  
				mxl.dwComponentType = ComponentType;  
				hr = mixerGetLineInfo((HMIXEROBJ)hMixer, &mxl,  
					MIXER_GETLINEINFOF_COMPONENTTYPE);  
				if (FAILED(hr))  
				{  
					mixerClose(hMixer);  
					continue;  
				}  
				kind = ComponentType;  
				item = mxl.dwSource;  
			}  
			else  
			{
				for (DWORD _idx = 0; _idx < caps.cDestinations; _idx++)
				{
					mxl.cbStruct = sizeof(mxl);  
					mxl.dwDestination = _idx;
					hr = mixerGetLineInfo((HMIXEROBJ)hMixer, &mxl,  
						MIXER_GETLINEINFOF_DESTINATION);
					if (mxl.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_WAVEIN)
					{
						mxl.dwComponentType = ComponentType;  
						// loop through the sources  
						count = 0xFFFF;  
						for(UINT i = 0; i < count; i++)  
						{  
							mxl.dwSource = i;  
							hr = mixerGetLineInfo((HMIXEROBJ)hMixer, &mxl,  
								MIXER_GETLINEINFOF_SOURCE);  
							if (FAILED(hr))  
								break;  
							if (mxl.dwComponentType == ComponentType)  
							{  
								item = i;  
								m_dwChannels = mxl.cChannels; 
								_idx = caps.cDestinations; // exit outer loop
								break;  
							}  
						}  
					}
				}
			}  
			if (item >= 0)  
			{  
				mc.cbStruct = sizeof(mc);  
				mxlc.cbStruct = sizeof(mxlc);  
				mxlc.dwLineID = mxl.dwLineID;  
				mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;  
				mxlc.cControls = 1;  
				mxlc.cbmxctrl = sizeof(MIXERCONTROL);  
				mxlc.pamxctrl = &mc;  
				hr = mixerGetLineControls((HMIXEROBJ)hMixer, &mxlc,  
					MIXER_GETLINECONTROLSF_ONEBYTYPE);  
				m_dwVolControlID = mc.dwControlID;  

				mc.cbStruct = sizeof(mc);  
				mxlc.cbStruct = sizeof(mxlc);  
				mxlc.dwLineID = mxl.dwLineID;  
				mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;  
				mxlc.cControls = 1;  
				mxlc.cbmxctrl = sizeof(MIXERCONTROL);  
				mxlc.pamxctrl = &mc;  
				hr = mixerGetLineControls((HMIXEROBJ)hMixer, &mxlc,  
					MIXER_GETLINECONTROLSF_ONEBYTYPE);
				if (FAILED(hr))
				{
					m_dwMuteControlID = -1;
				}
				else
				{
					m_dwMuteControlID = mc.dwControlID;  
				}

			}  
			mixerClose(hMixer);  
		}

		m_bOK = true;  
	}  
	int CMixer::GetDestination(HMIXER hMixer, DWORD ComponentType)  
	{  
		MIXERLINE mxl;  
		HRESULT hr;  
		mxl.cbStruct = sizeof(mxl);  
		mxl.dwComponentType = ComponentType;  
		hr = mixerGetLineInfo((HMIXEROBJ)hMixer, &mxl,  
			MIXER_GETLINEINFOF_COMPONENTTYPE);  
		if (FAILED(hr))  
		{  
			mixerClose(hMixer);  
			return -1;  
		}  
		return mxl.dwDestination;  
	}  
	void CMixer::SetMute(BOOL bMute)
	{
		if (!m_bOK) return;  
		for (UINT _id = 0; _id < devcount; _id++)
		{
			HMIXER hMixer;  
			HRESULT hr;  
			hr = mixerOpen(&hMixer, _id, 0, 0, 0);  
			if (FAILED(hr)) continue;  
			MIXERCONTROLDETAILS mxcd;  
			MIXERCONTROLDETAILS_BOOLEAN mxdu;  
			mxdu.fValue = bMute;  
			mxcd.cMultipleItems = 0;  
			mxcd.cChannels = 1;  
			mxcd.cbStruct = sizeof(mxcd);  
			mxcd.dwControlID = m_dwMuteControlID;  
			mxcd.cbDetails = sizeof(mxdu);  
			mxcd.paDetails = &mxdu;  
			hr = mixerSetControlDetails((HMIXEROBJ)hMixer, &mxcd,  
				MIXER_SETCONTROLDETAILSF_VALUE);  
			mixerClose(hMixer);  
		}
	}
	BOOL CMixer::GetMute()
	{
		if (!m_bOK) return FALSE;
		if (m_dwMuteControlID == -1)
		{
			return FALSE;
		}
		MIXERCONTROLDETAILS_BOOLEAN mxdu;
		mxdu.fValue = FALSE;
		for (UINT _id = 0; _id < devcount; _id++)
		{
			HMIXER hMixer;  
			HRESULT hr;  
			hr = mixerOpen(&hMixer, _id, 0, 0, 0);  
			if (FAILED(hr)) continue;  
			MIXERCONTROLDETAILS mxcd;  
			mxcd.cMultipleItems = 0;  
			mxcd.cChannels = m_dwChannels;  
			mxcd.cbStruct = sizeof(mxcd);  
			mxcd.dwControlID = m_dwMuteControlID;  
			mxcd.cbDetails = sizeof(mxdu);  
			mxcd.paDetails = &mxdu;  
			hr = mixerGetControlDetails((HMIXEROBJ)hMixer, &mxcd,  
				MIXER_GETCONTROLDETAILSF_VALUE);  
			mixerClose(hMixer);  
		}
		return mxdu.fValue; 
	}
	void CMixer::SetVolume(DWORD dwVol)  
	{  
		if (!m_bOK) return;  
		for (UINT _id = 0; _id < devcount; _id++)
		{
			HMIXER hMixer;  
			HRESULT hr;  
			hr = mixerOpen(&hMixer, _id, 0, 0, 0);  
			if (FAILED(hr)) continue;  
			MIXERCONTROLDETAILS mxcd;  
			MIXERCONTROLDETAILS_UNSIGNED mxdu;  
			mxdu.dwValue = dwVol;  
			mxcd.cMultipleItems = 0;  
			mxcd.cChannels = 1;  
			mxcd.cbStruct = sizeof(mxcd);  
			mxcd.dwControlID = m_dwVolControlID;  
			mxcd.cbDetails = sizeof(mxdu);  
			mxcd.paDetails = &mxdu;  
			hr = mixerSetControlDetails((HMIXEROBJ)hMixer, &mxcd,  
				MIXER_SETCONTROLDETAILSF_VALUE);  
			mixerClose(hMixer);  
		}
	}  
	DWORD CMixer::GetVolume()  
	{  
		if (!m_bOK) return 0;  
		MIXERCONTROLDETAILS_UNSIGNED mxdu;
		mxdu.dwValue = 0;
		for (UINT _id = 0; _id < devcount; _id++)
		{
			HMIXER hMixer;  
			HRESULT hr;  
			hr = mixerOpen(&hMixer, _id, 0, 0, 0);  
			if (FAILED(hr)) continue;  
			MIXERCONTROLDETAILS mxcd;  
			mxcd.cMultipleItems = 0;  
			mxcd.cChannels = m_dwChannels;  
			mxcd.cbStruct = sizeof(mxcd);  
			mxcd.dwControlID = m_dwVolControlID;  
			mxcd.cbDetails = sizeof(mxdu);  
			mxcd.paDetails = &mxdu;  
			hr = mixerGetControlDetails((HMIXEROBJ)hMixer, &mxcd,  
				MIXER_GETCONTROLDETAILSF_VALUE);  
			mixerClose(hMixer);  
		}
		return mxdu.dwValue;  
	}
};