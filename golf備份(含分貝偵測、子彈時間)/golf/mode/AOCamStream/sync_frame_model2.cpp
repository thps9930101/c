#include "sync_frame_model2.h"

namespace sync_model {

// =========================================================
void sync_frame_model2::_init(size_t bf_size, const sync_output_bf mode)
{
    sync_local_pool = std::make_shared<Customize::LocalPool<sync_data_t>>(&sync_data_pool.global_pool);
    sync_th.reset();
    th_ctrl = false;
    _vs_mtx = new std::mutex;
    //_vs_mtx = std::make_shared<std::mutex>();

    sync_count = 0;
    sync_fail_count = 0;
    sync_lost_count = 0;

    sync_output_mode = mode;
    if (sync_output_mode == que_output)
    {
        sync_data_que = std::make_shared<BoundedQueue<sync_data_t*>>(bf_size);
        get_sync_data = std::bind(&sync_frame_model2::get_sync_data_que, this, std::placeholders::_1);
    }
    else
    {
        sync_data_rb = std::make_shared<RingBuffer<sync_data_t*>>(bf_size);
        sync_data_rb->init();
        get_sync_data = std::bind(&sync_frame_model2::get_sync_data_rb, this, std::placeholders::_1);
        _rb_stop = false;
    }
}

sync_frame_model2::sync_frame_model2()
{
    _init(10, que_output);
}

sync_frame_model2::sync_frame_model2(size_t bf_size)
{
    _init(bf_size, que_output);
}

sync_frame_model2::sync_frame_model2(const sync_output_bf mode)
{
    _init(10, mode);
}

sync_frame_model2::sync_frame_model2(size_t bf_size, const sync_output_bf mode)
{
    _init(bf_size, mode);
}

sync_frame_model2::~sync_frame_model2()
{
    // sync init (thread stop & output data release)
    sync_init();

    // clear video stream ptr
    remove_all_video_stream();
    delete _vs_mtx;
}

// ---------------------------------------------------------
// thread ctrl
bool sync_frame_model2::sync_init()
{
    sync_stop();
    /*
    sync_data_t* s;
    for (;;)
    {
        if (!get_sync_data(s))
            break;
        free_sync_data(s);
    }
    */
    sync_output_clear();



    //log init -------

    start_time = clock_type::now();
    end_time = start_time;
    cost_time = end_time - start_time;

    sync_count = 0;
    sync_fail_count = 0;
    sync_lost_count = 0;

    getdata_start_time = clock_type::now();
    getdata_end_time = getdata_start_time;
    getdata_cost_time = getdata_end_time - getdata_start_time;

    sync_start_time = clock_type::now();
    sync_end_time = sync_start_time;
    sync_cost_time =sync_end_time - sync_start_time;

    push_data_start_time = clock_type::now();
    push_data_end_time = push_data_start_time;
    push_data_cost_time = push_data_end_time - push_data_start_time;

    return true;
}

bool sync_frame_model2::sync_start()
{
    if (sync_th.get() == nullptr && th_ctrl == false)
    {
        sync_init();//
        th_ctrl = true;
        if (sync_output_mode == que_output)
        {
            sync_data_que->Start();
            sync_th = std::make_shared<std::thread>(&sync_frame_model2::th_que_output, this);
        }
        else
        {
            _rb_stop = false;
            sync_th = std::make_shared<std::thread>(&sync_frame_model2::th_rb_output2, this);
        }
        return true;
    }
    return false;
}

void sync_frame_model2::sync_stop()
{
    th_ctrl = false;

    if(sync_output_mode == que_output)
        sync_data_que->Stop();
//    if(sync_output_mode == rb_output)
//        sync_data_rb->clear();

    _rb_stop = true;
    if (sync_th.get() != nullptr) {
        _rb_cond.notify_all();
        _cond.notify_all();
        sync_th->join();
        sync_th.reset();
        printf("sync stop \n");
    }
}

// ---------------------------------------------------------
bool sync_frame_model2::append_video_stream(video_stream_t *vs)
{
    if (vs->stream_rb.get() == nullptr || vs->object_pool.get() == nullptr)
    {
        return false;
    }

    //_vs_mtx.lock();
    //std::lock_guard<std::mutex> lock(*_vs_mtx);
    //bool a = _vs_mtx->try_lock();
    //printf("_vs_mtx %d \n",a);

    int n=0;
    /*
    while(!(_vs_mtx->try_lock()))
    {
        n++;
    }
    */

    /*if(!a)
    {
        //_vs_mtx->unlock();
        printf("_vs_mtx false \n");

        return false;
    }*/
    _vs_mtx->lock();
    multi_video_stream.emplace_back(*vs);
    //multi_video_stream.push_back(*vs);
    //_vs_mtx.unlock();
    //_vs_mtx->unlock();
    _vs_mtx->unlock();
    _cond.notify_all();

    printf("try_lock count %d \n",n);
    return true;
}

void sync_frame_model2::remove_video_stream(video_stream_t *vs)
{
    //_vs_mtx.lock();
    _vs_mtx->lock();
    for (auto vs_it = multi_video_stream.begin(); vs_it != multi_video_stream.end(); ++vs_it)
    {
        if (vs_it->stream_rb.get() == vs->stream_rb.get()) {
            vs_it = multi_video_stream.erase(vs_it);
            break;
        }
    }
    //_vs_mtx.unlock();
    _vs_mtx->unlock();
}

void sync_frame_model2::remove_all_video_stream()
{
    //_vs_mtx.lock();
    _vs_mtx->lock();
    multi_video_stream.clear();
    //_vs_mtx.unlock();
    _vs_mtx->unlock();
}

size_t sync_frame_model2::video_stream_count()
{
    std::lock_guard<std::mutex> lock(*_vs_mtx);
    return multi_video_stream.size();
}

//-----------------------------------------------------------
bool sync_frame_model2::get_sync_data_que(sync_data_t *&sync_data)
{
    return sync_data_que->Get((sync_data));
}

bool sync_frame_model2::get_sync_data_rb(sync_data_t *&sync_data)
{
    sync_data_t** tmp;
    sync_data_rb->getTail(tmp);
    while (tmp == nullptr)
    {
        if (_rb_stop) {
            sync_data = nullptr;
            return false;
        }
        std::unique_lock<std::mutex> _lock(_rb_mtx);
        _rb_cond.wait(_lock);
        sync_data_rb->getTail(tmp);
    }
    sync_data = *tmp;
    return true;
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
    sync_data = nullptr;

    _sync_mtx.unlock();
    return true;
}

void sync_frame_model2::sync_output_clear()
{
    _sync_mtx.lock();

    sync_data_t *sync_data;
    std::vector<video_stream_t>::iterator vs_it;
    std::vector<vstream_data_t>::iterator pool_it;
    while(get_sync_data(sync_data))
    {
        vs_it = sync_data->multi_video_stream.begin();
        pool_it = sync_data->sync_data.begin();

        for (;vs_it!=sync_data->multi_video_stream.end();)
        {
            vs_it->local_pool->ReturnObject(pool_it->data);
            ++vs_it;
            ++pool_it;
        }
        sync_data->sync_data.clear();
        sync_data->multi_video_stream.clear();
        sync_local_pool->ReturnObject(sync_data);
        sync_data = nullptr;
    }

    //rb
    if (sync_output_mode == sync_output_bf::rb_output)
    {
        sync_data_t** tmp;
        sync_data_rb->getTail_clear(tmp);
        if (tmp != nullptr)
        {
            sync_data = *tmp;
            tmp =nullptr;

            vs_it = sync_data->multi_video_stream.begin();
            pool_it = sync_data->sync_data.begin();

            for (;vs_it!=sync_data->multi_video_stream.end();)
            {
                vs_it->local_pool->ReturnObject(pool_it->data);
                ++vs_it;
                ++pool_it;
            }
            sync_data->sync_data.clear();
            sync_data->multi_video_stream.clear();
            sync_local_pool->ReturnObject(sync_data);
            sync_data = nullptr;
        }
        sync_data_rb->clear();
    }

    _sync_mtx.unlock();
}

// =========================================================
void sync_frame_model2::th_que_output()
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
        //_vs_mtx.lock();
        _vs_mtx->lock();

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
                sync_fail_count ++;
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
            //_vs_mtx.unlock();
            _vs_mtx->unlock();

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
            //_vs_mtx.unlock();
            _vs_mtx->unlock();
            continue;
        }
        _sync_mtx.unlock();

        // set sync data memory & push
        sync_data->sync_data = tmp_data_arr;
        sync_data->multi_video_stream = multi_video_stream;
        //_vs_mtx.unlock();
        _vs_mtx->unlock();

        sync_count = sync_data->sync_data.at(0).frm_no;

        if (!sync_data_que->Put(sync_data))
        {
            sync_lost_count++;
            free_sync_data(sync_data);
            continue;
        }        

        end_time = clock_type::now();
        cost_time = end_time - start_time;
        start_time = clock_type::now();


    }
    sync_data_que->Stop();
}

void sync_frame_model2::th_rb_output()
{
    uint32_t maxValue = 0;
    std::vector<vstream_data_t> tmp_data_arr;
    vstream_data_t* tmp_d;
    sync_data_t* sync_data;
    sync_data_t** sync_data_addr;

    //std::mutex* _vs_lock = _vs_mtx;
    //std::shared_ptr<std::mutex> _vs_lock = _vs_mtx;
    std::unique_lock<std::mutex> _vs_lock(*_vs_mtx);
    _vs_lock.unlock();

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
        //_vs_mtx->lock();
        //_vs_lock->lock();
        _vs_lock.lock();
        // ----------get new a round data----------
        getdata_start_time = clock_type::now();
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
            tmp_d->data = nullptr;
            ++vs_it;
        }

        getdata_end_time = clock_type::now();
        getdata_cost_time = getdata_end_time - getdata_start_time;
        // ----------sync----------
        sync_start_time = clock_type::now();

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
                tmp_d->data = nullptr;
                continue;
            }
            if (data_it->frm_no > maxValue)
            {
                sync_fail_count ++;
                maxValue = data_it->frm_no;
                data_it = tmp_data_arr.begin();
                vs_it = multi_video_stream.begin();
                continue;
            }
            ++data_it;
            ++vs_it;
        }

        sync_end_time = clock_type::now();
        sync_cost_time = sync_end_time - sync_start_time;
        // ----------check video stream count----------
         if (multi_video_stream.empty())
        {
            //_vs_mtx->unlock();
            //_vs_lock->unlock();
            _vs_lock.unlock();

            maxValue = 0;
            lock.lock();
            _cond.wait(lock);
            lock.unlock();
            continue;
        }
        // --------------use data-------------------
        // get sync data memory
        push_data_start_time = clock_type::now();

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
            //_vs_mtx->unlock();
            //_vs_lock->unlock();
            _vs_lock.unlock();
            continue;
        }
        _sync_mtx.unlock();

        // set sync data memory & push
        sync_data->sync_data = tmp_data_arr;
        sync_data->multi_video_stream = multi_video_stream;

        //_vs_mtx->unlock();
        //_vs_lock->unlock();
        _vs_lock.unlock();


        //printf("_vs_lock.unlock \n");
        sync_count = sync_data->sync_data.at(0).frm_no;

        sync_data_rb->getHead(sync_data_addr);
        //sync_data_addr = nullptr;
        if (sync_data_addr == nullptr)
        {
            sync_lost_count++;
            free_sync_data(sync_data);
            continue;
        }

        *sync_data_addr = sync_data;
        _rb_cond.notify_all();

        push_data_end_time = clock_type::now();
        push_data_cost_time = push_data_end_time - push_data_start_time;

        end_time = clock_type::now();
        cost_time = end_time - start_time;
        start_time = clock_type::now();

    }


    printf("sy rb end \n");
    _rb_cond.notify_all();
    //sync_data_que->Stop();
}

void sync_frame_model2::th_rb_output2()
{
    uint32_t maxValue = 0;
    std::vector<video_stream_t> tmp_multi_vstream;
    std::vector<vstream_data_t> tmp_data_arr;
    vstream_data_t* tmp_d;
    std::mutex* _vs_lock = _vs_mtx;

    sync_data_t* sync_data;
    sync_data_t** sync_data_addr;

    std::mutex tmp_lock;
    std::unique_lock<std::mutex> lock(tmp_lock);
    lock.unlock();

    auto data_it = tmp_data_arr.begin();
    auto vs_it = tmp_multi_vstream.begin();

    start_time = clock_type::now();
    end_time = clock_type::now();

    // get data
    auto get_ring_data = [&]()
    {
        vs_it->stream_rb->getTail(tmp_d);
//        printf("-------[snyc]:1-------\n");

        while (tmp_d == nullptr)
        {
//            printf("-------[snyc]:2-------\n");
            if ((*(vs_it->rb_stop))) {
                return false;
            }
//            printf("-------[snyc]:3-------\n");
            std::unique_lock<std::mutex> _lock(*(vs_it->rb_lock));
            vs_it->rb_condi->wait(_lock);
            vs_it->stream_rb->getTail(tmp_d);
//            printf("-------[snyc]:4-------\n");
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
//        printf("-------[snyc]:5-------\n");
        // ----------get new a round data----------
        getdata_start_time = clock_type::now();
        _vs_lock->lock();
        tmp_data_arr.clear();
        vs_it = multi_video_stream.begin();
        for (;vs_it!=multi_video_stream.end();)
        {
//            printf("-------[snyc]:5-1-------\n");
            if (!get_ring_data())
            {
//                printf("-------[snyc]:5-2-------\n");
                vs_it = multi_video_stream.erase(vs_it);
                continue;
            }
//            printf("-------[snyc]:5-3-------\n");

            // change max value
            if (tmp_d->frm_no > maxValue) {
                maxValue = tmp_d->frm_no;
            }
            tmp_data_arr.emplace_back(*tmp_d);
            tmp_d->data = nullptr;
            ++vs_it;
        }
        tmp_multi_vstream = multi_video_stream;
        _vs_lock->unlock();
//        printf("-------[snyc]:6-------\n");
        getdata_end_time = clock_type::now();
        getdata_cost_time = getdata_end_time - getdata_start_time;
        // ----------sync----------
        sync_start_time = clock_type::now();

        data_it = tmp_data_arr.begin();
        vs_it = tmp_multi_vstream.begin();

        for (;data_it!=tmp_data_arr.end();)
        {
            if (data_it->frm_no < maxValue)
            {
//                 printf("-------[snyc]:6-1-------\n");
                vs_it->local_pool->ReturnObject(data_it->data);
                if (!get_ring_data())
                {
//                    printf("-------[snyc]:7-------\n");
                    vs_it = tmp_multi_vstream.erase(vs_it);
                    data_it = tmp_data_arr.erase(data_it);
                    continue;
                }
//                printf("-------[snyc]:8-------\n");
                (*data_it) = *tmp_d;
                tmp_d->data = nullptr;
                continue;
            }
            if (data_it->frm_no > maxValue)
            {
//                printf("-------[snyc]:9-------\n");
                sync_fail_count ++;
                maxValue = data_it->frm_no;
                data_it = tmp_data_arr.begin();
                vs_it = tmp_multi_vstream.begin();
                continue;
            }
//            printf("-------[snyc]:10-------\n");
            ++data_it;
            ++vs_it;
        }

        sync_end_time = clock_type::now();
        sync_cost_time = sync_end_time - sync_start_time;
        // ----------check video stream count----------
//        printf("-------[snyc]:11-------\n");
        if (tmp_multi_vstream.empty())
        {
//            printf("-------[snyc]:12-------\n");
            maxValue = 0;
            lock.lock();
            _cond.wait(lock);
            lock.unlock();
            continue;
        }
        // --------------use data-------------------
        // get sync data memory
//        printf("-------[snyc]:13-------\n");
        push_data_start_time = clock_type::now();

        _sync_mtx.lock();
        sync_data = sync_local_pool->GetObject();
        if (sync_data == nullptr)
        {
//            printf("-------[snyc]:14-------\n");
            data_it = tmp_data_arr.begin();
            vs_it = tmp_multi_vstream.begin();
            for (;vs_it!=tmp_multi_vstream.end();)
            {
                vs_it->local_pool->ReturnObject(data_it->data);
                ++vs_it;
                ++data_it;
            }
            _sync_mtx.unlock();
            continue;
        }
        _sync_mtx.unlock();
//        printf("-------[snyc]:15-------\n");
        // set sync data memory & push
        sync_data->sync_data = tmp_data_arr;
        sync_data->multi_video_stream = tmp_multi_vstream;

        //printf("_vs_lock.unlock \n");
        sync_count = sync_data->sync_data.at(0).frm_no;

        sync_data_rb->getHead(sync_data_addr);
        //sync_data_addr = nullptr;
        if (sync_data_addr == nullptr)
        {
//            printf("-------[snyc]:16-------\n");
            sync_lost_count++;
            free_sync_data(sync_data);
            continue;
        }

        *sync_data_addr = sync_data;
        _rb_cond.notify_all();

        push_data_end_time = clock_type::now();
        push_data_cost_time = push_data_end_time - push_data_start_time;

        end_time = clock_type::now();
        cost_time = end_time - start_time;
        start_time = clock_type::now();
//        printf("-------[snyc]:17-------\n");
    }


    printf("sy rb end \n");
    _rb_cond.notify_all();
    //sync_data_que->Stop();
}

}
