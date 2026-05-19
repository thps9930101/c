#include "sync_frame_model2.h"

namespace sync_model {

// =========================================================
sync_frame_model2::sync_frame_model2()
{
    sync_local_pool = std::make_shared<Customize::LocalPool<sync_data_t>>(&sync_data_pool.global_pool);
    sync_data_que = std::make_shared<BoundedQueue<sync_data_t*>>(10);
    sync_th.reset();
    th_ctrl = false;
}

sync_frame_model2::sync_frame_model2(size_t que_size)
{
    sync_local_pool = std::make_shared<Customize::LocalPool<sync_data_t>>(&sync_data_pool.global_pool);
    sync_data_que = std::make_shared<BoundedQueue<sync_data_t*>>(que_size);
    sync_th.reset();
    th_ctrl = false;
}

sync_frame_model2::~sync_frame_model2()
{
    // sync init (thread stop & output data release)
    sync_init();

    // clear video stream ptr
    remove_all_video_stream();
}

// ---------------------------------------------------------
// thread ctrl
bool sync_frame_model2::sync_init()
{
    sync_stop();
    sync_data_t* s;
    for (;;)
    {
        if (!get_sync_data(s))
            break;
        free_sync_data(s);
    }
    return true;
}

bool sync_frame_model2::sync_start()
{
    if (sync_th.get() == nullptr && th_ctrl == false) {
        th_ctrl = true;
        sync_data_que->Start();
        sync_th = std::make_shared<std::thread>(&sync_frame_model2::th, this);
        return true;
    }
    return false;
}

void sync_frame_model2::sync_stop()
{
    th_ctrl = false;
    sync_data_que->Stop();
    if (sync_th.get() != nullptr) {
        _cond.notify_all();
        sync_th->join();
        sync_th.reset();
        printf("sync stop \n");
    }
}

// ---------------------------------------------------------
bool sync_frame_model2::append_video_stream(video_stream_t *vs)
{
    _vs_mtx.lock();

    if (vs->stream_rb.get() == nullptr || vs->object_pool.get() == nullptr)
    {
        _vs_mtx.unlock();
        return false;
    }
    multi_video_stream.emplace_back(*vs);

    _vs_mtx.unlock();
    _cond.notify_all();
    return true;
}

void sync_frame_model2::remove_video_stream(video_stream_t *vs)
{
    _vs_mtx.lock();
    for (auto vs_it = multi_video_stream.begin(); vs_it != multi_video_stream.end(); ++vs_it)
    {
        if (vs_it->stream_rb.get() == vs->stream_rb.get()) {
            vs_it = multi_video_stream.erase(vs_it);
            break;
        }
    }
    _vs_mtx.unlock();
}

void sync_frame_model2::remove_all_video_stream()
{
    _vs_mtx.lock();
    multi_video_stream.clear();
    _vs_mtx.unlock();
}

size_t sync_frame_model2::video_stream_count()
{
    std::lock_guard<std::mutex> lock(_vs_mtx);
    return multi_video_stream.size();
}

bool sync_frame_model2::get_sync_data(sync_data_t *&sync_data)
{
    return sync_data_que->Get((sync_data));
}

bool sync_frame_model2::free_sync_data(sync_data_t *sync_data)
{
    _sync_mtx.lock();
    if (sync_data == nullptr) {
        _sync_mtx.unlock();
        return false;
    }

    auto vs_it = sync_data->multi_video_stream.begin();
    auto pool_it = sync_data->sync_data.begin();
    for (;vs_it!=sync_data->multi_video_stream.end();)
    {
        vs_it->local_pool->ReturnObject(pool_it->data);
        ++vs_it;
        ++pool_it;
    }
    sync_data->sync_data.clear();
    sync_data->multi_video_stream.clear();
    sync_local_pool->ReturnObject(sync_data);

    _sync_mtx.unlock();
    return true;
}

// =========================================================
void sync_frame_model2::th()
{
    uint32_t maxValue = 0;
    std::vector<vstream_data_t> tmp_data_arr;
    vstream_data_t* tmp_d;
    sync_data_t* sync_data;

    std::mutex tmp_lock;
    std::unique_lock<std::mutex> lock(tmp_lock);
    lock.unlock();

    auto data_it = tmp_data_arr.begin();
    auto vs_it = multi_video_stream.begin();

    start_time = clock_type::now();
    end_time = clock_type::now();

    // get data

    auto get_ring_data = [&]()
    {
        vs_it->stream_rb->getTail(tmp_d);
        while (tmp_d == nullptr)
        {
            if ((*(vs_it->rb_stop))) {
                return false;
            }
            std::unique_lock<std::mutex> _lock(*(vs_it->rb_lock));
            vs_it->rb_condi->wait(_lock);
            vs_it->stream_rb->getTail(tmp_d);
        }
        return true;
    };


    /*
    auto get_ring_data = [&]()
    {
        std::unique_lock<std::mutex> _lock(*(vs_it->rb_lock));
        vs_it->rb_condi->wait(_lock, [&]() {
            vs_it->stream_rb->getTail(tmp_d);
            return !(tmp_d == nullptr) || (*(vs_it->rb_stop));
        });
        if ((*(vs_it->rb_stop)))
            return false;
        return true;
    };
    */


    while (th_ctrl)
    {
        _vs_mtx.lock();
        // ----------get new a round data----------
        tmp_data_arr.clear();
        vs_it = multi_video_stream.begin();
        for (;vs_it!=multi_video_stream.end();)
        {
            if (!get_ring_data())
            {
                vs_it = multi_video_stream.erase(vs_it);
                continue;
            }

            // change max value
            if (tmp_d->frm_no > maxValue) {
                maxValue = tmp_d->frm_no;
            }
            tmp_data_arr.emplace_back(*tmp_d);
            ++vs_it;
        }

        // ----------sync----------
        data_it = tmp_data_arr.begin();
        vs_it = multi_video_stream.begin();
        for (;data_it!=tmp_data_arr.end();)
        {
            if (data_it->frm_no < maxValue)
            {
                vs_it->local_pool->ReturnObject(data_it->data);
                if (!get_ring_data())
                {
                    vs_it = multi_video_stream.erase(vs_it);
                    data_it = tmp_data_arr.erase(data_it);
                    continue;
                }
                (*data_it) = *tmp_d;
                continue;
            }
            if (data_it->frm_no > maxValue)
            {
                maxValue = data_it->frm_no;
                data_it = tmp_data_arr.begin();
                vs_it = multi_video_stream.begin();
                continue;
            }
            ++data_it;
            ++vs_it;
        }

        // ----------check video stream count----------
        if (multi_video_stream.empty())
        {
            _vs_mtx.unlock();

            maxValue = 0;
            lock.lock();
            _cond.wait(lock);
            lock.unlock();
            continue;
        }

        // --------------use data-------------------
        // get sync data memory
        _sync_mtx.lock();
        sync_data = sync_local_pool->GetObject();
        if (sync_data == nullptr)
        {
            data_it = tmp_data_arr.begin();
            vs_it = multi_video_stream.begin();
            for (;vs_it!=multi_video_stream.end();)
            {
                vs_it->local_pool->ReturnObject(data_it->data);
                ++vs_it;
                ++data_it;
            }
            _sync_mtx.unlock();
            _vs_mtx.unlock();
            continue;
        }
        _sync_mtx.unlock();

        // set sync data memory & push
        sync_data->sync_data = tmp_data_arr;
        sync_data->multi_video_stream = multi_video_stream;
        _vs_mtx.unlock();

        sync_count = sync_data->sync_data.at(0).frm_no;
        free_sync_data(sync_data);

        /*if (!sync_data_que->Put(sync_data))
        {
            free_sync_data(sync_data);
        }*/



        end_time = clock_type::now();
        cost_time = end_time - start_time;
        start_time = clock_type::now();

    }
    sync_data_que->Stop();
}

}
