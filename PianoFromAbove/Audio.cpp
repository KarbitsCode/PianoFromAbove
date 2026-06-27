#include <Windows.h>
#include <string>
#include "Globals.h"
#include "MainProcs.h"
#include "Audio.h"

HRESULT AudioNotificationClient::OnDefaultDeviceChanged(
    EDataFlow flow,
    ERole role,
    LPCWSTR pwstrDefaultDeviceId)
{
    // Just so it won't reload 4 times
    if (flow != eRender || role != eMultimedia || pwstrDefaultDeviceId == NULL)
        return S_OK;

    PostMessage(g_hWnd, WM_DEVICECHANGE, DEVICECHANGE_AUDIO_ENDPOINT, 0);
    return S_OK;
}
