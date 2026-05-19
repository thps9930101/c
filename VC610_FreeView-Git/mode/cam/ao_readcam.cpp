#include "ao_readcam.h"

static void on_265(ao_readcam* Cam,struct ao::lm_video_data *h265)
{
    printf("video[%s] [%d] [%d] [%d] [%llu]\n",Cam->Cam_IP,h265->frm_no,h265->type,h265->len,h265->ntp_timestamp);
}

static void on_audio_data(ao_readcam* Cam,struct ao::lm_audio_data *audio)
{
    printf("audio[%s] [%d] [%d] [%llu]\n",Cam->Cam_IP,audio->frm_no,audio->len,audio->ntp_timestamp);
}

static void on_Connect(ao_readcam* Cam)
{
    printf("[%s] OnConnect\n",Cam->Cam_IP);
}

static void on_Disconnect(ao_readcam* Cam)
{
    printf("[%s] OnDisconnect\n",Cam->Cam_IP);
}

static void on_Exception(ao_readcam* Cam)
{
    printf("[%s] OnException\n",Cam->Cam_IP);
}

static void on_VideoMeta(ao_readcam* Cam,struct ao::lm_video_meta *vid_data)
{
    printf("[%s] width [%u] height [%u] fps [%u] gop [%u] \n"
           ,Cam->Cam_IP,vid_data->width,vid_data->height,vid_data->fps,vid_data->gop);
}

static void on_AudioMeta(ao_readcam* Cam,struct ao::lm_audio_meta *au_data)
{
    printf("[%s] sample_rate [%u] sample_size [%u] channel [%u] bitrate [%u] encoder [%u]\n"
           ,Cam->Cam_IP,au_data->sample_rate,au_data->sample_size,au_data->channel
           ,au_data->bitrate,au_data->encoder);
}

ao_readcam::ao_readcam(const char* IP)
{
    this->Cam_IP = strdup(IP);
    Cam = new ao::AO_Client(IP);
}

ao_readcam::~ao_readcam()
{
    if(Cam != nullptr)
    {
        delete Cam;
        Cam = nullptr;
    }

}

void ao_readcam::init()
{
    if(this->cam_cb_fun.cam_vid_data == nullptr)
        Cam->setOnH265DataCallback(std::bind(on_265,this,std::placeholders::_1));
    else
        this->Cam->setOnH265DataCallback(this->cam_cb_fun.cam_vid_data);

    if(this->cam_cb_fun.cam_aud_data == nullptr)
        Cam->setOnAudioDataCallback(std::bind(on_audio_data,this,std::placeholders::_1));
    else
        this->Cam->setOnAudioDataCallback(this->cam_cb_fun.cam_aud_data);

    if(this->cam_cb_fun.cam_connect == nullptr)
        this->Cam->setOnConnectCallback(std::bind(on_Connect,this));
    else
        this->Cam->setOnConnectCallback(this->cam_cb_fun.cam_connect);

    if(this->cam_cb_fun.cam_disconnect == nullptr)
        Cam->setOnDisconnectedCallback(std::bind(on_Disconnect,this));
    else
        this->Cam->setOnDisconnectedCallback(this->cam_cb_fun.cam_disconnect);

    if(this->cam_cb_fun.cam_exception == nullptr)
        Cam->setOnExceptionCallback(std::bind(on_Exception,this));
    else
        this->Cam->setOnExceptionCallback(this->cam_cb_fun.cam_exception);

    if(this->cam_cb_fun.cam_VideoMeta == nullptr)
        Cam->setOnVideoMetaCallback(std::bind(on_VideoMeta,this,std::placeholders::_1));
    else
        this->Cam->setOnVideoMetaCallback(this->cam_cb_fun.cam_VideoMeta);

    if(this->cam_cb_fun.cam_AudioMeta == nullptr)
        Cam->setOnAudioMetaCallback(std::bind(on_AudioMeta,this,std::placeholders::_1));
    else
        this->Cam->setOnAudioMetaCallback(this->cam_cb_fun.cam_AudioMeta);


}

void ao_readcam::start()
{
    Cam->start();
}

void ao_readcam::stop()
{
    Cam->stop();
}


void ao_readcam::setOnH265DataCallback(ao::OnH265DataCallback cb)
{
    this->cam_cb_fun.cam_vid_data = cb;
}

void ao_readcam::setOnAudioDataCallback(ao::OnAudioDataCallback cb)
{
    this->cam_cb_fun.cam_aud_data = cb;
}

void ao_readcam::setOnConnectCallback(ao::OnConnectCallback cb)
{
    this->cam_cb_fun.cam_connect = cb;
}

void ao_readcam::setOnDisconnectedCallback(ao::OnDisconnectCallback cb)
{
    this->cam_cb_fun.cam_disconnect = cb;
}

void ao_readcam::setOnExceptionCallback(ao::OnException cb)
{
    this->cam_cb_fun.cam_exception = cb;
}

void ao_readcam::setOnVideoMetaCallback(ao::OnVideoMetaCallback cb)
{
    this->cam_cb_fun.cam_VideoMeta = cb;
}
void ao_readcam::setOnAudioMetaCallback(ao::OnAudioMetaCallback cb)
{
    this->cam_cb_fun.cam_AudioMeta = cb;
}





