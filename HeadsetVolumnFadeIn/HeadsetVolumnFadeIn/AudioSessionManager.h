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

    CComPtr<IAudioSessionManager2> sessionManager{ nullptr };
    CComPtr<IMMDevice> device{ nullptr };
    CComPtr<IMMDeviceEnumerator> enumerator{ nullptr };
    CComPtr<NewSessionNotifier> sessionCreatedNotifier{ nullptr };
    std::map<CComPtr<IAudioSessionControl>, CComPtr<SessionEventNotifier>> sessionPairs;
    bool initFlag{ false };
};

