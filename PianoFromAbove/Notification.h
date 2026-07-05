#pragma once

#include <mmdeviceapi.h>
constexpr auto DEVICECHANGE_AUDIO_ENDPOINT = 0x1000;

class AudioNotificationClient : public IMMNotificationClient
{
public:
    AudioNotificationClient() : m_cRef(1) {}

    // IUnknown
    ULONG STDMETHODCALLTYPE AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        ULONG ulRef = InterlockedDecrement(&m_cRef);

        if (ulRef == 0)
            delete this;

        return ulRef;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID** ppv)
    {
        if (riid == __uuidof(IUnknown) ||
            riid == __uuidof(IMMNotificationClient))
        {
            *ppv = (IMMNotificationClient*)this;
            AddRef();
            return S_OK;
        }

        *ppv = NULL;
        return E_NOINTERFACE;
    }

    // IMMNotificationClient
    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(
        EDataFlow flow,
        ERole role,
        LPCWSTR pwstrDefaultDeviceId);

    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR)
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR)
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR, DWORD)
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(
        LPCWSTR,
        const PROPERTYKEY)
    {
        return S_OK;
    }

private:
    LONG m_cRef;
};
