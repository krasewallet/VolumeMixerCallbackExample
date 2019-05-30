#pragma once
#include <Windows.h>
#include <atlbase.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <Psapi.h>
#include <tchar.h>
#include <string>

using namespace std;

class CVolumeMonitor : IMMNotificationClient, IAudioSessionNotification, IAudioEndpointVolumeCallback {
public:
	CVolumeMonitor();
	~CVolumeMonitor();

	HRESULT Initialize();
	HRESULT Dispose();
	void setMute(BOOL isMute);

	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();
private:
	// IMMNotificationClient (only need to really implement OnDefaultDeviceChanged)
	IFACEMETHODIMP OnDeviceStateChanged(LPCWSTR /*pwstrDeviceId*/, DWORD /*dwNewState*/);// {   return S_OK;    }
	IFACEMETHODIMP OnDeviceAdded(LPCWSTR /*pwstrDeviceId*/);// {   return S_OK;    }
	IFACEMETHODIMP OnDeviceRemoved(LPCWSTR /*pwstrDeviceId*/);
	IFACEMETHODIMP OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId);   // ****
	IFACEMETHODIMP OnPropertyValueChanged(LPCWSTR /*pwstrDeviceId*/, const PROPERTYKEY /*key*/);// {   return S_OK;    }
	// IAudioEndpointVolumeCallback
	IFACEMETHODIMP OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA NotificationData);
	// IAudioSessionNotification
	IFACEMETHODIMP OnSessionCreated(IAudioSessionControl *pNewSession);

	// IUnknown
	IFACEMETHODIMP QueryInterface(const IID& iid, void** ppUnk);

	CComPtr<IMMDeviceEnumerator>		m_spEnumerator;
	CComPtr<IMMDevice>					m_spDevice;
	CComPtr<IAudioEndpointVolume>		m_spEndpointVolume;
	CComPtr<ISimpleAudioVolume>			m_spVolumeControl;
	CComPtr<IAudioSessionManager2>		m_manager;
	CComPtr<IAudioSessionEnumerator>	m_sessionEnumerator;

	int	sessionCount;
	BOOL	m_mute;
	long m_cRef;
	wstring m_nameAudioMessenger;
};
