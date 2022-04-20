#include "pch.h"
#include "NewSessionNotifier.h"
#include "Logger.h"

NewSessionNotifier::NewSessionNotifier()
{
    LOG_DEBUG("NewSessionNotifier Created");
}

NewSessionNotifier::~NewSessionNotifier()
{
    LOG_DEBUG("NewSessionNotifier Destroyed");
}

HRESULT __stdcall NewSessionNotifier::QueryInterface(REFIID riid, void** ppvObject)
{
    if (IID_IUnknown == riid) {
        AddRef();
        *ppvObject = static_cast<IUnknown*>(this);
    }
    else if (__uuidof(IAudioSessionEvents) == riid) {
        AddRef();
        *ppvObject = static_cast<IAudioSessionNotification*>(this);
    }
    else {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
    return S_OK;
}

ULONG __stdcall NewSessionNotifier::AddRef(void)
{
    InterlockedIncrement(&ref);
    LOG_DEBUG("NewSessionNotifier AddRef, ref = " + std::to_string(ref));
    return ref;
}

ULONG __stdcall NewSessionNotifier::Release(void)
{
    InterlockedDecrement(&ref);
    LOG_DEBUG("NewSessionNotifier ReleaseRef, ref = " + std::to_string(ref));
    if (ref == 0) {
        delete this;
    }
    return ref;
}

HRESULT __stdcall NewSessionNotifier::OnSessionCreated(IAudioSessionControl* NewSession)
{
    LOG_DEBUG("OnSessionCreated Occured");
    if (sessionCreatedCallback != nullptr) {
        sessionCreatedCallback(NewSession);
    }
    return S_OK;
}
