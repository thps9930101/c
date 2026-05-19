#include "splice.h"

splice::splice(sync_model::sync_frame_model2 *sy_data)
{
    sp_state = false;
    sync_data = sy_data;

    Sp_data_buf = new RingBuffer<GpuMat_Struct>(5);
    Sp_data_buf->init();

    sp_start_time = clock_type::now();
    sp_end_time = sp_start_time;
    sp_cost_time = sp_end_time - sp_start_time;

    sp_start_time2 = clock_type::now();
    sp_end_time2 = sp_start_time2;
    sp_cost_time2 = sp_end_time2 - sp_start_time2;

    sp_in_count = 0;
    sp_in_lost_count = 0;
    sp_out_lost_count = 0;
}

splice::~splice()
{
    if(Sp_data_buf !=nullptr)
    {
        delete Sp_data_buf;
        Sp_data_buf = nullptr;
    }
}

//------public------
bool splice::sp_start()
{
    if(sp_th.get() != nullptr || sp_state != false)
        return false;

    sp_state = true;

    //log init
    sp_start_time = clock_type::now();
    sp_end_time = sp_start_time;
    sp_cost_time = sp_end_time - sp_start_time;

    sp_start_time2 = clock_type::now();
    sp_end_time2 = sp_start_time2;
    sp_cost_time2 = sp_end_time2 - sp_start_time2;

    sp_in_count = 0;
    sp_in_lost_count = 0;
    sp_out_lost_count = 0;


    Sp_data_buf->clear();
    sp_th = std::make_shared<std::thread>(&splice::sp_run, this);

    return true;
}

bool splice::sp_stop()
{
    sp_state = false;

    if (sp_th.get() != nullptr) {
        Sp_data_buf->data_lock_cv.notify_all();
        sp_th->join();
        sp_th.reset();
        printf("sp stop \n");
    }


    return true;
}

void splice::set_Mode(const SP_MODE mode)
{
    sp_view_param sp_par;
    int srceen_size_w,srceen_size_h;
    switch (mode)
    {
        case SP_MODE::sp_4k_4x3:
            printf("4x3 \n");
            sp_par.col = 4;
            sp_par.row = 3;
            srceen_size_w = 3840;
            srceen_size_h = 2160;

            /*
            sp_par.re_w = 1280;
            sp_par.re_h = 720;
            sp_par.small_view_w = 960;
            sp_par.small_view_h = 720;
            sp_par.crop_left = 160;
            sp_par.crop_right = 160;
            sp_par.crop_top = 0;
            sp_par.crop_bottom = 0;
            */
            break;

        case SP_MODE::sp_4k_3x3:
            printf("3x3 \n");
            sp_par.col = 3;
            sp_par.row = 3;
            srceen_size_w = 3840;
            srceen_size_h = 2160;

            /*
            sp_par.re_w = 1280;
            sp_par.re_h = 720;
            sp_par.small_view_w = 1280;
            sp_par.small_view_h = 720;
            sp_par.crop_left = 0;
            sp_par.crop_right = 0;
            sp_par.crop_top = 0;
            sp_par.crop_bottom = 0;
            */
            break;

        case SP_MODE::sp_4k_3x2:
            printf("3x2 \n");
            sp_par.col = 3;
            sp_par.row = 2;
            srceen_size_w = 3840;
            srceen_size_h = 2160;

            /*
            sp_par.re_w = 1920;
            sp_par.re_h = 1080;
            sp_par.small_view_w = 1280;
            sp_par.small_view_h = 1080;
            sp_par.crop_left = 320;
            sp_par.crop_right = 320;
            sp_par.crop_top = 0;
            sp_par.crop_bottom = 0;
            */
            break;

        case SP_MODE::sp_4k_2x2 :
            printf("2x2 \n");
            sp_par.col = 2;
            sp_par.row = 2;
            srceen_size_w = 3840;
            srceen_size_h = 2160;

            /*
            sp_par.re_w = 1920;
            sp_par.re_h = 1080;
            sp_par.small_view_w = 1920;
            sp_par.small_view_h = 1080;
            sp_par.crop_left = 0;
            sp_par.crop_right = 0;
            sp_par.crop_top = 0;
            sp_par.crop_bottom = 0;
            */
            break;
        default:
            printf("fail \n");
            return;
    }

    sp_par.small_view_w = srceen_size_w / sp_par.col;
    sp_par.small_view_h = srceen_size_h / sp_par.row;
    sp_par.re_w = sp_par.small_view_h / 9 * 16;  //w:h=16:9
    sp_par.re_h = sp_par.small_view_h;
    sp_par.crop_left = (sp_par.re_w - sp_par.small_view_w) / 2;
    sp_par.crop_right = (sp_par.re_w - sp_par.small_view_w) / 2;
    sp_par.crop_top = 0;
    sp_par.crop_bottom = 0;


    _sp_mtx.lock();

    srceen_row = sp_par.row;
    srceen_col = sp_par.col;

    srceen_w = srceen_size_w;
    srceen_h = srceen_size_h;
    mat_resize = cv::Size(sp_par.re_w , sp_par.re_h);
    src_view_rect = cv::Rect(sp_par.crop_left,
                             sp_par.crop_top,
                             sp_par.re_w - sp_par.crop_left - sp_par.crop_right,
                             sp_par.re_h - sp_par.crop_top - sp_par.crop_bottom);

    dst_view_rect.clear();
    for (int r = 0; r < sp_par.row; ++r) {
        for (int c = 0; c < sp_par.col; ++c) {
            dst_view_rect.emplace_back(cv::Rect(c * sp_par.small_view_w,
                                                r * sp_par.small_view_h,
                                                sp_par.small_view_w,
                                                sp_par.small_view_h));
        }
    }
    _sp_mtx.unlock();

}

void splice::set_view_num(std::vector<uint32_t> v_num)
{
    _sp_mtx.lock();
    view_num = v_num;
    _sp_mtx.unlock();
}

void splice::set_Live_View_buf(RingBuffer<GpuMat_Struct> *Live_buf_ptr)
{
    _sp_mtx.lock();
    Live_data_buf = Live_buf_ptr;
    _sp_mtx.unlock();
}

void splice::set_view_rot(std::vector<SP_Rot> v_rot)
{
    _sp_mtx.lock();
    view_rot = v_rot;
    _sp_mtx.unlock();
}


bool splice::get_sp_data(GpuMat_Struct *&sp_data)
{
    //GpuMat_Struct* tmp;
    Sp_data_buf->getTail(sp_data);

    while (sp_data == nullptr)
    {
        if (!sp_state) {
            sp_data = nullptr;
            return false;
        }
        std::unique_lock<std::mutex> _lock(Sp_data_buf->data_lock);
        Sp_data_buf->data_lock_cv.wait(_lock);
        Sp_data_buf->getTail(sp_data);
    }
    //sp_data = tmp;
    return true;
}

void splice::log()
{
    printf("sp_count [%u] sp_in_lost [%u] sp_out_lost [%u] \n"
           "sp_cost_time [%lf] sp_cost_time2 [%lf]  \n",
           sp_in_count,sp_in_lost_count,sp_out_lost_count,sp_cost_time.count(),sp_cost_time2.count());
}

//------private------
void splice::sp_run()
{
    sync_model::sync_data_t *sy_data;
    GpuMat_Struct *out_mat;
    GpuMat_Struct *live_show;
    GpuMat_Struct out_mat_temp;

    std::vector<SP_Rot>::iterator view_rot_it;

    std::vector<uint32_t>::iterator view_it;
    std::vector<cv::Rect>::iterator view_rect_it;
    std::vector<sync_model::vstream_data_t>::iterator sy_it;

    std::vector<SP_Rot> view_rot_temp;

    std::vector<uint32_t> view_num_temp;
    std::vector<cv::Rect> dst_view_rect_temp;
    cv::Rect src_view_rect_temp;
    cv::Size mat_resize_temp;
    int srceen_w_temp;
    int srceen_h_temp;
    int srceen_row_temp;
    int srceen_col_temp;

    cv::cuda::Stream stream;
    cv::cuda::GpuMat gpumat_temp;
    cv::cuda::GpuMat gpumat_flip_temp;

    cv::cuda::GpuMat bg_temp;
    bg_temp.create(srceen_h,srceen_w,CV_8UC3);
    bg_temp.setTo(cv::Scalar(0, 0, 0));

    uint32_t sp_in_old_count = 0;
    int view_count = 0;
    int view_total = 0;

    sp_start_time = clock_type::now();
    sp_end_time = clock_type::now();

    while(sp_state)
    {
        _sp_mtx.lock();
        srceen_row_temp = srceen_row;
        srceen_col_temp = srceen_col;
        srceen_w_temp = srceen_w;
        srceen_h_temp = srceen_h;
        mat_resize_temp = mat_resize;
        src_view_rect_temp = src_view_rect;
        dst_view_rect_temp = dst_view_rect;
        view_num_temp = view_num;
        view_rot_temp = view_rot;
        _sp_mtx.unlock();

        if (!sync_data->get_sync_data(sy_data))
            break;

        sp_start_time2 = clock_type::now();

        sp_in_count = sy_data->sync_data.at(0).frm_no;
        if(sp_in_old_count != 0 )
        {
            sp_in_lost_count += sp_in_count - sp_in_old_count -1;
        }
        sp_in_old_count = sp_in_count;


        Sp_data_buf->getHead(out_mat);
        if(out_mat == nullptr) {
            out_mat = &out_mat_temp;
            sp_out_lost_count++;
        }
        out_mat->frame_num = sy_data->sync_data.at(0).frm_no;
        out_mat->ntp = sy_data->sync_data.at(0).ntp;
        Sp_data_buf->data_lock_cv.notify_all();

        //------
//        if (out_mat->gpu_mat.cols != srceen_w_temp)
//            out_mat->gpu_mat.create(srceen_h_temp,srceen_w_temp,CV_8UC3);
//        out_mat->gpu_mat.setTo(cv::Scalar(0, 0, 0));
        bg_temp.copyTo(out_mat->gpu_mat);
        //-------

        view_rot_it = view_rot_temp.begin();

        view_it = view_num_temp.begin();
        view_rect_it = dst_view_rect_temp.begin();
        view_count = 0;
        view_total = srceen_row_temp * srceen_col_temp;
        for (;view_it!=view_num_temp.end();)
        {
            if(view_count >= view_total)
                break;
            view_count++;

            for (sy_it=sy_data->sync_data.begin(); sy_it!=sy_data->sync_data.end(); sy_it++)
            {
                if ((*view_it) != sy_it->id)
                    continue;
                cv::cuda::resize(*(sy_it->data), gpumat_temp,mat_resize_temp,0, 0, cv::INTER_LINEAR, stream);


                if (view_rot_it != view_rot_temp.end())
                {
                    switch ((*view_rot_it))
                    {
                        case sp_Rot180:
                            cv::cuda::flip(gpumat_temp, gpumat_temp, -1, stream);
                            break;

                        case sp_FlipH:
                            cv::cuda::flip(gpumat_temp, gpumat_temp, 1, stream);
                            break;

                        case sp_FlipV:
                            cv::cuda::flip(gpumat_temp, gpumat_temp, 0, stream);
                            break;

                        case sp_None:
                        case sp_Rot90:
                        case sp_Rot270:
                        default:
                           //都不處理
                            break;
                    }
                }

                //flip  -1 180度(上下左右翻轉)  0 垂直翻轉    1 水平翻轉
                //cv::cuda::flip(gpumat_temp, gpumat_temp, 0, stream);

                gpumat_temp(src_view_rect_temp).copyTo((out_mat->gpu_mat)(*view_rect_it));

                break;
            }
            ++view_it;
            ++view_rect_it;
            if (view_rot_it != view_rot_temp.end())
                ++view_rot_it;
        }


        if(Live_data_buf !=nullptr)
        {
            _sp_mtx.lock();
            Live_data_buf->getHead(live_show);

            if(live_show!=nullptr)
            {
                out_mat->gpu_mat.copyTo(live_show->gpu_mat);
                live_show->frame_num = sy_data->sync_data.at(0).frm_no;
                live_show->ntp = sy_data->sync_data.at(0).ntp;
                Live_data_buf->data_lock_cv.notify_all();
            }
            _sp_mtx.unlock();
        }

        sync_data->free_sync_data(sy_data);

        sp_end_time = clock_type::now();
        sp_end_time2 = clock_type::now();

        sp_cost_time = sp_end_time - sp_start_time;
        sp_cost_time2 = sp_end_time2 - sp_start_time2;

        sp_start_time = clock_type::now();
    }

    printf("sp end \n");
}


