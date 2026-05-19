#ifndef SYNC_FRAME_MODEL_H
#define SYNC_FRAME_MODEL_H

#include <iostream>
#include <string>
#include <chrono>
#include <mutex>
#include <thread>
#include <condition_variable>

#include <opencv2/core/cuda.hpp>

#include "mode/my_container/BoundedQueue.h"
#include "mode/my_container/ObjectPool2.h"

using clock_type = std::chrono::high_resolution_clock;
using milli_type = std::chrono::duration<double,std::milli>;
using micro_type = std::chrono::duration<double,std::micro>;

//auto t1 = clock_type::now();
//printf("time3 [%lf]\n",milli_type(t2-t1).count());
typedef std::chrono::time_point<std::chrono::steady_clock,std::chrono::duration<long long ,std::ratio<1,1000000000>>> _time;



typedef struct alignas(64) video_stream_data_s {
    int id;
    uint32_t frm_no;
    uint64_t ntp;
    cv::cuda::GpuMat* data;
} vstream_data_t;

typedef BoundedQueue<vstream_data_t> vstream_data_que;
typedef std::shared_ptr<vstream_data_que> vstream_data_que_ptr;

typedef Customize::LocalPool<cv::cuda::GpuMat>cv_gpumat_localpool;
typedef std::shared_ptr<cv_gpumat_localpool> cv_gpumat_localpool_ptr;

typedef Customize::ObjectPool<cv::cuda::GpuMat> cv_gpumat_pool;
typedef std::shared_ptr<cv_gpumat_pool> cv_gpumat_pool_ptr;

typedef struct video_stream_s {
    vstream_data_que_ptr vdqp;
    cv_gpumat_pool_ptr cgpp;
} video_stream_t;

typedef struct input_video_stream {
    video_stream_t video_stream_param;
    cv_gpumat_localpool_ptr local_pool;
} ivs;

typedef struct sync_data_s {
    std::vector<vstream_data_t> sync_data;
    std::vector<ivs> multi_video_stream;
} sync_data_t;


/*
typedef RingBuffer<vstream_data_t> vstream_data_rb;
typedef std::shared_ptr<vstream_data_rb> vstream_data_rb_ptr;

typedef struct video_stream_s {
    std::shared_ptr<bool> rb_stop;
    std::shared_ptr<std::mutex> rb_lock;
    std::shared_ptr<std::condition_variable> rb_condi;
    vstream_data_rb_ptr vdrp;
    cv_gpumat_pool_ptr cgpp;
} video_stream_t;

*/




class sync_frame_model
{
public:
    sync_frame_model();
    sync_frame_model(size_t que_size);
    ~sync_frame_model();

    bool sync_init();
    bool sync_start();
    void sync_stop();

    bool append_video_stream(video_stream_t* vs);
    void remove_video_stream(video_stream_t* vs);
    void remove_all_video_stream();
    size_t video_stream_count();

    bool get_sync_data(sync_data_t *&sync_data);
    bool free_sync_data(sync_data_t *sync_data);
    bool free_sync_data2(sync_data_t *sync_data);

    //---test--------------------
    _time start_time;
    _time end_time;
    milli_type cost_time;
    uint32_t sync_count;

private:
    // thread
    void th();
    void th2();

    // input data
    std::vector<ivs> multi_video_stream;

    // output data
    Customize::ObjectPool<sync_data_t> sync_data_pool;
    std::shared_ptr<Customize::LocalPool<sync_data_t>> sync_local_pool;
    std::shared_ptr<BoundedQueue<sync_data_t*>> sync_data_que;

    bool th_ctrl;
    std::mutex _mtx;
    std::mutex _sync_mtx;
    std::condition_variable _cond;
    std::shared_ptr<std::thread> sync_th;
};

#endif // SYNC_FRAME_MODEL_H
