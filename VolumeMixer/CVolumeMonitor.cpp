#include "stdafx.h"
#include "CVolumeMonitor.h"

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

wstring GetProcessNameW(DWORD PID)
{
	wstring strName;
	size_t cchSize = 512;
	TCHAR szProcessName[512];
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
		FALSE, PID);

	if (hProcess == NULL) {
		_tcscpy_s(szProcessName, cchSize, TEXT(""));
	}
	else if (GetModuleFileNameEx(hProcess, (HMODULE)0, szProcessName, cchSize) == 0) {
		if (!GetProcessImageFileName(hProcess, szProcessName, cchSize)) {
			_tcscpy_s(szProcessName, cchSize, TEXT(""));
		}
	}
	strName = szProcessName;
	int pos = strName.find_last_of('\\');
	strName = strName.substr(pos + 1, strName.length() - pos - 1);
	CloseHandle(hProcess);
	return strName;
}

CVolumeMonitor::CVolumeMonitor() : m_cRef(1), m_nameAudioMessenger(L"ifly.qzk.AudioMessenger.exe")
{
}


CVolumeMonitor::~CVolumeMonitor()
{
}

HRESULT CVolumeMonitor::Initialize()
{
	HRESULT hr;

	// create enumerator
	hr = m_spEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));
	if (SUCCEEDED(hr))
	{
		hr = m_spEnumerator->RegisterEndpointNotificationCallback((IMMNotificationClient*)this);
		if (SUCCEEDED(hr))
		{
			m_spEnumerator->GetDefaultAudioEndpoint(EDataFlow::eRender,ERole::eMultimedia, &m_spDevice);
			m_spDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)&m_manager);
			m_manager->RegisterSessionNotification(this);
			m_spDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&m_spEndpointVolume);
			m_spEndpointVolume->RegisterControlChangeNotify(this);
		}
	}
	return hr;
}

HRESULT CVolumeMonitor::Dispose()
{
	m_spEndpointVolume->UnregisterControlChangeNotify(this);
	m_spEnumerator->UnregisterEndpointNotificationCallback(this);
	return S_OK;
}

IFACEMETHODIMP_(ULONG) CVolumeMonitor::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

IFACEMETHODIMP_(ULONG) CVolumeMonitor::Release()
{
	long lRef = InterlockedDecrement(&m_cRef);
	if (lRef == 0)
	{
		delete this;
	}
	return lRef;
}

IFACEMETHODIMP CVolumeMonitor::OnDeviceStateChanged(LPCWSTR, DWORD)
{
	return S_OK;
}

IFACEMETHODIMP CVolumeMonitor::OnDeviceAdded(LPCWSTR)
{
	return S_OK;
}

IFACEMETHODIMP CVolumeMonitor::OnDeviceRemoved(LPCWSTR)
{
	return S_OK;
}

IFACEMETHODIMP CVolumeMonitor::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId)
{
	return S_OK;
}

IFACEMETHODIMP CVolumeMonitor::OnPropertyValueChanged(LPCWSTR, const PROPERTYKEY)
{
	return S_OK;
}

IFACEMETHODIMP CVolumeMonitor::OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA NotificationData)
{
	UINT volume = (UINT)((NotificationData->fMasterVolume) * 100);
	TCHAR s[5];
	_itow_s(volume, s, 10);
	wstring msg = s;
	msg += L'\n';
	OutputDebugString(msg.c_str());
	return S_OK;
}

IFACEMETHODIMP CVolumeMonitor::OnSessionCreated(IAudioSessionControl *pNewSession)
{
	CComPtr<IAudioSessionControl2> ctrl2;
	HRESULT hr = pNewSession->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&ctrl2);

	if (SUCCEEDED(hr))
	{
		hr = ctrl2->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&m_spVolumeControl);

		if (SUCCEEDED(hr))
		{
			m_spVolumeControl->SetMute(m_mute, NULL);
		}	
	}
	return hr;
}

IFACEMETHODIMP CVolumeMonitor::QueryInterface(const IID & iid, void ** ppUnk)
{
	if ((iid == __uuidof(IUnknown)) ||
		(iid == __uuidof(IMMNotificationClient)))
	{
		*ppUnk = static_cast<IMMNotificationClient*>(this);
	}
	else if (iid == __uuidof(IAudioSessionNotification))
	{
		*ppUnk = static_cast<IAudioSessionNotification*>(this);
	}
	else if (iid == __uuidof(IAudioEndpointVolumeCallback)) {
		*ppUnk = static_cast<IAudioEndpointVolumeCallback*>(this);
	}
	else
	{
		*ppUnk = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

void CVolumeMonitor::setMute(BOOL isMute)
{
	HRESULT hr;

	m_manager->GetSessionEnumerator(&m_sessionEnumerator);
	// Get the session count
	m_sessionEnumerator->GetCount(&sessionCount);
	// Loop through all sessions
	for (int i = 0; i < sessionCount; i++)
	{
		CComPtr<IAudioSessionControl> ctrl;
		CComPtr<IAudioSessionControl2> ctrl2;
		DWORD processId = 0;

		hr = m_sessionEnumerator->GetSession(i, &ctrl);

		if (FAILED(hr))
		{
			continue;
		}

		hr = ctrl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&ctrl2);

		if (FAILED(hr))
		{
			continue;
		}
		hr = ctrl2->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&m_spVolumeControl);

		if (FAILED(hr))
		{
			continue;
		}
		hr = ctrl2->GetProcessId(&processId);
		wstring name = GetProcessNameW(processId);
		
		m_spVolumeControl->SetMute(isMute, NULL);
	}
	m_mute = isMute;
}
