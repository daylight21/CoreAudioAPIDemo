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
    // ע������Ƶ�������¼��ص�����
    void RegisterSessionCreateCallback(const std::function<void(CComPtr<IAudioSessionControl>)>& cb);
private:
    std::function<void(CComPtr<IAudioSessionControl>)> sessionCreatedCallback;
    ULONG ref{ 0 };
};

