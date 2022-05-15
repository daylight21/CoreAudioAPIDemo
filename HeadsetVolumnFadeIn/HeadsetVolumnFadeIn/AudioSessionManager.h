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
    
    // ��ʼ��
    bool Init();
    // ����ʼ��
    bool Uninit();

private:
    // Ϊ��ǰ���лỰע���¼�֪ͨ��
    bool RegNotifierForSessions();
    // �������Ƶ���¼��ļ���
    bool RegForNewSessionNotifier();
    // Ϊ������Ƶ��ע���¼�������
    bool RegNotifierForSingleSession(CComPtr<IAudioSessionControl> session);
    // ����Ƶ���¼��ص�����
    void OnSessionCreated(CComPtr<IAudioSessionControl> session);
    // ��Ƶ��״̬�ı��¼��ص�����
    void OnSessionStateChange(AudioSessionState state, const std::wstring& sessionId);
    // ������Ƶ��Active�¼�
    void DealWithSessionActive(const std::wstring& sessionId);
    // ������Ƶ��Inactive�¼�
    void DealWithSessionInactive(const std::wstring& sessionId);
    // ������Ƶ��Expired�¼�
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

