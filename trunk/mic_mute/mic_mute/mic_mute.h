#pragma once

#include "resource.h"
#include <windows.h>
#include <MMsystem.h>

class CMixer
{
public:
	enum DestKind {Play, Record};
private:
	DWORD m_dwVolControlID, m_dwMuteControlID, m_dwChannels, m_dwComponent;
	bool m_bOK;
	UINT m_dwDevNumber;
	DestKind m_dk;
public:
	static UINT DevCount()
	{
		return mixerGetNumDevs();
	}
	static HRESULT GetCaps(UINT _dev_number, MIXERCAPS * _caps)
	{
		HMIXER _mix;
		HRESULT _hr; 
		_hr = mixerOpen(&_mix, _dev_number, 0, 0, 0); 
		if (FAILED(_hr)) return _hr;  
		_hr = mixerGetDevCaps((UINT_PTR)_mix, _caps, sizeof(MIXERCAPS));
		if (FAILED(_hr)) return _hr;  
		_hr = mixerClose(_mix);
		if (FAILED(_hr)) return _hr;  

		return _hr;
	}
	bool SelectDevice(UINT Device)
	{
		if (!m_bOK)
		{
			return false;
		}
		if (Device >= DevCount())
		{
			return false;
		}
		return SelectComponent(m_dwComponent, m_dk, Device);
	}
	bool SelectComponent(DWORD ComponentType, DestKind dkKind, UINT Device = 0)
	{

		m_dwVolControlID = -1;
		m_bOK = false;
		m_dwChannels = 0;  
		m_dwDevNumber = Device;
		m_dwComponent = ComponentType;
		m_dk = dkKind;

		if (DevCount() < 1)
		{
			return m_bOK;
		}

		HMIXER hMixer;  
		HRESULT hr;  

		MIXERCAPS caps;
		hr = GetCaps(Device, &caps);
		if (FAILED(hr)) return m_bOK;  

		hr = mixerOpen(&hMixer, Device, 0, 0, 0);  
		if (FAILED(hr)) return m_bOK;  

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
				return m_bOK;  
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
				mxl.dwSource = 0;
				hr = mixerGetLineInfo((HMIXEROBJ)hMixer, &mxl,  
					MIXER_GETLINEINFOF_DESTINATION);
				if (mxl.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_WAVEIN)
				{
					mxl.dwComponentType = ComponentType;  
					// loop through the sources  
					count = mxl.cConnections;  
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
		m_bOK = true;
		return m_bOK;
	}
	CMixer::CMixer(): m_dwVolControlID(-1),  
		m_bOK(false), m_dwChannels(0)
	{

	}
	CMixer::CMixer(DWORD ComponentType, DestKind dkKind, UINT Device = 0): m_dwVolControlID(-1),  
		m_bOK(false), m_dwChannels(0)  

	{  
		SelectComponent(ComponentType, dkKind, Device);  
	}  

	void CMixer::SetMute(BOOL bMute)
	{
		if (!m_bOK) return;  
		if (m_dwMuteControlID == -1)
		{
			return;
		}
		HMIXER hMixer;  
		HRESULT hr;  
		hr = mixerOpen(&hMixer, m_dwDevNumber, 0, 0, 0);  
		if (FAILED(hr)) return;  
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
	BOOL CMixer::GetMute()
	{
		if (!m_bOK) return FALSE;
		if (m_dwMuteControlID == -1)
		{
			return FALSE;
		}
		MIXERCONTROLDETAILS_BOOLEAN mxdu;
		mxdu.fValue = FALSE;
		HMIXER hMixer;  
		HRESULT hr;  
		hr = mixerOpen(&hMixer, m_dwDevNumber, 0, 0, 0);  
		if (FAILED(hr)) return FALSE;  
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
		return mxdu.fValue; 
	}
	void CMixer::SetVolume(DWORD dwVol)  
	{  
		if (!m_bOK) return;  
		HMIXER hMixer;  
		HRESULT hr;  
		hr = mixerOpen(&hMixer, m_dwDevNumber, 0, 0, 0);  
		if (FAILED(hr)) return;  
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
	DWORD CMixer::GetVolume()  
	{  
		if (!m_bOK) return 0;  
		MIXERCONTROLDETAILS_UNSIGNED mxdu;
		mxdu.dwValue = 0;
		HMIXER hMixer;  
		HRESULT hr;  
		hr = mixerOpen(&hMixer, m_dwDevNumber, 0, 0, 0);  
		if (FAILED(hr)) return 0;  
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
		return mxdu.dwValue;  
	}
};