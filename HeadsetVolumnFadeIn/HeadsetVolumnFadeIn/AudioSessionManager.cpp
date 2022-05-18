#include "pch.h"
#include "AudioSessionManager.h"
#include "CommonMacro.h"
#include "Logger.h"
#include <avrt.h>
#include <thread>

namespace {
    constexpr long long REFTIMES_PER_SEC = 10000000;
    constexpr long long REFTIMES_PER_MILLSEC = 10000;
}

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
        return false;
    }
    return true;
}

bool AudioSessionManager::Uninit()
{
    state = false;
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

void AudioSessionManager::ListenSessionLoopback(std::atomic_bool& state)
{
    LOG_DEBUG("ENTER ListenSessionLoopback");
    CComPtr<IAudioClient> audioClient{ nullptr };
    HRESULT hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&audioClient);
    CHECK_HR_AND_NULLPTR(hr, audioClient, "Activate audioClient failed");
    // ��ȡ������ʽ
    WAVEFORMATEX* waveFormat = nullptr;
    hr = audioClient->GetMixFormat(&waveFormat);
    CHECK_HR_AND_NULLPTR(hr, waveFormat, "GetMixFormat failed");
    // ��ʼ��AudioClient
    hr = audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        0,
        0,
        waveFormat,
        NULL);
    CHECK_HR(hr, "Initialize audioClient!");

    // ��ȡ����ʱ��
    auto requestedDuration = static_cast<REFERENCE_TIME>(REFTIMES_PER_SEC);
    UINT32 bufferFrameNums{ 0 };
    hr = audioClient->GetBufferSize(&bufferFrameNums);
    CHECK_HR(hr, "GetBufferSize Failed!");
    auto actualDuration = static_cast<REFERENCE_TIME>(REFTIMES_PER_SEC * bufferFrameNums / waveFormat->nSamplesPerSec);

    CComPtr<IAudioCaptureClient> captureClient{ nullptr };
    hr = audioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&captureClient);
    CHECK_HR_AND_NULLPTR(hr, captureClient, "GetService failed");
    // �����Ƶ�����̵߳����ȼ�
    DWORD taskIndex{ 0 };
    HANDLE audioTaskHandle = AvSetMmThreadCharacteristics(L"Audio", &taskIndex);
    CHECK_NULLPTR(audioTaskHandle, "AvSetMmThreadCharacteristics Failed");
    // ��ʼ���Loopback����
    hr = audioClient->Start();
    CHECK_HR(hr, "Start Failed!");
    bool started = false;
    // ʹ��CreateEvent����ͬ��Ҳ����
    UINT32 nextPackerSize{ 0 };
    BYTE* loopbackData{ nullptr };
    UINT32 numFramesToRead{ 0 };
    DWORD dwFlags{ 0 };
    while (state) {
        Sleep(actualDuration / REFTIMES_PER_MILLSEC / 2);
        hr = captureClient->GetNextPacketSize(&nextPackerSize);
        CHECK_HR_BREAK(hr, "GetNextPacketSize Failed!");
        while (nextPackerSize != 0) {
            hr = captureClient->GetBuffer(
                &loopbackData,
                &numFramesToRead,
                &dwFlags,
                nullptr,
                nullptr
            );
            CHECK_HR_BREAK(hr, "captureClient GetBuffer Failed!");
            if (dwFlags & AUDCLNT_BUFFERFLAGS_SILENT) {
                loopbackData = NULL;
            }
            OnCaptureLoopbackData(loopbackData, numFramesToRead * waveFormat->nBlockAlign);
            captureClient->ReleaseBuffer(numFramesToRead);
            hr = captureClient->GetNextPacketSize(&nextPackerSize);
            CHECK_HR_BREAK(hr, "GetNextPacketSize Failed!");
        }
    }
    if (audioTaskHandle != nullptr) {
        AvRevertMmThreadCharacteristics(audioTaskHandle);
        audioTaskHandle = nullptr;
    }
    // TODO ��Դ����й¶
    if (waveFormat != nullptr) {
        CoTaskMemFree(waveFormat);
        waveFormat = nullptr;
    }
    if (audioClient != nullptr) {
        if (started) {
            audioClient->Stop();
        }
    }
    LOG_DEBUG("QUIT ListenSessionLoopback");
}

void AudioSessionManager::OnCaptureLoopbackData(LPBYTE pData, INT dataLen)
{
    LOG_DEBUG("Data Length = " + std::to_string(dataLen));
    // TODO ��ȡPCM���������㷨
    short tmp = 0;
    int sum = 0;
    short* addr = (short*)pData;
    for (int i = 0; i < dataLen; i += 1) {
        memcpy(&tmp, addr + i, sizeof(short));
        if (&tmp != nullptr)
            sum += abs(tmp);
    }
    sum = sum / (dataLen / 2);
    if (sum) {
        int db = (int)(20.0 * log10(sum));
        LOG_DEBUG("THIS FRAME VOL = " + std::to_string(db));
    }
}

bool AudioSessionManager::RegNotifierForSessions()
{
    // ��ȡ�Ự������
    HRESULT hr = device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)&sessionManager);
    CHECK_HR_AND_NULLPTR_RETURN(hr, sessionManager, false, "Activate sessionManager failed");
    if (!RegForNewSessionNotifier()) {
        LOG_ERROR("RegForNewSessionNotifier failed!");
        return false;
    }
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
        if (!RegNotifierForSingleSession(sessionControl)) {
            LOG_ERROR("RegNotifierForSingleSession Failed!");
            continue;
        }
    }
    return true;
}

bool AudioSessionManager::RegForNewSessionNotifier()
{
    // ע���»Ự�����¼�������
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
    // ��ȡsessionID
    CComPtr<IAudioSessionControl2> sessionControl2 = nullptr;
    HRESULT hr = session->QueryInterface(&sessionControl2);
    CHECK_HR_AND_NULLPTR_RETURN(hr, sessionControl2, false, "Get SessionControl2 Failed!");
    LPWSTR id = {0};
    hr = sessionControl2->GetSessionInstanceIdentifier(&id);
    CHECK_HR_RETURN(hr, false, "GetSessionInstanceIdentifier Failed!");
    std::wstring sessionId(id);
    // ע����Ƶ��״̬�ı��¼��ص�
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
    // �ж�����������������
    LOG_DEBUG("Session Active!");
    // ��Ҫͨ��SessionId֪����ǰActive��Session���ĸ�
    LOG_DEBUG(L"SessionID = " + sessionId);
    CHECK_MAP_CONTAINS_KEY_RETURN(sessionMap, sessionId);
    // ��ȡ����
    std::thread th(std::bind(&AudioSessionManager::ListenSessionLoopback, this, std::ref(state)));
    th.detach();
}

void AudioSessionManager::DealWithSessionInactive(const std::wstring& sessionId)
{
    LOG_DEBUG("Session Inactive!");
}

void AudioSessionManager::DealWithSessionExpired(const std::wstring& sessionId)
{
    LOG_DEBUG("Session Expired!");
}
