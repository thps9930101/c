#ifndef BOUNDEDQUEUE_H
#define BOUNDEDQUEUE_H

#include <mutex>
#include <deque>

template<typename T>
class BoundedQueue {
public:
    explicit BoundedQueue(size_t max_Size) : maxSize(max_Size), _stop(false){}

    bool Put(const T& item) {
        std::unique_lock<std::mutex> lock(mutex);
        //printf("queue.size [%d] , maxSize [%d] , _stop [%d] \n",queue.size() , maxSize , _stop);
        _not_full.wait(lock, [this] { return queue.size() < maxSize || _stop;});
        if (_stop) {
            return false;
        }
        //queue.push_back(item);
        queue.emplace_back(item);
        _not_empty.notify_one();
        return true;
    }

    bool Get(T& input) {
        std::unique_lock<std::mutex> lock(mutex);

        _not_empty.wait(lock, [this] { return !queue.empty() || _stop;});
        if (queue.empty() && _stop) {
            return false;
        }

        //input = queue.front();
        input = std::move(queue.front());
        queue.pop_front();
        _not_full.notify_one();
        return true;
    }

    void Init() {
        std::unique_lock<std::mutex> lock(mutex);
        queue.clear();
        _stop = false;
        _not_empty.notify_all();
        _not_full.notify_all();
    }

    void Clear() {
        std::unique_lock<std::mutex> lock(mutex);
        queue.clear();
    }

    void Stop() {
        std::unique_lock<std::mutex> lock(mutex);
        _stop = true;
        _not_empty.notify_all();
        _not_full.notify_all();
    }

    void Start() {
        std::unique_lock<std::mutex> lock(mutex);
        _stop = false;
    }

    size_t Size() {
        std::unique_lock<std::mutex> lock(mutex);
        return queue.size();
    }

private:
    std::deque<T> queue;
    size_t maxSize;
    std::mutex mutex;
    std::condition_variable _not_full, _not_empty;
    bool _stop;
};

//===================================================
//example
/*
void Producer(BoundedQueue<int> *queue, int numItems)
{
    for (int i = 1; i <= numItems; ++i) {
        if (queue->Put(i))
            printf("Producer produced item: %d \n", i);
    }
    queue->Stop();
}

void Consumer(BoundedQueue<int> *queue, int th_num)
{
    int item;
    for (;;) {
        if (queue->Get(item))
            printf("Consumer %d consumed item: %d \n", th_num, item);
        else
            break;
    }
}

int main(int argc, char *argv[])
{
    BoundedQueue<int> bq(5);

    printf("start \n");

    std::thread producerThread(Producer, &bq, 100);
    std::thread consumerThread1(Consumer, &bq, 1);
    std::thread consumerThread2(Consumer, &bq, 2);


    producerThread.join();
    consumerThread1.join();
    consumerThread2.join();

    size_t s = bq.Size();
    printf("%llu \n", s);
    printf("end \n");

    return 0;
}
*/
#endif // BOUNDEDQUEUE_H
