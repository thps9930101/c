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
//#include "mode/encoder/output_rtsp.h"
#include "mode/encoder/encode.h"
#include "mode/encoder/output_rtsp2.h"
#include "mode/encoder/output_file.h"
#include "mode/encoder/output_rtmp.h"

#include "mode/my_container/BoundedQueue.h"
#include "mode/my_container/RingBoundedQueue.h"
#include "mode/my_container/RingBuffer.h"
#include "mode/my_container/mem_pool.h"
#include "mode/my_container/ObjectPool2.h"
#include "mode/AOCamStream/sync_frame_model2.h"
#include "mode/public_struct.h"
#include "mode/splice/splice.h"
#include "conver.h"

#include "mode/cam/vid_record/multiple_vid_ctx.h"
#include "mode/cam/vid_record/multiple_vid_record.h"

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

    //-----------------------------
    //單機錄影
    multiple_vid_record Cam_Video;
    bool video_statu=false;
    bool Video_FirstOpen=true;
    string cam_video_path="";

    //-----------------------------------------------------
    Decoder_class *dc = nullptr;                                            //decoder模組
    std::thread* dc_thr = nullptr;
    bool dc_Live_show =false;
    bool connect_state = false;
    bool DC_state = false;

    std::shared_ptr<bool> sy_stop;
    std::shared_ptr<std::mutex> sy_rb_lock;
    std::shared_ptr<std::condition_variable> sy_rb_condi;

    sync_model::vstream_data_rb_ptr stream_rb;
    sync_model::cv_gpumat_pool_ptr object_pool;

    sync_model::cv_gpumat_localpool_ptr sy_local_pool;
    sync_model::cv_gpumat_localpool_ptr dc_local_pool;

    //-----------------------------
    uint32_t cam_data_count;
    uint32_t cam_data_lost_count;

    uint32_t dc2_count;
    uint32_t dc2_lost_count;
    _time dc2_start_time;
    _time dc2_end_time;
    milli_type dc2_cost_time;

    _time dc2_start_time2;
    _time dc2_end_time2;
    milli_type dc2_cost_time2;


    _time cam_record_start_time;
    _time cam_record_end_time;
    milli_type cam_record_cost_time;
    //------------------------------

    Cam_data(){
        vid_data_buf =nullptr;
        ao_cam = nullptr;
        vid_data_containers_full =false;

        dc = nullptr;
        dc_Live_show =false;

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
    }
};


struct Cam_staruct {
    uint32_t id;
    std::shared_ptr<Cam_data> Cam_d;

    bool operator<(const Cam_staruct& other) const {
        return id < other.id;
    }
};

//紀錄開始(錄影 rtsp rtmp)時間戳
struct time_state {

    bool save =false;

    uint32_t startFNo = 0;
    uint32_t endFNo = 0;

    uint64_t  start_ntp_timestamp = 0;
    uint64_t  end_ntp_timestamp = 0;

};

//單機錄影
struct Cam_Video {
    multiple_vid_ctx *vid_ctx = nullptr;
    multiple_vid_ctx_param Cam_Video_param;
    string Video_Path = ".";
    time_state Video_State;
};

//拼接錄影 rtsp rtmp
template<typename T>
struct Output {
    T output;
    std::string output_path;
    bool open = false;
    bool open_first = true;
    time_state Video_State;
};


enum Show_Mode{
    Show_Single = 0,
    Show_splice
};


class ao_cameram_fun
{
public:
    ao_cameram_fun();
    ~ao_cameram_fun();
    static std::function<void(const QString&, const QString&)> notifyFunc;

    void init(const Decoder_class_param dc_par,const Encoder_param enc_par);
    uint32_t AddCam(const char* ip);

    void DeleteCam(uint32_t cam_id);
    void DeleteAllCam();

    void start_all();
    void stop_all();

    void start_cam_num(uint32_t cam_id);
    void stop_cam_num(uint32_t cam_id);

    bool set_Encoder_param(const Encoder_param par);
    bool set_Live_num(uint32_t cam_id);
    void set_Show_Mode(const Show_Mode mode);
    void set_view_sp_num(std::vector<uint32_t> v_num);
    void set_view_sp_rot(std::vector<SP_Rot> v_rot);
    void set_sp_Mode(const SP_MODE mode);
    bool set_CamVideo_param(const multiple_vid_ctx_param par);
    void set_Camera_videoSave(bool enable);
    void set_Camera_videoPath(string path);

    int get_Cam_size();
    QImage get_LiveView();
    void get_LiveView(QImage *out_img);

    int open_rtsp(std::string rtsp_path);
    int close_rtsp();

    int open_file(std::string file_path);
    int close_file();

    int open_rtmp(std::string rtmp_path);
    int close_rtmp();

    void log();

    //--------------------------------------------

    std::set<Cam_staruct> Cam;

    bool save_video = false;


private:
    void Cam_Decoder(bool *state,std::shared_ptr<Cam_data> cam_d,Cam_Video *Cam_vid);
    void sp_output(bool *state);
    void Write_Log_File(QString type,QString msg);

    //--------------------------------------------
    sync_model::sync_frame_model2 sync_frame2;

    splice *sp = nullptr;
    std::thread* sp_output_th = nullptr;
    bool sp_output_state = false;
    std::vector<uint32_t> sp_view_num;
    std::vector<SP_Rot> sp_view_rot;

    RingBuffer<GpuMat_Struct> *Live_show_Mat = nullptr;
    cv::Mat Live_View_cpuMat;
    uint32_t Live_num;
    Show_Mode mode;

    Decoder_class_param dc_par;

    std::mutex  ec_lock;
    Encoder_param ec_par;
    Encoder_class enc_class;

    Output<output_file> out_file;

    Output<output_rtsp> out_rtsp;

    Output<output_rtmp> out_rtmp;

    AVFrame *avTemp = nullptr;
    uint8_t *out_nv12_uint8 = nullptr;
    uint8_t *GPU_nv12_uint8 = nullptr;
    int y_size = 0;

    //-----------------------------

    Cam_Video Cam_vid;

    //------log------
    uint32_t sp_out_in_count;

    _time sp_out_start_time;
    _time sp_out_end_time;
    milli_type sp_out_cost_time;

    _time sp_out_start_time2;
    _time sp_out_end_time2;
    milli_type sp_out_cost_time2;

    _time conver_start_time;
    _time conver_end_time;
    milli_type conver_cost_time;

    _time en_start_time;
    _time en_end_time;
    milli_type en_cost_time;

    _time view_start_time;
    _time view_end_time;
    milli_type view_cost_time;
    uint32_t view_count;

    //----------------------

};

#endif // AO_CAMERAM_FUN_H
