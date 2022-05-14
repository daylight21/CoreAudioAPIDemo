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
    sessionMap.clear();
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
    // 获取sessionID
    CComPtr<IAudioSessionControl2> sessionControl2 = nullptr;
    HRESULT hr = session->QueryInterface(&sessionControl2);
    CHECK_HR_AND_NULLPTR_RETURN(hr, sessionControl2, false, "Get SessionControl2 Failed!");
    LPWSTR id = {0};
    hr = sessionControl2->GetSessionInstanceIdentifier(&id);
    CHECK_HR_RETURN(hr, false, "GetSessionInstanceIdentifier Failed!");
    std::wstring sessionId(id);
    // 注册音频流状态改变事件回调
    auto sessionEventNotifier = new (std::nothrow) SessionEventNotifier(sessionId);
    CHECK_NULLPTR_RETURN(sessionEventNotifier, false, "Create SessionEventNotifier Failed!");
    sessionMap.emplace(sessionId, sessionControl2);
    sessionEventNotifier->RegisterSessionStateChangeCallback(std::bind(&AudioSessionManager::OnSessionStateChange, this, std::placeholders::_1, std::placeholders::_2));
    hr = sessionControl2->RegisterAudioSessionNotification(sessionEventNotifier);
    CHECK_HR_RETURN(hr, false, "RegisterAudioSessionNotification failed");
    sessionPairs.emplace(sessionControl2, sessionEventNotifier);
    return true;
}

void AudioSessionManager::OnSessionCreated(CComPtr<IAudioSessionControl> session)
{
    RegNotifierForSingleSession(session);
}

void AudioSessionManager::OnSessionStateChange(AudioSessionState state, const std::wstring& sessionId)
{
    switch (state) {
    case AudioSessionStateActive:
        DealWithSessionActive(sessionId);
        break;
    case AudioSessionStateInactive:
        DealWithSessionInactive(sessionId);
        break;
    case AudioSessionStateExpired:
        DealWithSessionExpired(sessionId);
        break;
    default:
        break;
    }
}

void AudioSessionManager::DealWithSessionActive(const std::wstring& sessionId)
{
    
    // TODO 判断音量，并作出调整
    LOG_DEBUG("Session Active!");
    // 需要通过SessionId知道当前Active的Session是哪个
    LOG_DEBUG(L"SessionID = " + sessionId);
    CHECK_MAP_CONTAINS_KEY_RETURN(sessionMap, sessionId);
    // 获取音量
    CComPtr<ISimpleAudioVolume> simpleVolume = nullptr;
    HRESULT hr = sessionMap[sessionId]->QueryInterface(&simpleVolume);
    CHECK_HR_AND_NULLPTR(hr, simpleVolume, "Get ISimpleAudioVolume Failed!");
    CComPtr<IChannelAudioVolume> channelVolume = nullptr;
    hr = sessionMap[sessionId]->QueryInterface(&channelVolume);
    CHECK_HR_AND_NULLPTR(hr, simpleVolume, "Get IChannelAudioVolume Failed!");
    float volume{ 0.0f };
    hr = simpleVolume->GetMasterVolume(&volume);
    CHECK_HR(hr, "GetMasterVolume Failed!");
    LOG_DEBUG("This session masterVolume = " + std::to_string(volume));
    hr = channelVolume->GetChannelVolume(0, &volume);
    CHECK_HR(hr, "GetChannelVolume Failed!");
    LOG_DEBUG("This session ChannelVolume = " + std::to_string(volume));
    CComPtr<IAudioStreamVolume> streamVolume = nullptr;
    hr = sessionMap[sessionId]->QueryInterface(&streamVolume);
    CHECK_HR_AND_NULLPTR(hr, streamVolume, "Get IAudioStreamVolume Failed!");
    hr = streamVolume->GetChannelVolume(1, &volume);
    CHECK_HR(hr, "GetChannelVolume Failed!");
    LOG_DEBUG("This session StreamVolume = " + std::to_string(volume));
}

void AudioSessionManager::DealWithSessionInactive(const std::wstring& sessionId)
{
    LOG_DEBUG("Session Inactive!");
}

void AudioSessionManager::DealWithSessionExpired(const std::wstring& sessionId)
{
    LOG_DEBUG("Session Expired!");
}
