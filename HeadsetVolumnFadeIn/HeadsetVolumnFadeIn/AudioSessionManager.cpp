#include "pch.h"
#include "AudioSessionManager.h"
#include "CommonMacro.h"
#include "Logger.h"

AudioSessionManager::AudioSessionManager()
{
    LOG_DEBUG("AudioSessionManager ctor");
}


AudioSessionManager::~AudioSessionManager()
{
    LOG_DEBUG("AudioSessionManager dtor");
    if (initFlag) {
        LOG_DEBUG("CoUninitialize Begin");
        CoUninitialize();
        LOG_DEBUG("CoUninitialize End");
    }
}

bool AudioSessionManager::Init()
{
    LOG_DEBUG("AudioSessionManager Init");
    if (enumerator != nullptr) {
        LOG_WARNING("Enumerator already initialized!");
    }
    // 初始化COM组件
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (hr == S_FALSE) {
        LOG_ERROR("Com already initialized!");
    } else {
        initFlag = true;
    }
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&enumerator);
    CHECK_HR_AND_NULLPTR_RETURN(hr, enumerator, false, "CoCreateInstance failed");
    hr = enumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &device);
    CHECK_HR_AND_NULLPTR_RETURN(hr, device, false, "GetDefaultAudioEndpoint failed");
    if (!RegNotifierForSessions()) {
        LOG_ERROR("RegNotifierForSessions Failed!");
    }
    return true;
}

bool AudioSessionManager::Uninit()
{
    HRESULT hr = sessionManager->UnregisterSessionNotification(sessionCreatedNotifier);
    if (hr != S_OK) {
        LOG_ERROR("UnregisterSessionNotification failed!");
    }
    sessionCreatedNotifier = nullptr;
    sessionManager = nullptr;
    for (auto ite : sessionPairs) {
        hr = ite.first->UnregisterAudioSessionNotification(ite.second);
        CHECK_HR_CONTINUE(hr, "UnregisterAudioSessionNotification Failed!");
    }
    sessionPairs.clear();
    device = nullptr;
    enumerator = nullptr;
    return true;
}

bool AudioSessionManager::RegNotifierForSessions()
{
    // 获取会话管理器
    HRESULT hr = device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)&sessionManager);
    CHECK_HR_AND_NULLPTR_RETURN(hr, sessionManager, false, "Activate sessionManager failed");
    if (!RegForNewSessionNotifier()) {
        LOG_ERROR("RegForNewSessionNotifier failed!");
        return false;
    }
    // 获取会话枚举器
    CComPtr<IAudioSessionEnumerator> sessionEnumerator = nullptr;
    hr = sessionManager->GetSessionEnumerator(&sessionEnumerator);
    CHECK_HR_AND_NULLPTR_RETURN(hr, sessionEnumerator, false, "Activate sessionManager failed");
    // 枚举出每一个会话并为每一个会话注册一个事件监听器
    int sessionCount = 0;
    hr = sessionEnumerator->GetCount(&sessionCount);
    CHECK_HR_RETURN(hr, false, "sessionEnumerator GetCount failed");
    LOG_DEBUG("SessionCount = " + std::to_string(sessionCount));
    for (int i = 0; i < sessionCount; ++i) {
        CComPtr<IAudioSessionControl> sessionControl = nullptr;
        hr = sessionEnumerator->GetSession(i, &sessionControl);
        CHECK_HR_AND_NULLPTR_CONTINUE(hr, sessionEnumerator, "GetSession failed");
        if (!RegNotifierForSingleSession(sessionControl)) {
            LOG_ERROR("RegNotifierForSingleSession Failed!");
            continue;
        }
    }
    return true;
}

bool AudioSessionManager::RegForNewSessionNotifier()
{
    // 注册新会话创建事件监听器
    sessionCreatedNotifier = new (std::nothrow) NewSessionNotifier();
    CHECK_NULLPTR_RETURN(sessionCreatedNotifier, false, "sessionCreatedNotifier create failed!");
    sessionCreatedNotifier->RegisterSessionCreateCallback(std::bind(&AudioSessionManager::OnSessionCreated, this, std::placeholders::_1));
    HRESULT hr = sessionManager->RegisterSessionNotification(sessionCreatedNotifier);
    CHECK_HR_RETURN(hr, false, "RegisterSessionNotification failed");
    return true;
}

bool AudioSessionManager::RegNotifierForSingleSession(CComPtr<IAudioSessionControl> session)
{
    CHECK_NULLPTR_RETURN(session, false, "Session is Null!");
    auto sessionEventNotifier = new (std::nothrow) SessionEventNotifier();
    CHECK_NULLPTR_RETURN(sessionEventNotifier, false, "Create SessionEventNotifier Failed!");
    sessionEventNotifier->RegisterSessionStateChangeCallback(std::bind(&AudioSessionManager::OnSessionStateChange, this, std::placeholders::_1));
    HRESULT hr = session->RegisterAudioSessionNotification(sessionEventNotifier);
    CHECK_HR_RETURN(hr, false, "RegisterAudioSessionNotification failed");
    sessionPairs.emplace(session, sessionEventNotifier);
    return true;
}

void AudioSessionManager::OnSessionCreated(CComPtr<IAudioSessionControl> session)
{
    RegNotifierForSingleSession(session);
}

void AudioSessionManager::OnSessionStateChange(AudioSessionState state)
{
    if (state == AudioSessionStateActive) {
        // TODO 判断音量，并作出调整
        LOG_DEBUG("Session Active");
    }
}
