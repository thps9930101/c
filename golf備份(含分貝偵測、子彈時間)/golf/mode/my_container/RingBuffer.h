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
    void HeadNext();
    void TailNext();

    _T* getfront();
    void getfront(_T *&d);
    void getfront(_T **d);

    _T* getback();
    void getback(_T *&d);
    void getback(_T **d);


    _T* getHead();
    _T* getTail();

    void getHead(_T *& d);
    void getTail(_T *& d);

    void getHead(_T ** d);
    void getTail(_T ** d);

    void getTail_clear(_T *& d);
    void getTail_clear(_T ** d);


    std::mutex data_lock;
    std::condition_variable data_lock_cv;

private:
    std::function<void(_T*,int)> init_cb;
    std::function<void(_T*,int,int,int)> init2_cb;
    std::function<void(_T*)> rel_cb;

    /*
    __declspec(align(64)) int data_size = 8*1024*1024;
    __declspec(align(64)) int len = 30;
    __declspec(align(64)) _T* data = nullptr;

    __declspec(align(64)) int Head = 0;
    __declspec(align(64)) int Tail = 0;
    */


    int data_size = 8*1024*1024;
    int width =1920;
    int height =1080;
    int channels =3;

    int len = 30;
    _T* data = nullptr;

    int Head = -1;
    int Tail = -1;

};

template<typename _T>
RingBuffer<_T>::RingBuffer(int len, int data_size,std::function<void(_T*,int)> init_cd,std::function<void(_T*)> rel_cb)
{
    this->init_cb = init_cd;
    this->rel_cb = rel_cb;
    this->len = len;
    this->data_size = data_size;
    this->data = nullptr;

    this->Head = -1;
    this->Tail = -1;
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

    this->Head = -1;
    this->Tail = -1;
}


template<typename _T>
RingBuffer<_T>::RingBuffer(int len)
{
    this->len = len;
    this->data = nullptr;

    this->Head = -1;
    this->Tail = -1;
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
    this->Head = -1;
    this->Tail = -1;
}


template<typename _T>
_T* RingBuffer<_T>::getHead() {
    _T *temp = nullptr;

    if(Tail == -1 && Head != -1 && (Head+1)% len ==0)
        return temp;

    if((Head+1)% len != Tail )
    {
        Head = (Head+1) % len;
        temp = &data[Head];
    }

    return temp;
}

template<typename _T>
_T* RingBuffer<_T>::getTail() {
    _T *temp = nullptr;

    if((Tail+1)% len != Head && Head != -1)
    {
        Tail = (Tail+1) % len;
        temp = &data[Tail];
    }

    return temp;
}

template<typename _T>
void RingBuffer<_T>::getHead(_T *& d) {

    d = nullptr;

    if(Tail == -1 && Head != -1 && (Head+1)% len ==0)
        return;

    if((Head+1)% len != Tail)
    {
        Head = (Head+1) % len;
        d = &data[Head];
    }

}

template<typename _T>
void RingBuffer<_T>::getTail(_T *& d) {

    d = nullptr;

    if((Tail+1)% len != Head && Head != -1)
    {
        Tail = (Tail+1) % len;
        d = &data[Tail];
    }
}

template<typename _T>
void RingBuffer<_T>::getHead(_T ** d) {

    *d = nullptr;

    if(Tail == -1 && Head != -1 && (Head+1)% len ==0)
        return;

    if((Head+1)% len != Tail)
    {
        Head = (Head+1) % len;
        *d = &data[Head];
    }

}

template<typename _T>
void RingBuffer<_T>::getTail(_T ** d) {

     *d = nullptr;

    if((Tail+1)% len != Head && Head != -1)
    {
        Tail = (Tail+1) % len;
        *d = &data[Tail];
    }
}

template<typename _T>
void RingBuffer<_T>::getTail_clear(_T *& d)
{
    d = nullptr;

   if(Tail != Head && Head != -1)
   {
       Tail = (Tail+1) % len;
       d = &data[Tail];
   }
}

template<typename _T>
void RingBuffer<_T>::getTail_clear(_T ** d)
{
    *d = nullptr;

   if(Tail != Head && Head != -1)
   {
       Tail = (Tail+1) % len;
       *d = &data[Tail];
   }
}


//============================================================
template<typename _T>
bool RingBuffer<_T>::empty()
{
    if ((Tail+1)% len == Head || Head != -1)
        return true;
    return false;
}

template<typename _T>
void RingBuffer<_T>::HeadNext()
{
    int newHead = (Head + 1) % len;

    if(Tail == -1 && Head != -1 &&newHead ==0)
        return;

    if (newHead != Tail) {
        Head = newHead;
    }
}

template<typename _T>
void RingBuffer<_T>::TailNext()
{
    int newTail = ((Tail+1) % len);
    if (newTail != Head && Head != -1) {
        Tail = newTail;
    }
}


template<typename _T>
_T* RingBuffer<_T>::getfront()
{
    if (Head == -1)
        return nullptr;
    return &data[Head];
}

template<typename _T>
void RingBuffer<_T>::getfront(_T *& d)
{
    if (Head != -1)
        d = &data[Head];
    else
        d = nullptr;
}

template<typename _T>
void RingBuffer<_T>::getfront(_T **d)
{
    if (Head != -1)
        *d = &data[Head];
    else
        *d = nullptr;
}

template<typename _T>
_T* RingBuffer<_T>::getback()
{
    if (Tail == -1)
        return nullptr;
    return &data[Tail];
}

template<typename _T>
void RingBuffer<_T>::getback(_T *&d)
{
    if (Tail != -1)
        d = &data[Tail];
    else
        d = nullptr;
}

template<typename _T>
void RingBuffer<_T>::getback(_T **d)
{
    if (Tail != -1)
        *d = &data[Tail];
    else
        *d = nullptr;
}

//==========================================================
//example
/*
//test1
void rb_init(int* d, int n)
{
    *d = 0;
}

void rb_del(int* d)
{

}

void Producer(mycontainers<int> *rb, int numItems)
{
    int* d;
    int i=0;
    for (;;)
    {
        // get memory
        rb->getHead(d);
        if (d == nullptr) {
            rb->data_lock_cv.notify_all();
            continue;
        }

        // use memory
        *d = ++i;
        rb->data_lock_cv.notify_all();
        if (i>numItems)
            break;

    }
    printf("Producer end \n");
}

void Consumer(mycontainers<int> *rb, int th_num)
{
    int* item;
    int c;
    for (;;)
    {
        rb->getTail(item);
        if (item == nullptr) {
            std::unique_lock<std::mutex> lock(rb->data_lock);
            rb->data_lock_cv.wait(lock);
            continue;
        }
        if (*item >= th_num) {
            c = *item;
            break;
        }
    }
    printf("final cnt %d \n", c);
}

//test2
typedef struct Mydata {
    int cnt;
    uint8_t* data;
} md;

void rb_init2(md* d, int n)
{
    d->cnt = 0;
    d->data = new uint8_t[n];
}

void rb_del2(md* d)
{
    if (d->data == nullptr)
        return;
    delete[] d->data;
    d->data = nullptr;
}

void Producer2(mycontainers<md> *rb, int numItems)
{
    md* d;
    int i=0;
    for (;;)
    {
        // get memory
        rb->getHead(d);
        if (d == nullptr) {
            rb->data_lock_cv.notify_all();
            continue;
        }

        // use memory
        d->cnt = ++i;
        rb->data_lock_cv.notify_all();
        if (i>numItems)
            break;

    }
    printf("Producer end \n");
}

void Consumer2(mycontainers<md> *rb, int th_num)
{
    md* item;
    int c;
    for (;;)
    {
        rb->getTail(item);
        if (item == nullptr) {
            std::unique_lock<std::mutex> lock(rb->data_lock);
            rb->data_lock_cv.wait(lock);
            continue;
        }
        if (item->cnt >= th_num) {
            c = item->cnt;
            break;
        }
    }
    printf("final cnt %d \n", c);
}

int main(int argc, char *argv[])
{
    // test1
    mycontainers<int> ring_buf(rb_init, rb_del, 10, 0);
    ring_buf.init();

    printf("start \n");
    int th_num = 100;

    std::thread producerThread1(Producer, &ring_buf, th_num);
    std::thread consumerThread1(Consumer, &ring_buf, th_num);

    producerThread1.join();
    consumerThread1.join();

    // test2
    mycontainers<md> ring_buf(rb_init2, rb_del2, 10, 1024);
    ring_buf.init();

    printf("start \n");
    int th_num = 100;

    std::thread producerThread2(Producer2, &ring_buf, th_num);
    std::thread consumerThread2(Consumer2, &ring_buf, th_num);

    producerThread2.join();
    consumerThread2.join();
    printf("end \n");

    return 0;
}
*/

#endif // MY_CONTAINER_H
