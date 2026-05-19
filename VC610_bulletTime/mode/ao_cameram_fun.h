#ifndef AO_CAMERAM_FUN_H
#define AO_CAMERAM_FUN_H

#include <QImage>
#include <iostream>
#include <vector>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/cudaimgproc.hpp"
#include "opencv2/cudawarping.hpp"
#include <opencv2/bgsegm.hpp>
#include <opencv2/videoio.hpp>
#include "opencv2/stitching.hpp"
#include "opencv2/cudacodec.hpp"
#include "opencv2/imgproc/types_c.h"
#include "opencv2/imgcodecs/legacy/constants_c.h"

#include <opencv2/dnn.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudabgsegm.hpp>
#include <opencv2/cudafilters.hpp>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "ao_client.h"

#include "mode/cam/ao_readcam.h"
#include "mode/decode/decode.h"


#include "mode/my_container/RingBuffer.h"

#include "mode/public_struct.h"
#include "conver.h"


//-----------
using clock_type = std::chrono::high_resolution_clock;
using milli_type = std::chrono::duration<double,std::milli>;
using micro_type = std::chrono::duration<double,std::micro>;

//auto t1 = clock_type::now();
//printf("time3 [%lf]\n",milli_type(t2-t1).count());
typedef std::chrono::time_point<std::chrono::steady_clock,std::chrono::duration<long long ,std::ratio<1,1000000000>>> _time;
//-----------

struct Cam_data{
    uint32_t CamNO;
    RingBuffer<ao::lm_video_data> *vid_data_buf =nullptr;                   //1.RB  Cam的data
    ao_readcam *ao_cam = nullptr;                                           //Cam模組
    bool vid_data_containers_full =false;                                   //判斷Cam的data RB是否滿了


    //-----------------------------------------------------
    Decoder_class *dc = nullptr;                                            //decoder模組
    std::thread* dc_thr = nullptr;
    bool dc_Live_show =false;
    bool connect_state = false;
    bool DC_state = false;



    //-----------------------------
    uint32_t cam_data_count;
    uint32_t cam_data_lost_count;


    //------------------------------

    Cam_data(){
        vid_data_buf =nullptr;
        ao_cam = nullptr;
        vid_data_containers_full =false;

        dc = nullptr;
        dc_thr = nullptr;
        dc_Live_show =false;
        connect_state = false;
        DC_state = false;

    }

    ~Cam_data(){

        if (vid_data_buf !=nullptr)
        {
            delete vid_data_buf;
            vid_data_buf=nullptr;
        }

        if (ao_cam !=nullptr)
        {
            delete ao_cam;
            ao_cam=nullptr;
        }

        if (dc !=nullptr)
        {
            delete dc;
            dc=nullptr;
        }

        if(dc_thr != nullptr)
        {
            dc_thr->join();
            delete dc_thr;
            dc_thr=nullptr;

        }
    }
};


struct Cam_staruct {
    uint32_t id;
    std::shared_ptr<Cam_data> Cam_d;

    bool operator<(const Cam_staruct& other) const {
        return id < other.id;
    }
};
typedef std::function<void(std::shared_ptr<Cam_data> Cam,struct ao::lm_video_data *h265)> OnCamVidData;
typedef std::function<void(std::shared_ptr<Cam_data> Cam,struct ao::lm_audio_data *audio)> OnCamAudData;
typedef std::function<void(std::shared_ptr<Cam_data> Cam)> OnCamConnect;
typedef std::function<void(std::shared_ptr<Cam_data> Cam)> OnCamDisconnect;
typedef std::function<void(std::shared_ptr<Cam_data> Cam)> OnCamException;
typedef std::function<void(std::shared_ptr<Cam_data> Cam,struct ao::lm_video_meta *vid_data)> OnCamVideoMeta;
typedef std::function<void(std::shared_ptr<Cam_data> Cam,struct ao::lm_audio_meta *au_data)> OnCamAudioMeta;


struct Cam_CallBack_Fun {
    OnCamVidData  cam_vid_data = nullptr;
    OnCamAudData cam_aud_data = nullptr;

    OnCamConnect  cam_connect = nullptr;
    OnCamDisconnect  cam_disconnect = nullptr;
    OnCamException  cam_exception = nullptr;

    OnCamVideoMeta  cam_VideoMeta = nullptr;
    OnCamAudioMeta  cam_AudioMeta = nullptr;
};

class ao_cameram_fun
{
public:
    ao_cameram_fun();
    ~ao_cameram_fun();


    void init(const Decoder_class_param dc_par);
    uint32_t AddCam(const char* ip);

    void DeleteCam(uint32_t cam_id);
    void DeleteAllCam();

    void start_all();
    void stop_all();

    void start_cam_num(uint32_t cam_id);
    void stop_cam_num(uint32_t cam_id);


    bool set_Live_num(uint32_t cam_id);

    void set_cam_cb_VidData(OnCamVidData rb);
    void set_cam_cb_AudData(OnCamAudData rb);

    void set_cam_cb_Connect(OnCamConnect rb);
    void set_cam_cb_Disconnect(OnCamDisconnect rb);
    void set_cam_cb_Exception(OnCamException rb);

    void set_cam_cb_VideoMeta(OnCamVideoMeta rb);
    void set_cam_cb_AudioMeta(OnCamAudioMeta rb);


    int get_Cam_size();
    QImage get_LiveView();
    void get_LiveView(QImage *out_img);


    void log();

    //--------------------------------------------

    std::set<Cam_staruct> Cam;


private:
    void Cam_Decoder(bool *state,std::shared_ptr<Cam_data> cam_d);
    void Write_Log_File(QString type,QString msg);

    //--------------------------------------------

    RingBuffer<GpuMat_Struct> *Live_show_Mat = nullptr;
    cv::Mat Live_View_cpuMat;
    uint32_t Live_num;

    Decoder_class_param dc_par;

    //-----------------------------

    Cam_CallBack_Fun cam_cb_fun;

    //------log------


    _time view_start_time;
    _time view_end_time;
    milli_type view_cost_time;
    uint32_t view_count;

    //----------------------

};

#endif // AO_CAMERAM_FUN_H
