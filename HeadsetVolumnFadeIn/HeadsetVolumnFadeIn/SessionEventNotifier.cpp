#include "pch.h"
#include "SessionEventNotifier.h"
#include "Logger.h"

SessionEventNotifier::SessionEventNotifier(const std::wstring& id) : sessionId(id)
{
    LOG_DEBUG("SessionEventNotifier Created");
}

SessionEventNotifier::~SessionEventNotifier()
{
    LOG_DEBUG("SessionEventNotifier Destroyed");
}

HRESULT __stdcall SessionEventNotifier::QueryInterface(REFIID riid, void **ppvObject)
{
    if (IID_IUnknown == riid) {
        AddRef();
        *ppvObject = static_cast<IUnknown*>(this);
    } else if (__uuidof(IAudioSessionEvents) == riid) {
        AddRef();
        *ppvObject = static_cast<IAudioSessionEvents*>(this);
    } else {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
    return S_OK;
}

ULONG __stdcall SessionEventNotifier::AddRef(void)
{
    InterlockedIncrement(&ref);
    LOG_DEBUG("SessionEventNotifier AddRef, ref = " + std::to_string(ref));
    return ref;
}

ULONG __stdcall SessionEventNotifier::Release(void)
{
    InterlockedDecrement(&ref);
    LOG_DEBUG("SessionEventNotifier ReleaseRef, ref = " + std::to_string(ref));
    if (ref == 0) {
        delete this;
    }
    return ref;
}

HRESULT __stdcall SessionEventNotifier::OnDisplayNameChanged(LPCWSTR NewDisplayName, LPCGUID EventContext)
{
    LOG_DEBUG("OnDisplayNameChanged Occured");
    return S_OK;
}

HRESULT __stdcall SessionEventNotifier::OnIconPathChanged(LPCWSTR NewIconPath, LPCGUID EventContext)
{
    LOG_DEBUG("OnIconPathChanged Occured");
    return S_OK;
}

HRESULT __stdcall SessionEventNotifier::OnSimpleVolumeChanged(float NewVolume, BOOL NewMute, LPCGUID EventContext)
{
    LOG_DEBUG("OnSimpleVolumeChanged Occured");
    return S_OK;
}

HRESULT __stdcall SessionEventNotifier::OnChannelVolumeChanged(DWORD ChannelCount, float NewChannelVolumeArray[], DWORD ChangedChannel, LPCGUID EventContext)
{
    LOG_DEBUG("OnChannelVolumeChanged Occured");
    return S_OK;
}

HRESULT __stdcall SessionEventNotifier::OnGroupingParamChanged(LPCGUID NewGroupingParam, LPCGUID EventContext)
{
    LOG_DEBUG("OnGroupingParamChanged Occured");
    return S_OK;
}

HRESULT __stdcall SessionEventNotifier::OnStateChanged(AudioSessionState NewState)
{
    LOG_DEBUG("OnStateChanged Occured");
    if (sessionStateChangeCallback != nullptr) {
        sessionStateChangeCallback(NewState, sessionId);
    }
    return S_OK;
}

HRESULT __stdcall SessionEventNotifier::OnSessionDisconnected(AudioSessionDisconnectReason DisconnectReason)
{
    LOG_DEBUG("OnSessionDisconnected Occured");
    return S_OK;
}

void SessionEventNotifier::RegisterSessionStateChangeCallback(const SessionStateChangeCallback& cb)
{
    sessionStateChangeCallback = cb;
}
