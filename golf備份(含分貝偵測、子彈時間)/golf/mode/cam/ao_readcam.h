#ifndef AO_READCAM_H
#define AO_READCAM_H

#include "ao_client.h"


struct Cam_CallBack {
    ao::OnH265DataCallback cam_vid_data = nullptr;
    ao::OnAudioDataCallback cam_aud_data = nullptr;
    ao::OnConnectCallback  cam_connect = nullptr;
    ao::OnDisconnectCallback cam_disconnect = nullptr;
    ao::OnException cam_exception = nullptr;

    ao::OnVideoMetaCallback cam_VideoMeta = nullptr;
    ao::OnAudioMetaCallback cam_AudioMeta = nullptr;


};


class ao_readcam
{
public:
    ao_readcam(const char* IP);
    ~ao_readcam();

    void init();
    void start();
    void stop();

    void setOnH265DataCallback(ao::OnH265DataCallback cb);
    void setOnAudioDataCallback(ao::OnAudioDataCallback cb);
    void setOnConnectCallback(ao::OnConnectCallback cb);
    void setOnDisconnectedCallback(ao::OnDisconnectCallback cb);
    void setOnExceptionCallback(ao::OnException cb);

    void setOnVideoMetaCallback(ao::OnVideoMetaCallback cb);
    void setOnAudioMetaCallback(ao::OnAudioMetaCallback cb);

    const char* Cam_IP;
    Cam_CallBack cam_cb_fun;

private:
    ao::AO_Client *Cam;


};

#endif // AO_READCAM_H
