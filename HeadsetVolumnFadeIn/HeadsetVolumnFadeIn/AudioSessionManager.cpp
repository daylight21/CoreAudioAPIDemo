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
    // ��ʼ��COM���
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
    // ��ȡ�Ự������
    HRESULT hr = device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)&sessionManager);
    CHECK_HR_AND_NULLPTR_RETURN(hr, sessionManager, false, "Activate sessionManager failed");
    // ע���»Ự�����¼�������
    sessionCreatedNotifier = new (std::nothrow) NewSessionNotifier();
    CHECK_NULLPTR_RETURN(sessionCreatedNotifier, false, "sessionCreatedNotifier create failed!");
    hr = sessionManager->RegisterSessionNotification(sessionCreatedNotifier);
    CHECK_HR_RETURN(hr, false, "RegisterSessionNotification failed");
    // ��ȡ�Ựö����
    CComPtr<IAudioSessionEnumerator> sessionEnumerator = nullptr;
    hr = sessionManager->GetSessionEnumerator(&sessionEnumerator);
    CHECK_HR_AND_NULLPTR_RETURN(hr, sessionEnumerator, false, "Activate sessionManager failed");
    // ö�ٳ�ÿһ���Ự��Ϊÿһ���Ựע��һ���¼�������
    int sessionCount = 0;
    hr = sessionEnumerator->GetCount(&sessionCount);
    CHECK_HR_RETURN(hr, false, "sessionEnumerator GetCount failed");
    LOG_DEBUG("SessionCount = " + std::to_string(sessionCount));
    for (int i = 0; i < sessionCount; ++i) {
        CComPtr<IAudioSessionControl> sessionControl = nullptr;
        hr = sessionEnumerator->GetSession(i, &sessionControl);
        CHECK_HR_AND_NULLPTR_CONTINUE(hr, sessionEnumerator, "GetSession failed");
        auto sessionEventNotifier = new (std::nothrow) SessionEventNotifier();
        CHECK_NULLPTR_CONTINUE(sessionEventNotifier, "Create SessionEventNotifier Failed!");
        hr = sessionControl->RegisterAudioSessionNotification(sessionEventNotifier);
        CHECK_HR_CONTINUE(hr, "RegisterAudioSessionNotification failed");
        sessionPairs.emplace(sessionControl, sessionEventNotifier);
    }
    return true;
}