#ifndef AO_CLIENT_H
#define AO_CLIENT_H

#ifdef _WIN32

#ifdef FULL_SOURCE
#define LIBAOCP_API
#else // FULL_SOURCE

#ifdef LIBAOCP_EXPORT
#define LIBAOCP_API __declspec(dllexport)
#else
#define LIBAOCP_API __declspec(dllimport)
#endif // LIBAOCP_EXPORT

#endif // FULL_SOURCE

#else // _WIN32
#define LIBAOCP_API
#endif // _WIN32

#include <functional>
#include <stdint.h>

#define LM_AUDIO_ENCODER_NONE   	(0)
#define LM_AUDIO_ENCODER_PCM	    (23)
#define LM_AUDIO_ENCODER_AAC	    (37)

namespace ao{
struct LIBAOCP_API lm_packet_header {
        uint32_t magic;        //必須0X55
        uint32_t len;
        uint32_t type;
};


struct LIBAOCP_API lm_video_meta {
    uint32_t width;
    uint32_t height;
    uint32_t fps;
    uint32_t gop;

    /* video data */
    uint32_t  len;
    uint64_t  ntp_timestamp;
    uint32_t  frm_no;
    uint32_t  type;   // I or P
};

struct LIBAOCP_API lm_video_data
{
    uint32_t  len;
    uint64_t  ntp_timestamp;
    uint32_t  frm_no;
    uint32_t  type;   // I or P
    uint8_t * data;
};

struct LIBAOCP_API lm_audio_meta {
    uint32_t sample_rate;
    uint32_t sample_size;
    uint32_t channel;
    uint32_t bitrate;
    uint32_t encoder;

    /* Audio data */
    uint32_t  len;
    uint64_t  ntp_timestamp;
    uint32_t  frm_no;
};

struct LIBAOCP_API lm_audio_data {
    uint32_t  len;
    uint64_t  ntp_timestamp;
    uint32_t  frm_no;
    uint8_t * data;
};

typedef std::function<void(struct lm_video_data * h265)> OnH265DataCallback;
typedef std::function <void(struct lm_audio_data * audio)> OnAudioDataCallback;
typedef std::function <void(void)> OnException;
typedef std::function <void(void)> OnDisconnectCallback;
typedef std::function <void(void)> OnConnectCallback;
typedef std::function <void(struct lm_video_meta * vid_data)> OnVideoMetaCallback;
typedef std::function <void(struct lm_audio_meta * au_data)> OnAudioMetaCallback;


class Ao_Client_Imp;
class LIBAOCP_API AO_Client
{
public:
    AO_Client(const char* IP);
    ~AO_Client();

    void start();
    void stop();

    void setOnH265DataCallback(const OnH265DataCallback & cb);
    void setOnAudioDataCallback(const OnAudioDataCallback & cb);
    void setOnExceptionCallback(const OnException & cb);
    void setOnDisconnectedCallback(const OnDisconnectCallback & cb);
    void setOnConnectCallback(const OnConnectCallback & cb);

    void setOnVideoMetaCallback(const OnVideoMetaCallback & cb);
    void setOnAudioMetaCallback(const OnAudioMetaCallback & cb);


private:

    Ao_Client_Imp *ao_imp = nullptr;


};
}
#endif // AO_CLIENT_H
