#ifndef MY_CONTAINER_H
#define MY_CONTAINER_H


#include <functional>
#include <mutex>

using namespace std;

template<typename _T>

class RingBuffer
{
public:
    RingBuffer(int len , int data_size,std::function<void(_T*,int)> init_cd,std::function<void(_T*)> rel_cb);
    RingBuffer(int len , int width ,int height  ,int channels ,std::function<void(_T*,int,int,int)> init_cd,std::function<void(_T*)> rel_cb);
    RingBuffer(int len);
    ~RingBuffer();

    void init();
    void clear();
    void release();

    bool empty();
    int size();

    void getHead(_T *& d);
    void getTail(_T *& d);

    void getHead(_T ** d);
    void getTail(_T ** d);


    std::mutex data_lock;
    std::condition_variable data_lock_cv;

private:
    std::function<void(_T*,int)> init_cb;
    std::function<void(_T*,int,int,int)> init2_cb;
    std::function<void(_T*)> rel_cb;

    int data_size = 8*1024*1024;
    int width =1920;
    int height =1080;
    int channels =3;

    int len = 30;
    _T* data = nullptr;

    int Head = 0;
    int Tail = 0;

};

template<typename _T>
RingBuffer<_T>::RingBuffer(int len, int data_size,std::function<void(_T*,int)> init_cd,std::function<void(_T*)> rel_cb)
{
    this->init_cb = init_cd;
    this->rel_cb = rel_cb;
    this->len = len;
    this->data_size = data_size;
    this->data = nullptr;

    this->Head = 0;
    this->Tail = 0;
}

template<typename _T>
RingBuffer<_T>::RingBuffer(int len , int width ,int height  ,int channels ,std::function<void(_T*,int,int,int)> init2_cd,std::function<void(_T*)> rel_cb)
{
    this->init2_cb = init2_cd;
    this->rel_cb = rel_cb;
    this->len = len;

    this->width = width;
    this->height = height;
    this->channels = channels;
    this->data = nullptr;

    this->Head = 0;
    this->Tail = 0;
}


template<typename _T>
RingBuffer<_T>::RingBuffer(int len)
{
    this->len = len;
    this->data = nullptr;

    this->Head = 0;
    this->Tail = 0;
}


template<typename _T>
RingBuffer<_T>::~RingBuffer()
{
    release();
}


template<typename _T>
void RingBuffer<_T>::init() {
    this->data = new _T[len];

    if(init_cb)
    {
        for (int i = 0; i<len; ++i) {
            init_cb(&(data[i]),data_size);
        }
    }

    if(init2_cb)
    {
        for (int i = 0; i<len; ++i) {
            init2_cb(&(data[i]),width,height,channels);
        }
    }
}

template<typename _T>
void RingBuffer<_T>::release() {

    if(data == nullptr)
        return;

    if(!rel_cb)
    {
        delete[]  data;
        data =nullptr;
        return;
    }

    for (int i = 0; i<len; ++i) {
        rel_cb(&(data[i]));
    }

    if(data != nullptr)
    {
        delete[]  data;
        data =nullptr;
    }

    printf("mycontainers release \n");

}

template<typename _T>
void RingBuffer<_T>::clear()
{
    this->Head = 0;
    this->Tail = 0;
}

template<typename _T>
void RingBuffer<_T>::getHead(_T *& d) {

    d = nullptr;

    if((Head+1)% len != Tail)
    {
        d = &data[Head];
        Head = (Head+1) % len;
    }
}

template<typename _T>
void RingBuffer<_T>::getTail(_T *& d) {

    d = nullptr;

    if(Tail != Head)
    {
        d = &data[Tail];
        Tail = (Tail+1) % len;
    }
}

template<typename _T>
void RingBuffer<_T>::getHead(_T ** d) {

    *d = nullptr;

    if((Head+1)% len != Tail)
    {
        *d = &data[Head];
        Head = (Head+1) % len;
    }

}

template<typename _T>
void RingBuffer<_T>::getTail(_T ** d) {

     *d = nullptr;

    if(Tail != Head)
    {
        *d = &data[Tail];
        Tail = (Tail+1) % len;
    }
}

//============================================================
template<typename _T>
bool RingBuffer<_T>::empty()
{
    if (Tail == Head)
        return true;
    return false;
}



#endif // MY_CONTAINER_H
