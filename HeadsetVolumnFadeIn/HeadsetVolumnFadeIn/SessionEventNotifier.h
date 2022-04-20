#pragma once
#include <audiopolicy.h>
#include <functional>

class SessionEventNotifier :
    public IAudioSessionEvents
{
public:
    SessionEventNotifier();
    ~SessionEventNotifier();

    // ��д�ӿ�
    // ��չ�ӿ�
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
    // �������ü���
    ULONG STDMETHODCALLTYPE AddRef(void) override;
    // �������ü���
    ULONG STDMETHODCALLTYPE Release(void) override;
    // ��ʾ���Ʊ仯�¼�
    HRESULT STDMETHODCALLTYPE OnDisplayNameChanged(
        _In_  LPCWSTR NewDisplayName,
        LPCGUID EventContext) override;
    // ͼ��仯�¼�
    HRESULT STDMETHODCALLTYPE OnIconPathChanged(
        _In_  LPCWSTR NewIconPath,
        LPCGUID EventContext) override;
    // ��Ƶ�������仯�¼��������������ѣ�
    HRESULT STDMETHODCALLTYPE OnSimpleVolumeChanged(
        _In_  float NewVolume,
        _In_  BOOL NewMute,
        LPCGUID EventContext) override;
    // ��Ƶ�������仯�¼��������������ѣ�
    HRESULT STDMETHODCALLTYPE OnChannelVolumeChanged(
        _In_  DWORD ChannelCount,
        _In_reads_(ChannelCount)  float NewChannelVolumeArray[],
        _In_  DWORD ChangedChannel,
        LPCGUID EventContext) override;
    // ����ע�¼�
    HRESULT STDMETHODCALLTYPE OnGroupingParamChanged(
        _In_  LPCGUID NewGroupingParam,
        LPCGUID EventContext) override;
    // ��Ƶ��״̬�ı��¼�
    HRESULT STDMETHODCALLTYPE OnStateChanged(
        _In_  AudioSessionState NewState) override;
    // ��Ƶ���Ͽ������¼�
    HRESULT STDMETHODCALLTYPE OnSessionDisconnected(
        _In_  AudioSessionDisconnectReason DisconnectReason) override;

private:
    std::function<void(AudioSessionState)> sessionStateChangeCallback;
    ULONG ref{ 0 };
};