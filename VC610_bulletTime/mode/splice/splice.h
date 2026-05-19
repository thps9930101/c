#ifndef SPLICE_H
#define SPLICE_H

#include <thread>
#include <vector>
#include <mutex>
#include "opencv2/cudawarping.hpp"
#include "mode/public_struct.h"
#include "mode/AOCamStream/sync_frame_model2.h"
#include "mode/my_container/RingBuffer.h"

#include <opencv2/core.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaarithm.hpp>
#include <Data/Enum/SP_Rot.h>
#include <Data/Enum/SP_Mode.h>
//enum SP_Rot{
//    sp_None = -1,
//    sp_Rot180,
//    sp_FlipH,
//    sp_FlipV,

//    //------
//    sp_Rot90,
//    sp_Rot270
//};

//enum SP_MODE{
//    sp_4k_4x3 = 0,
//    sp_4k_3x3,
//    sp_4k_3x2,
//    sp_4k_2x2
//};

struct sp_view_param{
    int row;
    int col;
    int re_w;
    int re_h;
    int small_view_w;
    int small_view_h;
    int crop_left;
    int crop_right;
    int crop_top;
    int crop_bottom;
};

class splice
{
public:
    splice(sync_model::sync_frame_model2 *sy_data);
    ~splice();

    bool sp_start();
    bool sp_stop();

    void set_Mode(SP_MODE mode);
    void set_view_num(std::vector<uint32_t> v_num);
    void set_Live_View_buf(RingBuffer<GpuMat_Struct> *Live_buf_ptr);

    void set_view_rot(std::vector<SP_Rot> v_rot);

    bool get_sp_data(GpuMat_Struct *&sp_data);

    void log();

private:
    void sp_run();

    std::shared_ptr<std::thread> sp_th;
    bool sp_state;

    RingBuffer<GpuMat_Struct> *Sp_data_buf =nullptr;

    sync_model::sync_frame_model2 *sync_data = nullptr;
    RingBuffer<GpuMat_Struct> *Live_data_buf =nullptr;

    std::vector<SP_Rot> view_rot;

    std::mutex _sp_mtx;
    std::vector<uint32_t> view_num;
    std::vector<cv::Rect> dst_view_rect;
    cv::Rect src_view_rect;
    cv::Size mat_resize;
    int srceen_w = 3840 ;
    int srceen_h = 2160 ;
    int srceen_row = 3;
    int srceen_col = 4;


    //log----------   
    _time sp_start_time;
    _time sp_end_time;
    milli_type sp_cost_time;

    _time sp_start_time2;
    _time sp_end_time2;
    milli_type sp_cost_time2;

    uint32_t sp_in_count;
    uint32_t sp_in_lost_count;
    uint32_t sp_out_lost_count;
    //-----------




};

#endif // SPLICE_H
