#ifndef MY_CONTAINER_H
#define MY_CONTAINER_H

#include <functional>
#include <mutex>
#include <condition_variable> // 你原本就有 data_lock_cv 成員，補上 include

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
    void HeadNext();   // ⬅️ 手動推進寫入端（一格）
    void TailNext();   // ⬅️ 手動推進讀取端（一格）

    _T* getfront();
    void getfront(_T *&d);
    void getfront(_T **d);

    _T* getback();
    void getback(_T *&d);
    void getback(_T **d);

    // 取得「下一格要寫入」的位置（不前進）
    _T* getHead();
    // 取得「下一格可讀出」的位置（不前進）
    _T* getTail();

    void getHead(_T *& d);
    void getTail(_T *& d);

    void getHead(_T ** d);
    void getTail(_T ** d);

    // 取出並清掉（= 取 Tail + 推進一格）
    void getTail_clear(_T *& d);
    void getTail_clear(_T ** d);

    // 給外部自行用（本類別內不 wait/notify）
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

    // Head = 最後已「發布」的索引（最後一次 HeadNext 後的位置），初始 -1 表示尚未發布
    // Tail = 最後已「消費」的索引（最後一次 TailNext 後的位置），初始 -1 表示尚未消費
    int Head = -1;
    int Tail = -1;

    // 目前有效元素數（避免 -1 邊界的滿/空誤判）
    int count = 0;

    std::mutex internal_lock;
};

//-------------------- ctor/dtor --------------------
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
    this->count = 0;
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
    this->count = 0;
}

template<typename _T>
RingBuffer<_T>::RingBuffer(int len)
{
    this->len = len;
    this->data = nullptr;
    this->Head = -1;
    this->Tail = -1;
    this->count = 0;
}

template<typename _T>
RingBuffer<_T>::~RingBuffer()
{
    release();
}

//-------------------- init/release/clear --------------------
template<typename _T>
void RingBuffer<_T>::init()
{
    std::lock_guard<std::mutex> lock(internal_lock);

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

    this->Head = -1;
    this->Tail = -1;
    this->count = 0;
}

template<typename _T>
void RingBuffer<_T>::release()
{
    std::lock_guard<std::mutex> lock(internal_lock);

    if(data == nullptr)
        return;

    if(!rel_cb)
    {
        delete[]  data;
        data =nullptr;
    }
    else
    {
        for (int i = 0; i<len; ++i) {
            rel_cb(&(data[i]));
        }
        delete[]  data;
        data =nullptr;
    }

    this->Head = -1;
    this->Tail = -1;
    this->count = 0;

    printf("mycontainers release \n");
}

template<typename _T>
void RingBuffer<_T>::clear()
{
    std::lock_guard<std::mutex> lock(internal_lock);
    this->Head = -1;
    this->Tail = -1;
    this->count = 0;
}

//-------------------- 狀態查詢 --------------------
template<typename _T>
bool RingBuffer<_T>::empty()
{
    std::lock_guard<std::mutex> lock(internal_lock);
    return count == 0;
}

//-------------------- 手動推進（不做任何 wait） --------------------
template<typename _T>
void RingBuffer<_T>::HeadNext()
{
    std::lock_guard<std::mutex> lock(internal_lock);
    if (!data) return;
    if (count >= len) return; // 滿了，不能再發布

    // 下一個要寫入（被發布）的索引 = (Head + 1) % len；Head 初始為 -1，這裡會變 0
    Head = (Head + 1 + len) % len;
    ++count;
}

template<typename _T>
void RingBuffer<_T>::TailNext()
{
    std::lock_guard<std::mutex> lock(internal_lock);
    if (!data) return;
    if (count <= 0) return; // 空了，無可消費

    // 下一個要消費的 index = (Tail + 1) % len；Tail 初始為 -1，這裡會變 0
    Tail = (Tail + 1 + len) % len;
    --count;
}

//-------------------- 只回傳指標，不前進 --------------------
// 下一格「要寫入」的位置
template<typename _T>
_T* RingBuffer<_T>::getHead()
{
    std::lock_guard<std::mutex> lock(internal_lock);
    if (!data) return nullptr;
    if (count >= len) return nullptr; // 滿了，沒有可寫槽

    int writePos = (Head + 1 + len) % len; // Head=-1 時會得到 0
    return &data[writePos];
}

// 下一格「可讀出」的位置
template<typename _T>
_T* RingBuffer<_T>::getTail()
{
    std::lock_guard<std::mutex> lock(internal_lock);
    if (!data) return nullptr;
    if (count <= 0) return nullptr; // 空了，沒有可讀槽

    int readPos = (Tail + 1 + len) % len; // Tail=-1 時會得到 0
    return &data[readPos];
}

template<typename _T>
void RingBuffer<_T>::getHead(_T *& d)
{
    d = getHead();
}

template<typename _T>
void RingBuffer<_T>::getTail(_T *& d)
{
    d = getTail();
}

template<typename _T>
void RingBuffer<_T>::getHead(_T ** d)
{
    *d = getHead();
}

template<typename _T>
void RingBuffer<_T>::getTail(_T ** d)
{
    *d = getTail();
}

// 取出並清掉（= 取 Tail + 推進一格）
template<typename _T>
void RingBuffer<_T>::getTail_clear(_T *& d)
{
    d = getTail();
    if (d) TailNext();
}

template<typename _T>
void RingBuffer<_T>::getTail_clear(_T ** d)
{
    *d = getTail();
    if (*d) TailNext();
}

//-------------------- front/back（參考：直接看 Head/Tail 所在格） --------------------
template<typename _T>
_T* RingBuffer<_T>::getfront()
{
    std::lock_guard<std::mutex> lock(internal_lock);
    if (!data || count == 0 || Head == -1) return nullptr;
    return &data[Head];
}

template<typename _T>
void RingBuffer<_T>::getfront(_T *& d)
{
    d = getfront();
}

template<typename _T>
void RingBuffer<_T>::getfront(_T ** d)
{
    *d = getfront();
}

template<typename _T>
_T* RingBuffer<_T>::getback()
{
    std::lock_guard<std::mutex> lock(internal_lock);
    if (!data || Tail == -1) return nullptr; // 尚未消費過
    return &data[Tail];
}

template<typename _T>
void RingBuffer<_T>::getback(_T *& d)
{
    d = getback();
}

template<typename _T>
void RingBuffer<_T>::getback(_T ** d)
{
    *d = getback();
}

#endif // MY_CONTAINER_H
