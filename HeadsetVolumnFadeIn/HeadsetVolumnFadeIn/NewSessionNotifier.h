#pragma once
#include <atlbase.h>
#include <audiopolicy.h>
#include <functional>

class NewSessionNotifier :
    public IAudioSessionNotification
{
public:
    NewSessionNotifier();
    ~NewSessionNotifier();

    // 重写接口
    // 扩展接口
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
    // 增加引用计数
    ULONG STDMETHODCALLTYPE AddRef(void) override;
    // 减少引用计数
    ULONG STDMETHODCALLTYPE Release(void) override;

    // 新音频流创建事件
    HRESULT STDMETHODCALLTYPE OnSessionCreated(IAudioSessionControl* NewSession);

private:
    std::function<void(CComQIPtr<IAudioSessionControl>)> sessionCreatedCallback;
    ULONG ref{ 0 };
};

