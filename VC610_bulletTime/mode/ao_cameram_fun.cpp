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

ao_cameram_fun::ao_cameram_fun()
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


//------------------------------------------------------------------------------------
static void on_265(std::shared_ptr<Cam_data> Cam,struct ao::lm_video_data *h265,Cam_CallBack_Fun *cam_rb)
{
    //printf("video [%s] [%d] [%d] [%d] [%llu]\n",Cam->ao_cam->Cam_IP,h265->frm_no,h265->type,h265->len,h265->ntp_timestamp);

    //----------------------------------------
    //FHD I 500KB  P  250KB   200/100
    //4K  I   2MB  P    1MB
    //----------------------------------------

    if(cam_rb->cam_vid_data !=nullptr)
    {
        cam_rb->cam_vid_data(Cam,h265);
    }

    ao::lm_video_data *temp;

    Cam->cam_data_count = h265->frm_no;

    if(Cam->vid_data_containers_full == true && h265->type !=1)
    {
        Cam->cam_data_lost_count ++;
        return;
    }

    Cam->vid_data_buf->getHead(temp);

    if(temp == nullptr)
    {
        Cam->cam_data_lost_count ++;
        Cam->vid_data_containers_full = true;
        return;
    }

    Cam->vid_data_containers_full = false;
    temp->len = h265->len;
    temp->type = h265->type;
    temp->frm_no = h265->frm_no;
    temp->ntp_timestamp = h265->ntp_timestamp;
    memcpy(temp->data, h265->data, h265->len);
    Cam->vid_data_buf->HeadNext();
    Cam->vid_data_buf->data_lock_cv.notify_all();
}


static void on_audio_data(std::shared_ptr<Cam_data> Cam,struct ao::lm_audio_data *audio,Cam_CallBack_Fun *cam_rb)
{
    //printf("audio[%s] [%d] [%d] [%llu]\n",Cam->ao_cam->Cam_IP,audio->frm_no,audio->len,audio->ntp_timestamp);

    if(cam_rb->cam_aud_data !=nullptr)
    {
        cam_rb->cam_aud_data(Cam,audio);
    }
}

static void on_Connect(std::shared_ptr<Cam_data> Cam,Cam_CallBack_Fun *cam_rb)
{
    Cam->connect_state = true;
    printf("[%s] OnConnect\n",Cam->ao_cam->Cam_IP);

    if(cam_rb->cam_connect !=nullptr)
    {
        cam_rb->cam_connect(Cam);
    }
}

static void on_Disconnect(std::shared_ptr<Cam_data> Cam,Cam_CallBack_Fun *cam_rb)
{
    Cam->DC_state = false;
    Cam->vid_data_buf->data_lock_cv.notify_all();

    Cam->connect_state = false;
    printf("[%s] OnDisconnect\n",Cam->ao_cam->Cam_IP);

    if(cam_rb->cam_disconnect !=nullptr)
    {
        cam_rb->cam_disconnect(Cam);
    }
}

static void on_Exception(std::shared_ptr<Cam_data> Cam,Cam_CallBack_Fun *cam_rb)
{
    Cam->DC_state = false;
    Cam->vid_data_buf->data_lock_cv.notify_all();

    Cam->connect_state = false;
    printf("[%s] OnException\n",Cam->ao_cam->Cam_IP);

    if(cam_rb->cam_exception !=nullptr)
    {
        cam_rb->cam_exception(Cam);
    }
}

static void on_VideoMeta(std::shared_ptr<Cam_data> Cam,struct ao::lm_video_meta *vid_data,Cam_CallBack_Fun *cam_rb)
{
    printf("cam [%s] width [%u] height [%u] fps [%u] gop [%u] \n"
           ,Cam->ao_cam->Cam_IP,vid_data->width,vid_data->height,vid_data->fps,vid_data->gop);

    if(cam_rb->cam_VideoMeta !=nullptr)
    {
        cam_rb->cam_VideoMeta(Cam,vid_data);
    }
}

static void on_AudioMeta(std::shared_ptr<Cam_data> Cam,struct ao::lm_audio_meta *au_data,Cam_CallBack_Fun *cam_rb)
{
    printf("cam [%s] sample_rate [%u] sample_size [%u] channel [%u] bitrate [%u] encoder [%u]\n"
           ,Cam->ao_cam->Cam_IP,au_data->sample_rate,au_data->sample_size,au_data->channel
           ,au_data->bitrate,au_data->encoder);

    if(cam_rb->cam_AudioMeta !=nullptr)
    {
        cam_rb->cam_AudioMeta(Cam,au_data);
    }
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


//------------------------------------------------------------------------------------
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

void ao_cameram_fun::init(const Decoder_class_param dc_par)
{
    Write_Log_File("Cam_Func","Cam_fun init ......");

    this->dc_par = dc_par;

    Live_show_Mat = new RingBuffer<GpuMat_Struct>(5);
    Live_show_Mat->init();

    //------log init---------

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


    Cam_staruct cam;
    cam.id = id;
    cam.Cam_d = std::make_shared<Cam_data>();

    cam.Cam_d->vid_data_buf = new RingBuffer<ao::lm_video_data>(10,8*1024*1024,vid_data_init,vid_data_del);
    cam.Cam_d->vid_data_buf->init();

    cam.Cam_d->ao_cam = new ao_readcam(ip);
    cam.Cam_d->ao_cam->setOnH265DataCallback(std::bind(on_265,cam.Cam_d,std::placeholders::_1,&cam_cb_fun));
    cam.Cam_d->ao_cam->setOnAudioDataCallback(std::bind(on_audio_data,cam.Cam_d,std::placeholders::_1,&cam_cb_fun));
    cam.Cam_d->ao_cam->setOnConnectCallback(std::bind(on_Connect,cam.Cam_d,&cam_cb_fun));
    cam.Cam_d->ao_cam->setOnDisconnectedCallback(std::bind(on_Disconnect,cam.Cam_d,&cam_cb_fun));
    cam.Cam_d->ao_cam->setOnExceptionCallback(std::bind(on_Exception,cam.Cam_d,&cam_cb_fun));

    cam.Cam_d->ao_cam->setOnVideoMetaCallback(std::bind(on_VideoMeta,cam.Cam_d,std::placeholders::_1,&cam_cb_fun));
    cam.Cam_d->ao_cam->setOnAudioMetaCallback(std::bind(on_AudioMeta,cam.Cam_d,std::placeholders::_1,&cam_cb_fun));
//--------------------------------------------------------------------------------------
    cam.Cam_d->ao_cam->init();

    cam.Cam_d->dc = new Decoder_class();
    cam.Cam_d->dc->create(dc_par);


    cam.Cam_d->CamNO = id;

    cam.Cam_d->dc_Live_show = false;
    cam.Cam_d->connect_state = false;
    cam.Cam_d->DC_state = false;


    //-----------
    cam.Cam_d->cam_data_count = 0;
    cam.Cam_d->cam_data_lost_count = 0;


    if(!Cam.insert(cam).second) {
        id = 0;
        delete cam.Cam_d->ao_cam;
        cam.Cam_d->ao_cam = nullptr;
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
        msg = QString("Live_show_Mat clear  ......");
        Write_Log_File("Cam_Func",msg);

        Live_show_Mat->clear();

        msg = QString("Live_show_Mat clear  ...... end");
        Write_Log_File("Cam_Func",msg);

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
    cam_it->Cam_d->vid_data_buf->clear();  // cam rb clear

    // log count init
    cam_it->Cam_d->cam_data_count = 0;
    cam_it->Cam_d->cam_data_lost_count = 0;
    cam_it->Cam_d->DC_state = true;


    //start
    cam_it->Cam_d->ao_cam->start();
    if(cam_it->Cam_d->dc_thr == nullptr)
        cam_it->Cam_d->dc_thr = new std::thread(&ao_cameram_fun::Cam_Decoder,this,&cam_it->Cam_d->DC_state,cam_it->Cam_d);


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
bool ao_cameram_fun::set_Live_num(uint32_t cam_id)
{
    QString msg;

    msg = QString("Set Live Num %1 ......").arg(QString::number(cam_id));
    Write_Log_File("Cam_Func",msg);


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


void ao_cameram_fun::set_cam_cb_VidData(OnCamVidData rb)
{
    cam_cb_fun.cam_vid_data = rb;
}

void ao_cameram_fun::set_cam_cb_AudData(OnCamAudData rb)
{
    cam_cb_fun.cam_aud_data = rb;
}

void ao_cameram_fun::set_cam_cb_Connect(OnCamConnect rb)
{
    cam_cb_fun.cam_connect= rb;
}

void ao_cameram_fun::set_cam_cb_Disconnect(OnCamDisconnect rb)
{
    cam_cb_fun.cam_disconnect =rb;
}

void ao_cameram_fun::set_cam_cb_Exception(OnCamException rb)
{
    cam_cb_fun.cam_exception =rb;
}

void ao_cameram_fun::set_cam_cb_VideoMeta(OnCamVideoMeta rb)
{
    cam_cb_fun.cam_VideoMeta =rb;
}

void ao_cameram_fun::set_cam_cb_AudioMeta(OnCamAudioMeta rb)
{
    cam_cb_fun.cam_AudioMeta =rb;
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

    cv::cuda::resize(gpumat_temp->gpu_mat,gpumat_temp->gpu_mat,cv::Size(1280,720),0, 0, cv::INTER_LINEAR, stream);

    gpumat_temp->gpu_mat.download(cpuMat);

    QImage qImage(cpuMat.data, cpuMat.cols, cpuMat.rows, static_cast<int>(cpuMat.step), QImage::Format_BGR888);

    tmp = qImage.copy();
    view_end_time = clock_type::now();
    view_cost_time = view_end_time - view_start_time;

    Live_show_Mat->TailNext();//++

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

    Live_show_Mat->TailNext();//++

}

//------------------------------------------------------------------------------------
void ao_cameram_fun::log()
{
    printf("=========================================== \n");
    for (auto cam_it = Cam.begin(); cam_it != Cam.end(); ++cam_it)
    {
        printf("[Cam %s] cam_frame [%u] cam_lost [%u]\n",
               cam_it->Cam_d->ao_cam->Cam_IP,
               cam_it->Cam_d->cam_data_count,
               cam_it->Cam_d->cam_data_lost_count);
    }

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
    cv::cuda::GpuMat gpu_out;
    AVFrame *gpu_frame;
    AVPacket *avpkt;
    avpkt = av_packet_alloc();
    av_init_packet(avpkt);

    gpu_frame =  av_frame_alloc();

    bool is_first = true;   //防止切換時是P幀

    while (*state)
    {
        cam_d->vid_data_buf->getTail(temp);
        if(temp == nullptr)
        {
            std::unique_lock<std::mutex> lock(cam_d->vid_data_buf->data_lock);
            cam_d->vid_data_buf->data_lock_cv.wait(lock);
            continue;
        }

        if(cam_d->dc_Live_show)   //選到的相機才會變true
        {
            //printf("cam [%u] decoder live \n",cam_d->CamNO);
            if(is_first)
            {
                if(temp->type == 2)
                {
                    cam_d->vid_data_buf->TailNext();
                    continue;
                }
            }

            is_first = false;
            avpkt->data = temp->data;
            avpkt->size = temp->len;

            if(cam_d->dc->send_decode_pkt(avpkt)>=0)
            {
                //std::this_thread::sleep_for(std::chrono::milliseconds(1));  // 延遲 1 ms
                if(cam_d->dc->get_decode_frame_GPU(gpu_frame)>=0)
                {

                    MyConver::Avframe_NV12_to_Mat_BGR_2(gpu_frame,gpu_out);

                    Live_show_Mat->getHead(live_show);

                    if(live_show!=nullptr)
                    {
                        gpu_out.copyTo(live_show->gpu_mat);
                        live_show->frame_num = temp->frm_no;
                        live_show->ntp = temp->ntp_timestamp;
                        Live_show_Mat->HeadNext();//++
                        Live_show_Mat->data_lock_cv.notify_all();
                    }
                }
                else
                {
                    printf("get_decode_frame_GPU error \n");
                }
            }else
            {
                printf("send_decode_pkt error \n");
            }
        }
        else
        {
            is_first = true;
        }
        cam_d->vid_data_buf->TailNext();//++
    }
    av_frame_free(&gpu_frame);
    av_packet_unref(avpkt);
    av_packet_free(&avpkt);
    printf("[%u]Cam_Decoder  end \n" , cam_d->CamNO);


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



