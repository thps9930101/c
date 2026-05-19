#include "sync_frame_model.h"



// =========================================================




sync_frame_model::sync_frame_model()
{    
    sync_local_pool = std::make_shared<Customize::LocalPool<sync_data_t>>(&sync_data_pool.global_pool);
    sync_data_que = std::make_shared<BoundedQueue<sync_data_t*>>(10);
    sync_th.reset();
    th_ctrl = false;
}

sync_frame_model::sync_frame_model(size_t que_size)
{
    sync_local_pool = std::make_shared<Customize::LocalPool<sync_data_t>>(&sync_data_pool.global_pool);
    sync_data_que = std::make_shared<BoundedQueue<sync_data_t*>>(que_size);
    sync_th.reset();
    th_ctrl = false;
}

sync_frame_model::~sync_frame_model()
{
    // thread stop
    sync_stop();

    // input data release
    _mtx.lock();
    int _size = multi_video_stream.size();
    vstream_data_t tmp;

    for (int i=0; i<_size; i++)
    { 
        multi_video_stream.at(i).video_stream_param.vdqp->Stop();
        for (;;)
        {
            if (!multi_video_stream.at(i).video_stream_param.vdqp->Get(tmp)) {
                break;
            }
            multi_video_stream.at(i).local_pool->ReturnObject(tmp.data);
        }
    }
    multi_video_stream.clear();
    _mtx.unlock();
}

// ---------------------------------------------------------
// thread ctrl
bool sync_frame_model::sync_init()
{
    sync_stop();
    // sync_data_que->Init();

    sync_data_t* s;
    for (;;)
    {
        if (!get_sync_data(s))
            break;
        free_sync_data(s);
    }
    return true;
}

bool sync_frame_model::sync_start()
{
    if (sync_th.get() == nullptr && th_ctrl == false) {
        th_ctrl = true;
        sync_data_que->Start();
        sync_th = std::make_shared<std::thread>(&sync_frame_model::th, this);
        return true;
    }
    return false;
}

void sync_frame_model::sync_stop()
{
    th_ctrl = false;
    sync_data_que->Stop();
    if (sync_th.get() != nullptr) {
        _cond.notify_all();
        sync_th->join();
        sync_th.reset();
    }
}

// ---------------------------------------------------------
bool sync_frame_model::append_video_stream(video_stream_t *vs)
{
    _mtx.lock();

    if (vs->vdqp.get() == nullptr || vs->cgpp.get() == nullptr)
    {
        _mtx.unlock();
        return false;
    }

    ivs tmp;
    tmp.video_stream_param = *vs;
    tmp.local_pool = std::make_shared<cv_gpumat_localpool>(&(vs->cgpp->global_pool));
    multi_video_stream.push_back(tmp);

    _mtx.unlock();
    _cond.notify_all();
    return true;
}

void sync_frame_model::remove_video_stream(video_stream_t *vs)
{
    _mtx.lock();
    int s = multi_video_stream.size();
    for (int i=0; i<s; i++)
    {
        if (multi_video_stream.at(i).video_stream_param.vdqp.get() == vs->vdqp.get()) {
            multi_video_stream.erase(multi_video_stream.begin() + i);
            break;
        }
    }
    _mtx.unlock();
}

void sync_frame_model::remove_all_video_stream()
{
    _mtx.lock();
    multi_video_stream.clear();
    _mtx.unlock();
}

size_t sync_frame_model::video_stream_count()
{
    std::lock_guard<std::mutex> lock(_mtx);
    return multi_video_stream.size();
}

bool sync_frame_model::get_sync_data(sync_data_t *&sync_data)
{
    return sync_data_que->Get((sync_data));
}

bool sync_frame_model::free_sync_data(sync_data_t *sync_data)
{
    _sync_mtx.lock();
    if (sync_data == nullptr) {
        _sync_mtx.unlock();
        return false;
    }

    int d_size = sync_data->sync_data.size();
    for (int i=0; i<d_size; ++i)
    {
        sync_data->multi_video_stream.at(i).local_pool->ReturnObject(sync_data->sync_data.at(i).data);
    }
    sync_data->sync_data.clear();
    sync_data->multi_video_stream.clear();
    sync_local_pool->ReturnObject(sync_data);

    _sync_mtx.unlock();
    return true;
}

// =========================================================
void sync_frame_model::th()
{
    // init
    bool check_All = true;
    int q_size = 0;
    uint32_t maxValue = 0;

    std::vector<vstream_data_t> tmp_data_arr;
    vstream_data_t tmp_d;

    std::mutex tmp_lock;
    std::unique_lock<std::mutex> lock(tmp_lock);
    lock.unlock();

    //printf("[sync] start \n");
    int i;

    start_time = clock_type::now();

    while (th_ctrl)
    {
        _mtx.lock();
        // ----------get new a round data----------
        q_size = multi_video_stream.size();
        tmp_data_arr.clear();
        for (i=0; i<q_size; i++)
        {
            if (!multi_video_stream.at(i).video_stream_param.vdqp->Get(tmp_d))
            {
                multi_video_stream.erase(multi_video_stream.begin() + i);
                q_size = multi_video_stream.size();
                i--;
                continue;
            }

            if (tmp_d.frm_no > maxValue)
            {
                maxValue = tmp_d.frm_no;
            }
            tmp_data_arr.push_back(tmp_d);
        }
        // ----------sync----------
        for (i=0; i<q_size; i++)
        {
            if (tmp_data_arr.at(i).frm_no < maxValue)
            {
                multi_video_stream.at(i).local_pool->ReturnObject(tmp_data_arr.at(i).data);
                if (!multi_video_stream.at(i).video_stream_param.vdqp->Get(tmp_data_arr.at(i)))
                {
                    multi_video_stream.erase(multi_video_stream.begin() + i);
                    q_size = multi_video_stream.size();
                    i--;
                    continue;
                }
                i--;
                continue;
            }

            if (tmp_data_arr.at(i).frm_no > maxValue)
            {
                maxValue = tmp_data_arr.at(i).frm_no;
                i=-1;
                continue;
            }
        }
        // ----------check video stream count----------
        if (multi_video_stream.empty())
        {
            _mtx.unlock();

            maxValue = 0;
            lock.lock();
            _cond.wait(lock);
            lock.unlock();
            continue;
        }
        // --------------use data-------------------
        // get sync data memory
        _sync_mtx.lock();
        sync_data_t* sync_data = sync_local_pool->GetObject();
        if (sync_data == nullptr)
        {
            // release mat data
            for (int i=0; i<q_size; i++)
            {
                multi_video_stream.at(i).local_pool->ReturnObject(tmp_data_arr.at(i).data);
            }
            _sync_mtx.unlock();
            _mtx.unlock();
            continue;
        }
        _sync_mtx.unlock();
        // set sync data memory & push
        sync_data->sync_data = tmp_data_arr;
        sync_data->multi_video_stream = multi_video_stream;    
        _mtx.unlock();
        if (!sync_data_que->Put(sync_data))
        {
            free_sync_data(sync_data);
        }

        sync_count = maxValue;

        end_time = clock_type::now();

        cost_time = end_time - start_time;

        start_time = clock_type::now();

    }
    sync_data_que->Stop();
    //printf("[sync] stop \n");
}

