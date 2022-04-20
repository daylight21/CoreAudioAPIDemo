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

    // ��д�ӿ�
    // ��չ�ӿ�
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
    // �������ü���
    ULONG STDMETHODCALLTYPE AddRef(void) override;
    // �������ü���
    ULONG STDMETHODCALLTYPE Release(void) override;

    // ����Ƶ�������¼�
    HRESULT STDMETHODCALLTYPE OnSessionCreated(IAudioSessionControl* NewSession);

private:
    std::function<void(CComQIPtr<IAudioSessionControl>)> sessionCreatedCallback;
    ULONG ref{ 0 };
};

