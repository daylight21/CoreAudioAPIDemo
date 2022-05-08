#pragma once
#include <audiopolicy.h>
#include <functional>
#include <string>

using SessionStateChangeCallback = std::function<void(AudioSessionState, const std::wstring&)>;
class SessionEventNotifier :
    public IAudioSessionEvents
{
public:
    SessionEventNotifier(const std::wstring& id);
    SessionEventNotifier() = delete;
    ~SessionEventNotifier();

    // 重写接口
    // 扩展接口
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
    // 增加引用计数
    ULONG STDMETHODCALLTYPE AddRef(void) override;
    // 减少引用计数
    ULONG STDMETHODCALLTYPE Release(void) override;
    // 显示名称变化事件
    HRESULT STDMETHODCALLTYPE OnDisplayNameChanged(
        _In_  LPCWSTR NewDisplayName,
        LPCGUID EventContext) override;
    // 图标变化事件
    HRESULT STDMETHODCALLTYPE OnIconPathChanged(
        _In_  LPCWSTR NewIconPath,
        LPCGUID EventContext) override;
    // 音频流音量变化事件（触发条件较难）
    HRESULT STDMETHODCALLTYPE OnSimpleVolumeChanged(
        _In_  float NewVolume,
        _In_  BOOL NewMute,
        LPCGUID EventContext) override;
    // 音频流音量变化事件（触发条件较难）
    HRESULT STDMETHODCALLTYPE OnChannelVolumeChanged(
        _In_  DWORD ChannelCount,
        _In_reads_(ChannelCount)  float NewChannelVolumeArray[],
        _In_  DWORD ChangedChannel,
        LPCGUID EventContext) override;
    // 不关注事件
    HRESULT STDMETHODCALLTYPE OnGroupingParamChanged(
        _In_  LPCGUID NewGroupingParam,
        LPCGUID EventContext) override;
    // 音频流状态改变事件
    HRESULT STDMETHODCALLTYPE OnStateChanged(
        _In_  AudioSessionState NewState) override;
    // 音频流断开连接事件
    HRESULT STDMETHODCALLTYPE OnSessionDisconnected(
        _In_  AudioSessionDisconnectReason DisconnectReason) override;
    // 注册音频流状态改变回调函数
    void RegisterSessionStateChangeCallback(const SessionStateChangeCallback& cb);
private:
    SessionStateChangeCallback sessionStateChangeCallback;
    std::wstring sessionId;
    ULONG ref{ 0 };
};