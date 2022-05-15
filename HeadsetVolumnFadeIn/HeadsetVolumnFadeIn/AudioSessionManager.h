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
    // 添加新音频流事件的监听
    bool RegForNewSessionNotifier();
    // 为单个音频流注册事件监听器
    bool RegNotifierForSingleSession(CComPtr<IAudioSessionControl> session);
    // 新音频流事件回调函数
    void OnSessionCreated(CComPtr<IAudioSessionControl> session);
    // 音频流状态改变事件回调函数
    void OnSessionStateChange(AudioSessionState state, const std::wstring& sessionId);
    // 处理音频流Active事件
    void DealWithSessionActive(const std::wstring& sessionId);
    // 处理音频流Inactive事件
    void DealWithSessionInactive(const std::wstring& sessionId);
    // 处理音频流Expired事件
    void DealWithSessionExpired(const std::wstring& sessionId);

    CComPtr<IAudioSessionManager2> sessionManager{ nullptr };
    CComPtr<IMMDevice> device{ nullptr };
    CComPtr<IMMDeviceEnumerator> enumerator{ nullptr };
    CComPtr<NewSessionNotifier> sessionCreatedNotifier{ nullptr };
    CComPtr<IAudioClient> audioClient{ nullptr };
    std::map<CComPtr<IAudioSessionControl2>, CComPtr<SessionEventNotifier>> sessionPairs;
    std::map<std::wstring, CComPtr<IAudioSessionControl2>> sessionMap;
    bool initFlag{ false };
};

