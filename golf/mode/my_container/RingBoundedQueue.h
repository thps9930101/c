#ifndef RINGBOUNDEDQUEUE_H
#define RINGBOUNDEDQUEUE_H

#include <mutex>

template<typename T>
class RingBoundedQueue {
private:
    T* buf;
    bool _stop;
    size_t maxSize;

    size_t head_pos;
    size_t tail_pos;
    std::mutex mutex;
    std::condition_variable _not_full, _not_empty;

    void bufClear() {
        head_pos = 0;
        tail_pos = 0;
    }

public:
    explicit RingBoundedQueue(size_t max_size) : maxSize(max_size), _stop(false), head_pos(0), tail_pos(0) {
        this->buf = new T[maxSize];
    }

    ~RingBoundedQueue() {
        delete [] buf;
    }

    void init() {
        std::unique_lock<std::mutex> lock(mutex);
        bufClear();
        _stop = false;
        _not_empty.notify_all();
        _not_full.notify_all();
    }

    void clear() {
        std::unique_lock<std::mutex> lock(mutex);
        bufClear();
    }

    void stop() {
        std::unique_lock<std::mutex> lock(mutex);
        _stop = true;
        _not_empty.notify_all();
        _not_full.notify_all();
    }

    void start() {
        std::unique_lock<std::mutex> lock(mutex);
        _stop = false;
    }

    size_t size() const {
        if (head_pos >= tail_pos) {
            return head_pos - tail_pos;
        } else {
            return maxSize - (tail_pos - head_pos);
        }
    }

    bool push(const T& item) {
        std::unique_lock<std::mutex> lock(mutex);
        // check idle buf
        while (((head_pos + 1) % maxSize) == tail_pos && !_stop) {
            _not_full.wait(lock);
        }
        if (_stop) {
            return false;
        }
        // push
        buf[head_pos] = item;
        head_pos = (head_pos + 1) % maxSize;
        _not_empty.notify_one();
        return true;
    }

    bool get(T& input) {
        std::unique_lock<std::mutex> lock(mutex);
        // check buf pos
        while (tail_pos == head_pos && !_stop) {
            _not_empty.wait(lock);
        }
        if (tail_pos == head_pos && _stop) {
            return false;
        }
        // get
        input = std::move(buf[tail_pos]);
        tail_pos = (tail_pos + 1) % maxSize;
        _not_full.notify_one();
        return true;
    }
};

//===================================================
//example
/*
void Producer(RingBoundedQueue<int> *queue, int numItems)
{
    for (int i = 1; i <= numItems; ++i) {
        if (queue->push(i))
            printf("Producer produced item: %d \n", i);
    }
    queue->stop();
}

void Consumer(RingBoundedQueue<int> *queue, int th_num)
{
    int item;
    for (;;) {
        if (queue->get(item))
            printf("Consumer %d consumed item: %d \n", th_num, item);
        else
            break;
    }
}

int main(int argc, char *argv[])
{
    RingBoundedQueue<int> bq(5);

    printf("start \n");

    std::thread producerThread(Producer, &bq, 100);
    std::thread consumerThread1(Consumer, &bq, 1);
    std::thread consumerThread2(Consumer, &bq, 2);


    producerThread.join();
    consumerThread1.join();
    consumerThread2.join();

    size_t s = bq.size();
    printf("%llu \n", s);
    printf("end \n");

    return 0;
}
*/

#endif // RINGBOUNDEDQUEUE_H
