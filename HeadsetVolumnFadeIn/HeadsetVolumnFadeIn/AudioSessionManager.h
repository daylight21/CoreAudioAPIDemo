#pragma once
#include "SessionEventNotifier.h"
#include "NewSessionNotifier.h"
#include <map>
#include <memory>
#include <mmdeviceapi.h>

class AudioSessionManager
{
public:
    AudioSessionManager();
    ~AudioSessionManager();
    
    // 初始化
    bool Init();
    // 反初始化
    bool Uninit();

private:
    // 为当前所有会话注册事件通知器
    bool RegNotifierForSessions();

    CComPtr<IAudioSessionManager2> sessionManager{ nullptr };
    CComPtr<IMMDevice> device{ nullptr };
    CComPtr<IMMDeviceEnumerator> enumerator{ nullptr };
    CComPtr<NewSessionNotifier> sessionCreatedNotifier{ nullptr };
    std::map<CComPtr<IAudioSessionControl>, CComPtr<SessionEventNotifier>> sessionPairs;
    bool initFlag{ false };
};

