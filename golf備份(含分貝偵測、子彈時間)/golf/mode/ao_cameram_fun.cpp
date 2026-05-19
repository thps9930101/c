#include "ao_cameram_fun.h"

#include <QDateTime>
#include <QFile>
#include <QTextStream>


using clock_type = std::chrono::high_resolution_clock;
using milli_type = std::chrono::duration<double,std::milli>;
using micro_type = std::chrono::duration<double,std::micro>;

//auto t1 = clock_type::now();
//printf("time3 [%lf]\n",milli_type(t2-t1).count());
typedef std::chrono::time_point<std::chrono::steady_clock,std::chrono::duration<long long ,std::ratio<1,1000000000>>> _time;

// 每秒 60 張，保存 20 秒，最大一張 frame 大小 500KB
//FrameBuffer vid_buffer(60, 20, 1000);

ao_cameram_fun::ao_cameram_fun():sync_frame2(5,sync_model::sync_frame_model2::rb_output)
{

}

ao_cameram_fun::~ao_cameram_fun()
{
    for (auto cam_it = Cam.begin(); cam_it != Cam.end(); ++cam_it)
    {
        delete cam_it->Cam_d->ao_cam;
        cam_it->Cam_d->ao_cam = nullptr;
    }
    Cam.clear();
    printf("~ao_cameram_fun \n");
}

uint32_t  i_size=0;
uint32_t  p_size=0;

//------------------------------------------------------------------------------------
static void on_265(std::shared_ptr<Cam_data> Cam,Cam_Video *Cam_vid,struct ao::lm_video_data *h265)
{
    int ret;
//    printf("video ntp_timestamp[%llu] \n",h265->ntp_timestamp);

//    printf("video [%s] [%d] [%d] [%d] [%llu]\n",Cam->ao_cam->Cam_IP,h265->frm_no,h265->type,h265->len,h265->ntp_timestamp);

    //---
    /*if(h265->type == 1)
    {
        if(h265->len > i_size)
            i_size = h265->len;
    }
    else
    {
        if(h265->len > p_size)
            p_size = h265->len;
    }

    //FHD I 500KB  P  250KB   200/100
    //4K  I   2MB  P    1MB

    //printf("max I = [%d] max P = [%d] \n",i_size,p_size);*/
    //----

    ao::lm_video_data *temp;

    //加入至buffer
//    vid_buffer.addFrame(Cam->ao_cam->Cam_IP, *h265);

    Cam->cam_data_count = h265->frm_no;
    //-----------------------------------

    Cam->cam_record_start_time = clock_type::now();
    if(Cam_vid->Video_State.save)
    {
        Cam_vid->Video_State.endFNo = 0;
        if(Cam_vid->Video_State.startFNo ==0 && h265->type == 1)
            Cam_vid->Video_State.startFNo = h265->frm_no +10;

        if(Cam_vid->Video_State.startFNo != 0 && Cam_vid->Video_State.startFNo <= h265->frm_no)
        {
          if(Cam->Video_FirstOpen)
          {
              if(Cam_vid->Video_State.start_ntp_timestamp == 0)
                  Cam_vid->Video_State.start_ntp_timestamp = h265->ntp_timestamp;
              Cam->cam_video_path = Cam_vid->Video_Path + "/" + to_string(Cam->CamNO)+".mp4";
              ret = Cam->Cam_Video.create(Cam->cam_video_path.c_str(),Cam_vid->vid_ctx->ctx);
              if(ret<0)
              {
                  Cam_vid->Video_State.save = false;
                  Cam->video_statu = false;
                  Cam->Video_FirstOpen = true;
              }
              Cam->Video_FirstOpen = false;
              printf("[%u]FNO start[%d] \n",Cam->CamNO,h265->frm_no);
          }

          Cam->Cam_Video.write_pkt_data(h265->data,h265->len);

        }
        Cam->video_statu = true;
    }
    else
    {
        if(Cam->video_statu == true)
        {
            if(Cam_vid->Video_State.endFNo == 0)
                Cam_vid->Video_State.endFNo = h265->frm_no+10;

            if(Cam_vid->Video_State.endFNo == 0 || (Cam_vid->Video_State.endFNo) >= h265->frm_no)
            {
                Cam->Cam_Video.write_pkt_data(h265->data,h265->len);
            }
            else
            {
                Cam_vid->Video_State.end_ntp_timestamp = h265->ntp_timestamp;
                Cam_vid->Video_State.startFNo = 0;
                Cam->video_statu=false;
                Cam->Video_FirstOpen = true;
                printf("[%u]FNO end[%d] \n",Cam->CamNO,h265->frm_no);
                Cam->Cam_Video.release();
            }
        }
    }
    Cam->cam_record_end_time  = clock_type::now();

    Cam->cam_record_cost_time  = Cam->cam_record_end_time - Cam->cam_record_start_time;
    //-----------------------------------
    if(Cam->vid_data_containers_full == true && h265->type !=1)
    {
        Cam->cam_data_lost_count ++;
        return;
    }
//    printf("video temp null [%s] [%d] [%d] [%d] [%llu]\n",Cam->ao_cam->Cam_IP,h265->frm_no,h265->type,h265->len,h265->ntp_timestamp);
    Cam->vid_data_buf->getHead(temp);

    if(temp == nullptr)
    {
        Cam->cam_data_lost_count ++;
        Cam->vid_data_containers_full = true;
        //printf("video temp null [%s] [%d] [%d] [%d] [%llu]\n",Cam->ao_cam->Cam_IP,h265->frm_no,h265->type,h265->len,h265->ntp_timestamp);
        return;
    }

    Cam->vid_data_containers_full = false;
    temp->len = h265->len;
    temp->type = h265->type;
    temp->frm_no = h265->frm_no;
    temp->ntp_timestamp = h265->ntp_timestamp;
    memcpy(temp->data, h265->data, h265->len);

    Cam->vid_data_buf->data_lock_cv.notify_all();



}

uint32_t  a_size=0;
float db = 0;

float ao_cameram_fun::returnDB()
{
    return db;
}

static void on_audio_data(std::shared_ptr<Cam_data> Cam,Cam_Video *Cam_vid,Output<output_file> *out_file,Output<output_rtsp> *out_rtsp,Output<output_rtmp> *out_rtmp,struct ao::lm_audio_data *audio)
{
//    printf("audio ntp_timestamp[%llu] \n",audio->ntp_timestamp);
    //printf("audio len[%llu] \n",audio->len);
    //if(audio->len > 450)
        //printf("audio len[%llu] \n",audio->len);
    /*if(audio->len > a_size)
    {
        a_size = audio->len;
        printf("max au = [%d] \n",a_size);
    }*/

//    printf("audio[%s] [%d] [%d] [%llu]\n",Cam->ao_cam->Cam_IP,audio->frm_no,audio->len,audio->ntp_timestamp);
//    analyzer->analyze(*audio);  // ✅ 直接傳入
//    vid_buffer.processAudioData(*audio);
//    db = vid_buffer.returnDB();
    if(Cam_vid->Video_State.save)
    {
        if(Cam->Cam_Video.is_open())
        {
            if((Cam_vid->Video_State.start_ntp_timestamp != 0 && Cam_vid->Video_State.start_ntp_timestamp <= audio->ntp_timestamp) && (Cam_vid->Video_State.end_ntp_timestamp == 0 || Cam_vid->Video_State.end_ntp_timestamp >= audio->ntp_timestamp))
            {
                Cam->Cam_Video.write_pkt_au(audio->data,audio->len);
            }
        }
    }

    if(out_file->Video_State.save)
    {
        if(out_file->output.is_open())
        {
            if((out_file->Video_State.start_ntp_timestamp != 0 && out_file->Video_State.start_ntp_timestamp <= audio->ntp_timestamp) && (out_file->Video_State.end_ntp_timestamp == 0 || out_file->Video_State.end_ntp_timestamp >= audio->ntp_timestamp))
            {
                out_file->output.write_au(audio->data,audio->len);
            }
        }
    }

    if(out_rtsp->Video_State.save)
    {
        if(out_rtsp->output.is_open())
        {
            if((out_rtsp->Video_State.start_ntp_timestamp != 0 && out_rtsp->Video_State.start_ntp_timestamp <= audio->ntp_timestamp) && (out_rtsp->Video_State.end_ntp_timestamp == 0 || out_rtsp->Video_State.end_ntp_timestamp >= audio->ntp_timestamp))
            {
                out_rtsp->output.write_au(audio->data,audio->len);
            }
        }
    }

    if(out_rtmp->Video_State.save)
    {
        if(out_rtmp->output.is_open())
        {
            if((out_rtmp->Video_State.start_ntp_timestamp != 0 && out_rtmp->Video_State.start_ntp_timestamp <= audio->ntp_timestamp) && (out_rtmp->Video_State.end_ntp_timestamp == 0 || out_rtmp->Video_State.end_ntp_timestamp >= audio->ntp_timestamp))
            {
                out_rtmp->output.write_au(audio->data,audio->len);
            }
        }
    }
}

static void on_Connect(std::shared_ptr<Cam_data> Cam)
{
    Cam->connect_state = true;
    printf("[%s] OnConnect\n",Cam->ao_cam->Cam_IP);
}

std::function<void(const QString&, const QString&)> ao_cameram_fun::notifyFunc = nullptr;

static void on_Disconnect(std::shared_ptr<Cam_data> Cam)
{
    Cam->DC_state = false;
    Cam->vid_data_buf->data_lock_cv.notify_all();

    //sync----------------
    *(Cam->sy_stop) = true;
    Cam->sy_rb_condi->notify_all();

    Cam->connect_state = false;
    printf("[%s] OnDisconnect\n",Cam->ao_cam->Cam_IP);
    if (ao_cameram_fun::notifyFunc) {
        ao_cameram_fun::notifyFunc(Cam->ao_cam->Cam_IP, "Disconnect");
    }
}

static void on_Exception(std::shared_ptr<Cam_data> Cam)
{
    Cam->DC_state = false;
    Cam->vid_data_buf->data_lock_cv.notify_all();

    //sync----------------
    *(Cam->sy_stop) = true;
    Cam->sy_rb_condi->notify_all();

    Cam->connect_state = false;
    printf("[%s] OnException\n",Cam->ao_cam->Cam_IP);
    if (ao_cameram_fun::notifyFunc) {
        ao_cameram_fun::notifyFunc(Cam->ao_cam->Cam_IP, "Exception");
    }
}

static void on_VideoMeta(std::shared_ptr<Cam_data> Cam,struct ao::lm_video_meta *vid_data)
{
    printf("cam [%s] width [%u] height [%u] fps [%u] gop [%u] \n"
           ,Cam->ao_cam->Cam_IP,vid_data->width,vid_data->height,vid_data->fps,vid_data->gop);
}

static void on_AudioMeta(std::shared_ptr<Cam_data> Cam,struct ao::lm_audio_meta *au_data)
{
    printf("cam [%s] sample_rate [%u] sample_size [%u] channel [%u] bitrate [%u] encoder [%u]\n"
           ,Cam->ao_cam->Cam_IP,au_data->sample_rate,au_data->sample_size,au_data->channel
           ,au_data->bitrate,au_data->encoder);
}

//------------------------------------------------------------------------------------
static void vid_data_init(ao::lm_video_data* d, int size)
{
    d->data = new uint8_t[size];
}

static void vid_data_del(ao::lm_video_data* d)
{
    if (d->data == nullptr)
        return;
    delete[] d->data;
    d->data = nullptr;
}

static void Mat_data_init(cv::cuda::GpuMat* d , int w , int h , int ch)
{
    if(ch == 1) //CV_8UC1
        d->create(h,w,CV_8UC1);
    if(ch == 2) //CV_8UC2
        d->create(h,w,CV_8UC2);
    if(ch == 3) //CV_8UC3
        d->create(h,w,CV_8UC3);
    if(ch == 4) //CV_8UC4
        d->create(h,w,CV_8UC4);

}

static void Mat_data_del(cv::cuda::GpuMat* d)
{
    d->release();
    delete d;
}

static unsigned int IP_StringToInt(const std::string& ip_str)
{
    std::vector<int> ip_parts;
    std::istringstream iss(ip_str);
    std::string part;

    while (std::getline(iss, part, '.')) {
        ip_parts.push_back(std::stoi(part));
    }

    if (ip_parts.size() == 4) {
        return (ip_parts[0] << 24) | (ip_parts[1] << 16) | (ip_parts[2] << 8) | ip_parts[3];
    } else {
        printf("Invalid IP address format. \n");
        return 0;
    }
}

//------------------------------------------------------------------------------------
//public

void ao_cameram_fun::init(const Decoder_class_param dc_par,const Encoder_param ec_par)
{
    Write_Log_File("Cam_Func","Cam_fun init ......");
    this->dc_par = dc_par;
    this->ec_par = ec_par;
    qDebug()<<"ec_par"<<ec_par.en_format;
    Live_show_Mat = new RingBuffer<GpuMat_Struct>(5);
    Live_show_Mat->init();

    sp = new splice(&sync_frame2);
    sp->set_Mode(SP_MODE::sp_4k_4x3);


    //------log init---------
    sp_out_in_count = 0;

    sp_out_start_time =  clock_type::now();
    sp_out_end_time = sp_out_start_time;
    sp_out_cost_time = sp_out_end_time - sp_out_start_time;

    sp_out_start_time2 =  clock_type::now();
    sp_out_end_time2 = sp_out_start_time2;
    sp_out_cost_time2 = sp_out_end_time2 - sp_out_start_time2;

    conver_start_time = clock_type::now();
    conver_end_time = conver_start_time;
    conver_cost_time = conver_end_time - conver_start_time;

    en_start_time = clock_type::now();
    en_end_time = en_start_time;
    en_cost_time = en_end_time - en_start_time;

    view_start_time = clock_type::now();
    view_end_time = view_start_time;
    view_cost_time = view_end_time -view_start_time;

    Write_Log_File("Cam_Func","Cam_fun init ...... end");
}

uint32_t ao_cameram_fun::AddCam(const char* ip)
{
    QString msg;
    msg = QString("Add Cam %1 ......").arg(QString::fromUtf8(ip));
    Write_Log_File("Cam_Func",msg);
    std::pair<std::set<Cam_staruct>::iterator,bool> result;

    uint32_t id = IP_StringToInt(ip);
    printf("number: %u \n", id);

    if(id == 0)
    {
        msg = QString("Add Cam %1 Error.").arg(QString::fromUtf8(ip));
        Write_Log_File("Cam_Func",msg);
        return 0;
    }

    msg = QString("Add Cam %1 id:%2 ").arg(QString::fromUtf8(ip)).arg(QString::number(id));
    Write_Log_File("Cam_Func",msg);

    if(Cam_vid.vid_ctx == nullptr)
    {
        Cam_vid.vid_ctx = new multiple_vid_ctx();
        Cam_vid.vid_ctx->create(Cam_vid.Cam_Video_param);
    }

    Cam_staruct cam;
    cam.id = id;
    cam.Cam_d = std::make_shared<Cam_data>();

    cam.Cam_d->vid_data_buf = new RingBuffer<ao::lm_video_data>(30,8*1024*1024,vid_data_init,vid_data_del);
    cam.Cam_d->vid_data_buf->init();

    cam.Cam_d->ao_cam = new ao_readcam(ip);
    cam.Cam_d->ao_cam->setOnH265DataCallback(std::bind(on_265,cam.Cam_d,&Cam_vid,std::placeholders::_1));
    cam.Cam_d->ao_cam->setOnAudioDataCallback(std::bind(on_audio_data,cam.Cam_d,&Cam_vid,&out_file,&out_rtsp,&out_rtmp,std::placeholders::_1));
    cam.Cam_d->ao_cam->setOnConnectCallback(std::bind(on_Connect,cam.Cam_d));
    cam.Cam_d->ao_cam->setOnDisconnectedCallback(std::bind(on_Disconnect,cam.Cam_d));
    cam.Cam_d->ao_cam->setOnExceptionCallback(std::bind(on_Exception,cam.Cam_d));

    cam.Cam_d->ao_cam->setOnVideoMetaCallback(std::bind(on_VideoMeta,cam.Cam_d,std::placeholders::_1));
    cam.Cam_d->ao_cam->setOnAudioMetaCallback(std::bind(on_AudioMeta,cam.Cam_d,std::placeholders::_1));
//--------------------------------------------------------------------------------------
    cam.Cam_d->ao_cam->init();

    cam.Cam_d->dc = new Decoder_class();
    cam.Cam_d->dc->create(dc_par);

    cam.Cam_d->sy_stop = std::make_shared<bool>(false);
    cam.Cam_d->sy_rb_lock = std::make_shared<std::mutex>();
    cam.Cam_d->sy_rb_condi = std::make_shared<std::condition_variable>();
    cam.Cam_d->stream_rb = std::make_shared<sync_model::vstream_data_rb>(30);
    cam.Cam_d->stream_rb->init();
    cam.Cam_d->object_pool = std::make_shared<sync_model::cv_gpumat_pool>();
    cam.Cam_d->sy_local_pool = std::make_shared<sync_model::cv_gpumat_localpool>(&cam.Cam_d->object_pool->global_pool);
    cam.Cam_d->dc_local_pool = std::make_shared<sync_model::cv_gpumat_localpool>(&cam.Cam_d->object_pool->global_pool);

    cam.Cam_d->CamNO = id;

    cam.Cam_d->dc_Live_show = false;
    cam.Cam_d->connect_state = false;
    cam.Cam_d->DC_state = false;

    //----------------------
    cam.Cam_d->video_statu = false;
    cam.Cam_d->Video_FirstOpen = true;

    //-----------
    cam.Cam_d->cam_data_count = 0;
    cam.Cam_d->cam_data_lost_count = 0;

    cam.Cam_d->dc2_count = 0;
    cam.Cam_d->dc2_lost_count = 0;

    if(!Cam.insert(cam).second) {
        id = 0;
        delete cam.Cam_d->ao_cam;
        cam.Cam_d->ao_cam = nullptr;
    }

    if(sp_view_num.size() < Cam.size())
    {
        sp_view_num.push_back(id);
        sp->set_view_num(sp_view_num);
    }

    if(sp_view_rot.size() < Cam.size())
    {
        sp_view_rot.push_back(SP_Rot::sp_None);
        sp->set_view_rot(sp_view_rot);
    }


//    vid_buffer.init(ip);
    //------------------
    sync_frame2.sync_start();

    sp->sp_start();

    if(sp_output_th == nullptr)
    {
         sp_output_state = true;
         sp_output_th = new std::thread(&ao_cameram_fun::sp_output,this,&sp_output_state);
    }

    msg = QString("Add Cam %1 ...... end").arg(QString::fromUtf8(ip));
    Write_Log_File("Cam_Func",msg);



    return id;

}

void ao_cameram_fun::DeleteCam(uint32_t cam_id)
{
    QString msg;

    auto cam_it = Cam.find(Cam_staruct{cam_id, nullptr});

    msg = QString("Delete Cam id:%1 ......").arg(QString::number(cam_id));
    Write_Log_File("Cam_Func",msg);

    if(cam_it != Cam.end())
    {
        cam_it->Cam_d->ao_cam->stop();

        cam_it->Cam_d->DC_state = false;
        *(cam_it->Cam_d->sy_stop) = true;

        cam_it->Cam_d->sy_rb_condi->notify_all();

        cam_it->Cam_d->vid_data_buf->data_lock_cv.notify_all();
        if(cam_it->Cam_d->dc_thr != nullptr)
        {
            cam_it->Cam_d->dc_thr->join();
            delete  cam_it->Cam_d->dc_thr;
            cam_it->Cam_d->dc_thr = nullptr;
        }
        delete cam_it->Cam_d->ao_cam;
        cam_it->Cam_d->ao_cam = nullptr;
        Cam.erase(cam_it);
    }

    msg = QString("Delete Cam id:%1 ...... end").arg(QString::number(cam_id));
    Write_Log_File("Cam_Func",msg);

    if(Cam.size() <= 0)
    {
        msg = QString("sync/splice stop ......");
        Write_Log_File("Cam_Func",msg);

        sync_frame2.sync_stop();
        sp->sp_stop();
        sync_frame2.sync_output_clear();
        Live_show_Mat->clear();

        sp_output_state = false;
        sp_output_th->join();
        delete sp_output_th;
        sp_output_th = nullptr;

        msg = QString("sync/splice stop ...... end");
        Write_Log_File("Cam_Func",msg);


        if(Cam_vid.vid_ctx != nullptr)
        {
            Cam_vid.vid_ctx->release();
            delete Cam_vid.vid_ctx;
            Cam_vid.vid_ctx = nullptr;
        }

    }

}

void ao_cameram_fun::DeleteAllCam()
{
    QString msg;

    if(Cam.size() <= 0)
        return;

    msg = QString("Delete All Cam ......");
    Write_Log_File("Cam_Func",msg);

    for (auto cam_it = Cam.begin(); cam_it != Cam.end();)
    {
        DeleteCam(cam_it->id);
        cam_it = Cam.begin();
    }

    msg = QString("Delete All Cam ...... end");
    Write_Log_File("Cam_Func",msg);
}

void ao_cameram_fun::start_cam_num(uint32_t cam_id)
{
    QString msg;

    auto cam_it = Cam.find(Cam_staruct{cam_id, nullptr});

    msg = QString("Cam Start id:%1 ......").arg(QString::number(cam_id));
    Write_Log_File("Cam_Func",msg);


    if(cam_it == Cam.end())
    {
        msg = QString("Cam Start id:%1 Error.").arg(QString::number(cam_id));
        Write_Log_File("Cam_Func",msg);
        return;
    }

    if(cam_it->Cam_d->connect_state)
    {
        msg = QString("Cam Start id:%1 already started.").arg(QString::number(cam_id));
        Write_Log_File("Cam_Func",msg);
        return;
    }

    // init rb Clear
    sync_model::vstream_data_t* sync_tmp;
    cam_it->Cam_d->vid_data_buf->clear();  // cam rb clear
    for (;;) {
        // decode=>sync_rb to clear
        //Cam.at(i)->stream_rb->getTail(sync_tmp);
        cam_it->Cam_d->stream_rb->getTail_clear(sync_tmp);
        if (sync_tmp == nullptr)
            break;
        if (sync_tmp->data == nullptr)
            break;
        cam_it->Cam_d->sy_local_pool->ReturnObject(sync_tmp->data);
        sync_tmp->data = nullptr;
    }
    cam_it->Cam_d->stream_rb->clear();

    // log count init
    cam_it->Cam_d->cam_data_count = 0;
    cam_it->Cam_d->cam_data_lost_count = 0;
    cam_it->Cam_d->dc2_count = 0;
    cam_it->Cam_d->dc2_lost_count = 0;
    cam_it->Cam_d->DC_state = true;
    cam_it->Cam_d->dc2_start_time = clock_type::now();
    cam_it->Cam_d->dc2_end_time = cam_it->Cam_d->dc2_start_time;
    cam_it->Cam_d->dc2_cost_time = cam_it->Cam_d->dc2_end_time - cam_it->Cam_d->dc2_start_time;
    cam_it->Cam_d->dc2_start_time2 = clock_type::now();
    cam_it->Cam_d->dc2_end_time2 = cam_it->Cam_d->dc2_start_time2;
    cam_it->Cam_d->dc2_cost_time2 = cam_it->Cam_d->dc2_end_time2 - cam_it->Cam_d->dc2_start_time2;
    cam_it->Cam_d->cam_record_start_time = clock_type::now();
    cam_it->Cam_d->cam_record_end_time = cam_it->Cam_d->cam_record_start_time;
    cam_it->Cam_d->dc2_cost_time2 = cam_it->Cam_d->cam_record_end_time - cam_it->Cam_d->cam_record_start_time;

    //start
    cam_it->Cam_d->ao_cam->start();
    if(cam_it->Cam_d->dc_thr == nullptr)
        cam_it->Cam_d->dc_thr = new std::thread(&ao_cameram_fun::Cam_Decoder,this,&cam_it->Cam_d->DC_state,cam_it->Cam_d);

    //sync
    sync_model::video_stream_t  vid_str_t;

    *(cam_it->Cam_d->sy_stop) = false;
    vid_str_t.rb_stop = cam_it->Cam_d->sy_stop;
    vid_str_t.rb_lock = cam_it->Cam_d->sy_rb_lock;
    vid_str_t.rb_condi = cam_it->Cam_d->sy_rb_condi;
    vid_str_t.stream_rb = cam_it->Cam_d->stream_rb;
    vid_str_t.local_pool = cam_it->Cam_d->sy_local_pool;
    vid_str_t.object_pool = cam_it->Cam_d->object_pool;

    sync_frame2.append_video_stream(&vid_str_t);

    msg = QString("Cam Start id:%1 ...... end").arg(QString::number(cam_id));
    Write_Log_File("Cam_Func",msg);

    printf("start cam %u \n",cam_id);
}

void ao_cameram_fun::stop_cam_num(uint32_t cam_id)
{

    QString msg;

    auto cam_it = Cam.find(Cam_staruct{cam_id, nullptr});

    msg = QString("Cam Stop id:%1 ......").arg(QString::number(cam_id));
    Write_Log_File("Cam_Func",msg);

    if(cam_it == Cam.end())
    {
        msg = QString("Cam Stop id:%1 Error").arg(QString::number(cam_id));
        Write_Log_File("Cam_Func",msg);
        return;
    }


    cam_it->Cam_d->DC_state = false;
    cam_it->Cam_d->ao_cam->stop();

    *(cam_it->Cam_d->sy_stop) = true;
    cam_it->Cam_d->sy_rb_condi->notify_all();

    cam_it->Cam_d->vid_data_buf->data_lock_cv.notify_all();
    if(cam_it->Cam_d->dc_thr !=nullptr)
    {
        cam_it->Cam_d->dc_thr->join();
        delete  cam_it->Cam_d->dc_thr;
        cam_it->Cam_d->dc_thr = nullptr;
    }


    msg = QString("Cam Stop id:%1 ...... end").arg(QString::number(cam_id));
    Write_Log_File("Cam_Func",msg);

    printf("stop cam %u \n",cam_id);

}

void ao_cameram_fun::start_all()
{
    QString msg;

    if(Cam.size() <= 0)
        return;

    msg = QString("Cam All Start ......");
    Write_Log_File("Cam_Func",msg);

    for (auto cam_it = Cam.begin(); cam_it != Cam.end(); ++cam_it)
    {
        start_cam_num(cam_it->id);
    }

    msg = QString("Cam All Start ...... end");
    Write_Log_File("Cam_Func",msg);

    printf("-----------------start all----------------------\n");
}

void ao_cameram_fun::stop_all()
{
    QString msg;

    if(Cam.size() <= 0)
        return;

    msg = QString("Cam All Stop ......");
    Write_Log_File("Cam_Func",msg);

    for (auto cam_it = Cam.begin(); cam_it != Cam.end(); ++cam_it)
    {
        stop_cam_num(cam_it->id);
    }

    msg = QString("Cam All Stop ...... end");
    Write_Log_File("Cam_Func",msg);

    printf("-----------------stop all----------------------\n");
}

//------------------------------------------------------------------------------------
int ao_cameram_fun::open_rtsp(std::string rtsp_path)
{
    QString msg;

    msg = QString("Open RTSP : %1 ......").arg(QString::fromStdString(rtsp_path));
    Write_Log_File("Cam_Func",msg);

    if(out_rtsp.open)
    {
        msg = QString("Open RTSP : %1 already Opened").arg(QString::fromStdString(rtsp_path));
        Write_Log_File("Cam_Func",msg);
        return -1;
    }

    if(out_file.open == false && out_rtsp.open == false && out_rtmp.open ==false)
    {
        msg = QString("Encoder create ......");
        Write_Log_File("Encoder",msg);

        if( enc_class.create(ec_par) < 0)
        {
            msg = QString("Encoder create Error");
            Write_Log_File("Encoder",msg);
            return -1;
        }

        msg = QString("Encoder create ...... end");
        Write_Log_File("Encoder",msg);
    }

//    //log init
//    sp_out_in_count = 0;

//    sp_out_start_time =  clock_type::now();
//    sp_out_end_time = sp_out_start_time;
//    sp_out_cost_time = sp_out_end_time - sp_out_start_time;

//    sp_out_start_time2 =  clock_type::now();
//    sp_out_end_time2 = sp_out_start_time2;
//    sp_out_cost_time2 = sp_out_end_time2 - sp_out_start_time2;

//    conver_start_time = clock_type::now();
//    conver_end_time = conver_start_time;
//    conver_cost_time = conver_end_time - conver_start_time;

//    en_start_time = clock_type::now();
//    en_end_time = en_start_time;
//    en_cost_time = en_end_time - en_start_time;


    msg = QString("RTSP Create ......");
    Write_Log_File("Cam_Func",msg);

    if(out_rtsp.output.create(rtsp_path.c_str(),ec_par,enc_class.get_ctx()) <0)
    {
        msg = QString("RTSP Create Error");
        Write_Log_File("Cam_Func",msg);
        return -1;
    }

    msg = QString("RTSP Create ...... end");
    Write_Log_File("Cam_Func",msg);

    out_rtsp.open_first = true;
    out_rtsp.open =true;
    out_rtsp.output_path = rtsp_path;

    msg = QString("Open RTSP : %1 ...... end").arg(QString::fromStdString(rtsp_path));
    Write_Log_File("Cam_Func",msg);

    out_rtsp.Video_State.save = true;
    out_rtsp.Video_State.start_ntp_timestamp = 0;
    out_rtsp.Video_State.end_ntp_timestamp = 0;
    out_rtsp.Video_State.startFNo = 0;
    out_rtsp.Video_State.endFNo = 0;

    return 0;
}

int ao_cameram_fun::close_rtsp()
{  
    QString msg;

    msg = QString("Close RTSP : %1 ......").arg(QString::fromStdString(out_rtsp.output_path));
    Write_Log_File("Cam_Func",msg);

    if(!out_rtsp.open)
    {
        msg = QString("Close RTSP : %1 already Closed").arg(QString::fromStdString(out_rtsp.output_path));
        Write_Log_File("Cam_Func",msg);
        return -1;
    }

    msg = QString("Close RTSP : release %1 ......").arg(QString::fromStdString(out_rtsp.output_path));
    Write_Log_File("Cam_Func",msg);

    if(out_rtsp.output.release()<0)
    {
        msg = QString("Close RTSP : %1 release Error").arg(QString::fromStdString(out_rtsp.output_path));
        Write_Log_File("Cam_Func",msg);
        return -1;
    }

    msg = QString("Close RTSP : release %1 ...... end").arg(QString::fromStdString(out_rtsp.output_path));
    Write_Log_File("Cam_Func",msg);

    out_rtsp.open = false;

    if(out_file.open == false && out_rtsp.open == false && out_rtmp.open == false)
    {
        msg = QString("Encoder release ......");
        Write_Log_File("Encoder",msg);

        if( enc_class.release()< 0)
        {
            msg = QString("Encoder release Error");
            Write_Log_File("Encoder",msg);
            return -1;
        }

        msg = QString("Encoder release ...... end");
        Write_Log_File("Encoder",msg);
    }

    msg = QString("Close RTSP : %1 ...... end").arg(QString::fromStdString(out_rtsp.output_path));
    Write_Log_File("Cam_Func",msg);

    out_rtsp.Video_State.save = false;
//    out_rtsp.Video_State.start_ntp_timestamp = 0;
//    out_rtsp.Video_State.end_ntp_timestamp = 0;
//    out_rtsp.Video_State.startFNo = 0;
//    out_rtsp.Video_State.endFNo = 0;

    return 0;
}

int ao_cameram_fun::open_file(std::string file_path)
{
    QString msg;

    msg = QString("Open File : %1 ......").arg(QString::fromStdString(file_path));
    Write_Log_File("Cam_Func",msg);

    if(out_file.open)
    {
        msg = QString("Open File : %1 already Opened").arg(QString::fromStdString(file_path));
        Write_Log_File("Cam_Func",msg);
        return -1;
    }

    if(out_file.open == false && out_rtsp.open == false && out_rtmp.open ==false)
    {
        msg = QString("Encoder create ......");
        Write_Log_File("Encoder",msg);

        if( enc_class.create(ec_par) < 0)
        {
            qDebug()<<"ec_par:"<<ec_par.en_format;

            msg = QString("Encoder create Error");
            Write_Log_File("Encoder",msg);
            return -1;
        }

        msg = QString("Encoder create ...... end");
        Write_Log_File("Encoder",msg);
    }

    msg = QString("File Create : %1 ......").arg(QString::fromStdString(file_path));
    Write_Log_File("Cam_Func",msg);

    if(out_file.output.create(file_path.c_str() ,ec_par,enc_class.get_ctx())<0)
    {
        msg = QString("File Create : %1 Error").arg(QString::fromStdString(file_path));
        Write_Log_File("Cam_Func",msg);
        return -1;
    }

    msg = QString("File Create : %1 ...... end").arg(QString::fromStdString(file_path));
    Write_Log_File("Cam_Func",msg);

    out_file.open_first = true;
    out_file.open = true;
    out_file.output_path = file_path;

    msg = QString("Open File : %1 ...... end").arg(QString::fromStdString(file_path));
    Write_Log_File("Cam_Func",msg);

    out_file.Video_State.save = true;
    out_file.Video_State.start_ntp_timestamp = 0;
    out_file.Video_State.end_ntp_timestamp = 0;
    out_file.Video_State.startFNo = 0;
    out_file.Video_State.endFNo = 0;

    return 0;
}

int ao_cameram_fun::close_file()
{
    QString msg;

    msg = QString("Close File : %1 ......").arg(QString::fromStdString(out_file.output_path));
    Write_Log_File("Cam_Func",msg);

    if(!out_file.open)
    {
        msg = QString("Close File : %1 already Closed").arg(QString::fromStdString(out_file.output_path));
        Write_Log_File("Cam_Func",msg);
        return -1;
    }

    msg = QString("Close File : release %1 ......").arg(QString::fromStdString(out_file.output_path));
    Write_Log_File("Cam_Func",msg);

    if(out_file.output.release()<0)
    {
        msg = QString("Close File : %1 release Error").arg(QString::fromStdString(out_file.output_path));
        Write_Log_File("Cam_Func",msg);
        return -1;
    }

    msg = QString("Close File : release %1...... end").arg(QString::fromStdString(out_file.output_path));
    Write_Log_File("Cam_Func",msg);

    out_file.open = false;

    if(out_file.open == false && out_rtsp.open == false && out_rtmp.open == false)
    {
        msg = QString("Encoder release ......");
        Write_Log_File("Encoder",msg);

        if( enc_class.release()< 0)
        {
            msg = QString("Encoder release Error");
            Write_Log_File("Encoder",msg);
            return -1;
        }

        msg = QString("Encoder release ...... end");
        Write_Log_File("Encoder",msg);
    }

    msg = QString("Close File : %1 ...... end").arg(QString::fromStdString(out_file.output_path));
    Write_Log_File("Cam_Func",msg);

    out_file.Video_State.save = false;
//    out_file.Video_State.start_ntp_timestamp = 0;
//    out_file.Video_State.end_ntp_timestamp = 0;
//    out_file.Video_State.startFNo = 0;
//    out_file.Video_State.endFNo = 0;

    return 0;
}

int ao_cameram_fun::open_rtmp(std::string rtmp_path)
{
    QString msg;

    msg = QString("Open RTMP : %1 ......").arg(QString::fromStdString(rtmp_path));
    Write_Log_File("Cam_Func",msg);

    if(out_rtmp.open)
    {
        msg = QString("Open RTMP : %1 already Opened").arg(QString::fromStdString(rtmp_path));
        Write_Log_File("Cam_Func",msg);
        return -1;
    }

    if(out_file.open == false && out_rtsp.open == false && out_rtmp.open ==false)
    {
        msg = QString("Encoder create ......");
        Write_Log_File("Encoder",msg);

        if( enc_class.create(ec_par) < 0)
        {
            msg = QString("Encoder create Error");
            Write_Log_File("Encoder",msg);
            return -1;
        }

        msg = QString("Encoder create ...... end");
        Write_Log_File("Encoder",msg);
    }

    msg = QString("RTMP Create ......");
    Write_Log_File("Cam_Func",msg);

    if(out_rtmp.output.create(rtmp_path.c_str(),ec_par,enc_class.get_ctx()) <0)
    {
        msg = QString("RTMP Create Error");
        Write_Log_File("Cam_Func",msg);
        return -1;
    }

    msg = QString("RTMP Create ...... end");
    Write_Log_File("Cam_Func",msg);

    out_rtmp.open_first = true;
    out_rtmp.open =true;
    out_rtmp.output_path = rtmp_path;

    msg = QString("Open RTMP : %1 ...... end").arg(QString::fromStdString(rtmp_path));
    Write_Log_File("Cam_Func",msg);

    out_rtmp.Video_State.save = true;
    out_rtmp.Video_State.start_ntp_timestamp = 0;
    out_rtmp.Video_State.end_ntp_timestamp = 0;
    out_rtmp.Video_State.startFNo = 0;
    out_rtmp.Video_State.endFNo = 0;

    return 0;
}

int ao_cameram_fun::close_rtmp()
{
    QString msg;

    msg = QString("Close RTMP : %1 ......").arg(QString::fromStdString(out_rtmp.output_path));
    Write_Log_File("Cam_Func",msg);

    if(!out_rtmp.open)
    {
        msg = QString("Close RTMP : %1 already Closed").arg(QString::fromStdString(out_rtmp.output_path));
        Write_Log_File("Cam_Func",msg);
        return -1;
    }

    msg = QString("Close RTMP : release %1 ......").arg(QString::fromStdString(out_rtmp.output_path));
    Write_Log_File("Cam_Func",msg);

    if(out_rtmp.output.release()<0)
    {
        msg = QString("Close RTMP : %1 release Error").arg(QString::fromStdString(out_rtmp.output_path));
        Write_Log_File("Cam_Func",msg);
        return -1;
    }

    msg = QString("Close RTMP : release %1 ...... end").arg(QString::fromStdString(out_rtmp.output_path));
    Write_Log_File("Cam_Func",msg);

    out_rtmp.open = false;

    if(out_file.open == false && out_rtsp.open == false && out_rtmp.open == false)
    {
        msg = QString("Encoder release ......");
        Write_Log_File("Encoder",msg);

        if( enc_class.release()< 0)
        {
            msg = QString("Encoder release Error");
            Write_Log_File("Encoder",msg);
            return -1;
        }

        msg = QString("Encoder release ...... end");
        Write_Log_File("Encoder",msg);
    }

    msg = QString("Close RTMP : %1 ...... end").arg(QString::fromStdString(out_rtmp.output_path));
    Write_Log_File("Cam_Func",msg);

    out_rtmp.Video_State.save = false;
//    out_rtmp.Video_State.start_ntp_timestamp = 0;
//    out_rtmp.Video_State.end_ntp_timestamp = 0;
//    out_rtmp.Video_State.startFNo = 0;
//    out_rtmp.Video_State.endFNo = 0;

    return 0;
}

//------------------------------------------------------------------------------------
bool ao_cameram_fun::set_Encoder_param(const  Encoder_param par)
{ 
    QString msg;

    msg = QString("Set Encoder param ......");
    Write_Log_File("Cam_Func",msg);

    if(out_file.open == true || out_rtsp.open == true || out_rtmp.open == true)
    {
        msg = QString("Set Encoder param Error");
        Write_Log_File("Cam_Func",msg);
        return false;
    }

   this->ec_par = par;

   if(out_nv12_uint8 !=nullptr)
   {
       delete out_nv12_uint8;
       out_nv12_uint8 = nullptr;
   }

   if(GPU_nv12_uint8 != nullptr)
   {
       MyConver::Free_GPU_uint8(GPU_nv12_uint8);
       GPU_nv12_uint8 = nullptr;
   }

   if(avTemp != nullptr)
   {
       av_frame_free(&avTemp);
       avTemp = nullptr;
   }

   //--------------------------

   out_nv12_uint8 = new uint8_t[ec_par.width * ec_par.height * 3 / 2];
   GPU_nv12_uint8 = MyConver::Create_GPU_uint8(ec_par.width * ec_par.height * 3 / 2);
   y_size = ec_par.width * ec_par.height;

   avTemp= av_frame_alloc();
   avTemp->format = ec_par.pix_format;
   avTemp->width = ec_par.width;
   avTemp->height = ec_par.height;
   av_frame_get_buffer(avTemp, 0);

   msg = QString("Set Encoder param ...... end");
   Write_Log_File("Cam_Func",msg);

   return true;

}

bool ao_cameram_fun::set_Live_num(uint32_t cam_id)
{
    QString msg;

    msg = QString("Set Live Num %1 ......").arg(QString::number(cam_id));
    Write_Log_File("Cam_Func",msg);

    if(mode != Show_Mode::Show_Single)
    {
        msg = QString("Set Live Num %1 Mode Error").arg(QString::number(cam_id));
        Write_Log_File("Cam_Func",msg);
        return false;
    }

    //Live_show stop
    for (auto it=Cam.begin(); it!=Cam.end(); ++it)
    {
        it->Cam_d->dc_Live_show = false;
    }

    //Live_show start
    auto cam_it = Cam.find(Cam_staruct{cam_id, nullptr});

    if(cam_it == Cam.end())
    {
        msg = QString("Set Live Num %1 Error").arg(QString::number(cam_id));
        Write_Log_File("Cam_Func",msg);
        return false;
    }

    cam_it->Cam_d->dc_Live_show = true;
    Live_num = cam_id;

    msg = QString("Set Live Num %1 ...... end").arg(QString::number(cam_id));
    Write_Log_File("Cam_Func",msg);

    return true;
}

void ao_cameram_fun::set_Show_Mode(const Show_Mode mode)
{
    QString msg;

    msg = QString("Set Show Mode %1 ......").arg(mode);
    Write_Log_File("Cam_Func",msg);

    this->mode = mode;

    sp->set_Live_View_buf(nullptr);
    for (auto it=Cam.begin(); it!=Cam.end(); ++it)
    {
        it->Cam_d->dc_Live_show = false;
    }

    auto cam_it = Cam.find(Cam_staruct{Live_num, nullptr});

    switch (mode)
    {
        case Show_Mode::Show_Single:
            if(!(set_Live_num(Live_num)))
            {
                Cam.begin()->Cam_d->dc_Live_show = true;
            }
            break;

        case Show_Mode::Show_splice :
            sp->set_Live_View_buf(Live_show_Mat);
            break;
    }

    msg = QString("Set Show Mode %1 ...... end").arg(mode);
    Write_Log_File("Cam_Func",msg);
}

void ao_cameram_fun::set_view_sp_num(std::vector<uint32_t> v_num)
{
    QString msg;

    msg = QString("Set View Splice Num ......");
    Write_Log_File("Cam_Func",msg);

    if(sp ==nullptr)
    {
        msg = QString("Splice Not Created");
        Write_Log_File("Cam_Func",msg);
        return;
    }

    sp_view_num = v_num;
    sp->set_view_num(sp_view_num);

    msg = QString("Set View Splice Num ...... end");
    Write_Log_File("Cam_Func",msg);
}

void ao_cameram_fun::set_view_sp_rot(std::vector<SP_Rot> v_rot)
{
    QString msg;

    msg = QString("Set View Splice Rot ......");
    Write_Log_File("Cam_Func",msg);

    if(sp ==nullptr)
    {
        msg = QString("Splice Not Created");
        Write_Log_File("Cam_Func",msg);
        return;
    }

    sp_view_rot = v_rot;
    sp->set_view_rot(sp_view_rot);

    msg = QString("Set View Splice Rot ...... end");
    Write_Log_File("Cam_Func",msg);
}

void ao_cameram_fun::set_sp_Mode(const SP_MODE mode)
{
    QString msg;

    msg = QString("Set Splice Mode %1 ......").arg(mode);
    Write_Log_File("Cam_Func",msg);

    if(sp == nullptr)
    {
        msg = QString("Splice Not Created");
        Write_Log_File("Cam_Func",msg);
        return;
    }

    sp->set_Mode(mode);

    msg = QString("Set Splice Mode %1 ...... end").arg(mode);
    Write_Log_File("Cam_Func",msg);
}

bool ao_cameram_fun::set_CamVideo_param(const multiple_vid_ctx_param par)
{
    Cam_vid.Cam_Video_param = par;
    return true;
}

void ao_cameram_fun::set_Camera_videoSave(bool enable)
{
    if(enable)
    {
        Cam_vid.Video_State.start_ntp_timestamp = 0;
        Cam_vid.Video_State.end_ntp_timestamp = 0;
    }

    Cam_vid.Video_State.save = enable;
}

void ao_cameram_fun::set_Camera_videoPath(string path)
{
    this->Cam_vid.Video_Path = path;
}

//------------------------------------------------------------------------------------
int ao_cameram_fun::get_Cam_size()
{
    return this->Cam.size();
}

QImage ao_cameram_fun::get_LiveView()
{
    QImage tmp;
    GpuMat_Struct *gpumat_temp;
    cv::Mat cpuMat;
    cv::cuda::Stream stream;

    view_start_time = clock_type::now();
    Live_show_Mat->getTail(gpumat_temp);

    if(gpumat_temp == nullptr)
        return tmp;

    cv::cuda::resize(gpumat_temp->gpu_mat,gpumat_temp->gpu_mat,cv::Size(1920,1080),0, 0, cv::INTER_LINEAR, stream);

    gpumat_temp->gpu_mat.download(cpuMat);

    QImage qImage(cpuMat.data, cpuMat.cols, cpuMat.rows, static_cast<int>(cpuMat.step), QImage::Format_BGR888);

    tmp = qImage.copy();
    view_end_time = clock_type::now();
    view_cost_time = view_end_time - view_start_time;

    return tmp;
}

void ao_cameram_fun::get_LiveView(QImage *out_img)
{
    QImage tmp;
    GpuMat_Struct *gpumat_temp;
    cv::cuda::Stream stream;

    view_start_time = clock_type::now();
    Live_show_Mat->getTail(gpumat_temp);

    if(gpumat_temp == nullptr)
    {
        *out_img = tmp;
        return;
    }

    cv::cuda::resize(gpumat_temp->gpu_mat,gpumat_temp->gpu_mat,cv::Size(1280,720),0, 0, cv::INTER_LINEAR, stream);

    gpumat_temp->gpu_mat.download(Live_View_cpuMat);

    view_count = gpumat_temp->frame_num;
    *out_img = QImage(Live_View_cpuMat.data, Live_View_cpuMat.cols, Live_View_cpuMat.rows, static_cast<int>(Live_View_cpuMat.step), QImage::Format_BGR888);

    view_end_time = clock_type::now();
    view_cost_time = view_end_time - view_start_time;

}

//------------------------------------------------------------------------------------
void ao_cameram_fun::log()
{
    printf("=========================================== \n");
    for (auto cam_it = Cam.begin(); cam_it != Cam.end(); ++cam_it)
    {
        printf("[Cam %s] cam_frame [%u] cam_lost [%u] recode [%lf] dc_frame [%u] dc_lost [%u] dc_cost1 [%lf] dc_cost2 [%lf]\n",
               cam_it->Cam_d->ao_cam->Cam_IP,
               cam_it->Cam_d->cam_data_count,
               cam_it->Cam_d->cam_data_lost_count,
               cam_it->Cam_d->cam_record_cost_time.count(),
               cam_it->Cam_d->dc2_count,
               cam_it->Cam_d->dc2_lost_count,
               cam_it->Cam_d->dc2_cost_time.count(),
               cam_it->Cam_d->dc2_cost_time2.count());
    }

    printf("------------------------------------------- \n");

    printf("sy_count [%u] sy_fail [%u] sy_lost [%u]\n",
           sync_frame2.sync_count,sync_frame2.sync_fail_count,sync_frame2.sync_lost_count);

    printf("sy_cost [%lf] sy_gatdata_cost [%lf] sy_sync_cost [%lf] sy_pushdata_cost [%lf]\n",
           sync_frame2.cost_time.count(),sync_frame2.getdata_cost_time.count(),
           sync_frame2.sync_cost_time.count(),sync_frame2.push_data_cost_time.count());

    printf("------------------------------------------- \n");

    sp->log();

    printf("------------------------------------------- \n");

    printf("sp_out_in_count [%u] sp_out_cost_time [%lf] sp_out_cost_time2 [%lf] conver_cost [%lf] en_cost [%lf]\n",
           sp_out_in_count,sp_out_cost_time.count(),sp_out_cost_time2.count(),
           conver_cost_time.count(),en_cost_time.count());

    printf("------------------------------------------- \n");

    printf("view_cost_time [%lf] view_count [%u]\n",view_cost_time.count(),view_count);

    printf("=========================================== \n");

}

//------------------------------------------------------------------------------------
//private
void ao_cameram_fun::Cam_Decoder(bool *state,std::shared_ptr<Cam_data> cam_d)
{
    ao::lm_video_data *temp;
    GpuMat_Struct *live_show;
    cv::cuda::GpuMat *gpu_out;
    AVFrame *gpu_frame;
    AVPacket *avpkt;
    avpkt = av_packet_alloc();
    av_init_packet(avpkt);

    gpu_frame =  av_frame_alloc();

    cam_d->dc2_start_time = clock_type::now();

    sync_model::vstream_data_t *vd;

    while (*state)
    {
        cam_d->vid_data_buf->getTail(temp);
        if(temp == nullptr)
        {
            std::unique_lock<std::mutex> lock(cam_d->vid_data_buf->data_lock);
            cam_d->vid_data_buf->data_lock_cv.wait(lock);
            continue;
        }

        cam_d->dc2_start_time2 = clock_type::now();

        cam_d->dc2_count = temp->frm_no;

        avpkt->data = temp->data;
        avpkt->size = temp->len;

        if(cam_d->dc->send_decode_pkt(avpkt)>=0)
        {
            if(cam_d->dc->get_decode_frame_GPU(gpu_frame)>=0)
            {
                cam_d->dc2_count = temp->frm_no;

                gpu_out = cam_d->dc_local_pool->GetObject();
                if (gpu_out == nullptr) {
                    printf("Get Objcet Fail \n");
                    cam_d->dc2_lost_count ++;
                    continue;
                }

                MyConver::Avframe_NV12_to_Mat_BGR_2(gpu_frame,*gpu_out);

                if(cam_d->dc_Live_show)
                {
                    Live_show_Mat->getHead(live_show);

                    if(live_show!=nullptr)
                    {
                        gpu_out->copyTo(live_show->gpu_mat);
                        live_show->frame_num = temp->frm_no;
                        live_show->ntp = temp->ntp_timestamp;
                        Live_show_Mat->data_lock_cv.notify_all();
                    }
                }

                cam_d->stream_rb->getHead(vd);

                if (vd == nullptr)
                {
                    cam_d->sy_rb_condi->notify_all();
                    //lost_cnt++;
                    cam_d->dc2_lost_count ++;
                    cam_d->dc_local_pool->ReturnObject(gpu_out);
                    continue;
                }
                if (vd->data != nullptr) {
                    printf("dec output rb data %p \n", vd->data);
                }

                //printf("[%s] dc \n ",cam_d->ao_cam->Cam_IP);

                vd->id = cam_d->CamNO;
                vd->ntp = temp->ntp_timestamp;
                vd->frm_no = temp->frm_no;
                vd->data = gpu_out;
                cam_d->sy_rb_condi->notify_all();


                //-------------
                cam_d->dc2_end_time = clock_type::now();
                cam_d->dc2_end_time2 = clock_type::now();

                cam_d->dc2_cost_time = cam_d->dc2_end_time - cam_d->dc2_start_time;
                cam_d->dc2_cost_time2 = cam_d->dc2_end_time2 - cam_d->dc2_start_time2;

                cam_d->dc2_start_time = clock_type::now();
            }
        }
    }

    *(cam_d->sy_stop) = true;
    cam_d->sy_rb_condi->notify_all();


    av_frame_free(&gpu_frame);
    av_packet_unref(avpkt);
    av_packet_free(&avpkt);
    printf("[%u]Cam_Decoder  end \n" , cam_d->CamNO);


}

void ao_cameram_fun::sp_output(bool *state)
{
    GpuMat_Struct *in_mat;

    if(out_nv12_uint8 ==nullptr)
        out_nv12_uint8 = new uint8_t[ec_par.width * ec_par.height * 3 / 2];

    if(GPU_nv12_uint8 == nullptr)
        GPU_nv12_uint8 = MyConver::Create_GPU_uint8(ec_par.width * ec_par.height * 3 / 2);

    if(y_size == 0 )
        y_size = ec_par.width * ec_par.height;

    if(avTemp == nullptr)
    {
        avTemp= av_frame_alloc();
        avTemp->format = ec_par.pix_format;
        avTemp->width = ec_par.width;
        avTemp->height = ec_par.height;
        av_frame_get_buffer(avTemp, 0);
    }

    AVPacket *en_avpkt;
    en_avpkt = av_packet_alloc();
    av_init_packet(en_avpkt);

    AVPacket packetCopy_rtsp;
    av_init_packet(&packetCopy_rtsp);
    AVPacket packetCopy_rtmp;
    av_init_packet(&packetCopy_rtmp);
    _time start_t;
    _time end_t;
    milli_type cost_time;

    double fps_t;

    sp_out_start_time = clock_type::now();

    while(*state)
    {
        if(!sp->get_sp_data(in_mat))
            break;

        if(in_mat ==nullptr)
            continue;

//        uint32_t tmp_frame_num;
        sp_out_in_count = in_mat->frame_num;
//        if(sp_out_in_count - tmp_frame_num != 1)
//        {
//            printf("[Bug] spFrameNo:%d:%d \n",sp_out_in_count,tmp_frame_num);
//        }
        sp_out_start_time2 = clock_type::now();

        if(out_file.open == true || out_rtsp.open == true || out_rtmp.open ==true)
        {
            start_t = clock_type::now();

            conver_start_time = clock_type::now();
            MyConver::Mat_BGR_to_uint8_NV12_3(&in_mat->gpu_mat,GPU_nv12_uint8,out_nv12_uint8);
            conver_end_time = clock_type::now();
            conver_cost_time = conver_end_time - conver_start_time;

            avTemp->data[0] = out_nv12_uint8 ;
            avTemp->data[1] = out_nv12_uint8+y_size;


            en_start_time = clock_type::now();
            if(enc_class.send_encode_frame(avTemp) < 0 )
                continue;

            if(enc_class.get_encode_pkt(en_avpkt) < 0 )
                continue;

            en_end_time = clock_type::now();
            en_cost_time = en_end_time - en_start_time;

            av_packet_ref(&packetCopy_rtsp, en_avpkt);
            av_packet_ref(&packetCopy_rtmp, en_avpkt);

            end_t = clock_type::now();

            cost_time = end_t - start_t ;

            if(out_rtsp.open == true && (out_rtsp.open_first == false || packetCopy_rtsp.flags == 1))
            {
                if(out_rtsp.open_first)
                    out_rtsp.Video_State.start_ntp_timestamp = in_mat->ntp;
                out_rtsp.output.write_frame(&packetCopy_rtsp);
                out_rtsp.open_first = false;
            }

            if(out_rtmp.open == true && (out_rtmp.open_first == false || packetCopy_rtmp.flags == 1))
            {
                if(out_rtmp.open_first)
                    out_rtmp.Video_State.start_ntp_timestamp = in_mat->ntp;
                out_rtmp.output.write_frame(&packetCopy_rtmp);
                out_rtmp.open_first = false;
            }
            if(out_file.open == true && (out_file.open_first == false || en_avpkt->flags == 1))
            {
                if(out_file.open_first)
                    out_file.Video_State.start_ntp_timestamp = in_mat->ntp;
                out_file.output.write_frame(en_avpkt);
                out_file.open_first = false;
//                tmp_frame_num = in_mat->frame_num;
//                printf("spFrameNo:%d \n",in_mat->frame_num);
            }
            av_packet_unref(en_avpkt);
        }

        if(!out_rtsp.open_first && !out_rtsp.open)
            out_rtsp.Video_State.end_ntp_timestamp = in_mat->ntp;

        if(!out_rtmp.open_first && !out_rtmp.open)
            out_rtmp.Video_State.end_ntp_timestamp = in_mat->ntp;

        if(!out_file.open_first && !out_file.open)
            out_file.Video_State.end_ntp_timestamp = in_mat->ntp;

        sp_out_end_time = clock_type::now();
        sp_out_end_time2 = clock_type::now();

        sp_out_cost_time = sp_out_end_time - sp_out_start_time;
        sp_out_cost_time2 = sp_out_end_time2 - sp_out_start_time2;

        sp_out_start_time = clock_type::now();
    }

    if(GPU_nv12_uint8 != nullptr)
    {
        MyConver::Free_GPU_uint8(GPU_nv12_uint8);
        GPU_nv12_uint8 = nullptr;
        printf("Free_GPU_uint8 \n");
    }

    delete out_nv12_uint8;
    out_nv12_uint8 = nullptr;

    av_frame_free(&avTemp);
    avTemp = nullptr;
    av_packet_free(&en_avpkt);
    av_packet_unref(&packetCopy_rtsp);
    av_packet_unref(&packetCopy_rtmp);

    y_size = 0;

    printf("sp_output  end \n");
}

void ao_cameram_fun::Write_Log_File(QString type,QString msg)
{
  QString fileName = "Cam_Kernel.log";
  QDateTime currentDateTime = QDateTime::currentDateTime();
  QString formattedDateTime = currentDateTime.toString("[yyyy-MM-dd hh:mm:ss]");
  QString massage = "";


  QFile file(fileName);
  if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
      QTextStream stream(&file);

      massage = QString("%1 [%2] %3").arg(formattedDateTime).arg(type).arg(msg);

      stream << massage << endl;

      file.close();

  } else {
      printf("Failed to open file");
  }
}


