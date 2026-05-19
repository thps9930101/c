#ifndef SYNC_FRAME_MODEL2_H
#define SYNC_FRAME_MODEL2_H

#include <iostream>
#include <string>
#include <functional>
#include <chrono>
#include <mutex>
#include <thread>
#include <condition_variable>

#include <opencv2/core/cuda.hpp>

#include "mode/my_container/BoundedQueue.h"
#include "mode/my_container/ObjectPool2.h"
#include "mode/my_container/RingBuffer.h"

using clock_type = std::chrono::high_resolution_clock;
using milli_type = std::chrono::duration<double,std::milli>;
using micro_type = std::chrono::duration<double,std::micro>;

//auto t1 = clock_type::now();
//printf("time3 [%lf]\n",milli_type(t2-t1).count());
typedef std::chrono::time_point<std::chrono::steady_clock,std::chrono::duration<long long ,std::ratio<1,1000000000>>> _time;




namespace sync_model {

typedef struct alignas(64) video_stream_data_s {
    uint32_t id;
    uint32_t frm_no;
    uint64_t ntp;
    cv::cuda::GpuMat* data = nullptr;
} vstream_data_t;

typedef RingBuffer<vstream_data_t> vstream_data_rb;
typedef std::shared_ptr<vstream_data_rb> vstream_data_rb_ptr;

typedef Customize::LocalPool<cv::cuda::GpuMat>cv_gpumat_localpool;
typedef std::shared_ptr<cv_gpumat_localpool> cv_gpumat_localpool_ptr;

typedef Customize::ObjectPool<cv::cuda::GpuMat> cv_gpumat_pool;
typedef std::shared_ptr<cv_gpumat_pool> cv_gpumat_pool_ptr;

typedef struct video_stream_s {
    std::shared_ptr<bool> rb_stop;
    std::shared_ptr<std::mutex> rb_lock;
    std::shared_ptr<std::condition_variable> rb_condi;
    vstream_data_rb_ptr stream_rb;
    cv_gpumat_pool_ptr object_pool;
    cv_gpumat_localpool_ptr local_pool;
} video_stream_t;


typedef struct sync_data_s {
    std::vector<vstream_data_t> sync_data;
    std::vector<video_stream_t> multi_video_stream;
} sync_data_t;

class sync_frame_model2
{
public:
    enum sync_output_bf {
        que_output = 0,
        rb_output
    } sync_output_mode;

private:


    void _init(size_t bf_size, const sync_output_bf mode);

public:
    sync_frame_model2();
    sync_frame_model2(size_t bf_size);
    sync_frame_model2(const sync_output_bf mode);
    sync_frame_model2(size_t bf_size, const sync_output_bf mode);
    ~sync_frame_model2();

    bool sync_init();
    bool sync_start();
    void sync_stop();

    bool append_video_stream(video_stream_t* vs);
    void remove_video_stream(video_stream_t* vs);
    void remove_all_video_stream();
    size_t video_stream_count();

    //bool get_sync_data(sync_data_t *&sync_data);
    std::function<bool(sync_data_t *&sync_data)> get_sync_data;
    bool free_sync_data(sync_data_t *sync_data);
    void sync_output_clear();

    //---test--------------------
    _time start_time;
    _time end_time;
    milli_type cost_time;
    uint32_t sync_count;
    uint32_t sync_fail_count = 0;
    uint32_t sync_lost_count = 0;

    _time getdata_start_time;
    _time getdata_end_time;
    milli_type getdata_cost_time;

    _time sync_start_time;
    _time sync_end_time;
    milli_type sync_cost_time;

    _time push_data_start_time;
    _time push_data_end_time;
    milli_type push_data_cost_time;



private:
    // get sync data
    bool get_sync_data_que(sync_data_t *&sync_data);
    bool get_sync_data_rb(sync_data_t *&sync_data);

    //thread
    void th_que_output();
    void th_rb_output();
    void th_rb_output2();

    // input data
    std::vector<video_stream_t> multi_video_stream;

    // output data
    Customize::ObjectPool<sync_data_t> sync_data_pool;
    std::shared_ptr<Customize::LocalPool<sync_data_t>> sync_local_pool;
    std::shared_ptr<BoundedQueue<sync_data_t*>> sync_data_que;
    std::shared_ptr<RingBuffer<sync_data_t*>> sync_data_rb;
    bool _rb_stop;
    std::mutex _rb_mtx;
    std::condition_variable _rb_cond;

    bool th_ctrl;
    std::mutex* _vs_mtx;
    //std::shared_ptr<std::mutex> _vs_mtx;
    std::mutex _sync_mtx;
    std::condition_variable _cond;
    std::shared_ptr<std::thread> sync_th;
};

}
#endif // SYNC_FRAME_MODEL2_H
