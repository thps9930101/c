#ifndef MEM_POOL_H
#define MEM_POOL_H

#include <iostream>
#include <vector>
#include <mutex>
#include <atomic>

#define kBlockSize 10
#define kBlockChunkSize 3
#define kFreeSlotsSize 10

template <typename T>
union Slot {
    Slot* next_ = nullptr;
    T val_;
};

template <typename T>
struct Block {
  Slot<T> slots_[kBlockSize];
  size_t idx_ = 0;
};

template <typename T>
struct BlockChunk {
  Block<T> blocks_[kBlockChunkSize];
  size_t idx_ = 0;
};

template <typename T>
struct FreeSlots {
    Slot<T>* head_ = nullptr;
    size_t length_ = 0;
};

template <typename T>
struct BlockManager {
  std::vector<BlockChunk<T>*> block_chunks_;
};

template <typename T>
struct FreeSlotsManager {
  size_t free_num_ = 0;
  std::vector<Slot<T>*> freeslots_ptrs;
};

// ===================================================
template <typename T>
class __declspec(align(64)) GlobalPool
{
private:
    BlockManager<T> block_manager_;
    FreeSlotsManager<T> freeslots_manager_;

    //pthread_spinlock_t freeslots_mtx_;
    //pthread_mutex_t block_mtx_;
    std::mutex freeslots_mtx_;
    std::mutex block_mtx_;

public:
    ~GlobalPool()
    {
        for (int i = 0; i < block_manager_.block_chunks_.size(); i++) {
            delete block_manager_.block_chunks_[i];
        }
    }


    bool PopFreeSlots(FreeSlots<T> &freeslots)
    {
        freeslots_mtx_.lock();
        // 如果Global Pool中有可用的空闲链表
        if (freeslots_manager_.free_num_ > 0) {
            freeslots.head_ = freeslots_manager_.freeslots_ptrs[--freeslots_manager_.free_num_];
            freeslots_mtx_.unlock();
            // Global Pool中每条空闲链表的长度都为kFreeSlotsSize
            freeslots.length_ = kFreeSlotsSize;
            return true;
        }
        freeslots_mtx_.unlock();
        return false;
    }


    bool PushFreeSlots(FreeSlots<T> &freeslots)
    {
        freeslots_mtx_.lock();

        // 如果freeslots_manager中存储的空闲链表的指针位置不够用，增加1000个位置
        if (freeslots_manager_.free_num_ >= freeslots_manager_.freeslots_ptrs.size()) {
            freeslots_manager_.freeslots_ptrs.resize(freeslots_manager_.freeslots_ptrs.size() + 1000);
        }

        // 将Local Pool的空闲链表的队头指针存储到freeslots_manager中
        freeslots_manager_.freeslots_ptrs[freeslots_manager_.free_num_++] = freeslots.head_;
        freeslots_mtx_.unlock();

        // 重置Local Pool中空闲链表的信息
        freeslots.head_ = NULL;
        freeslots.length_ = 0;
        return true;
    }


    // 申请空间
    bool NewBlockChunk()
    {
        BlockChunk<T> *new_block_chunk = new (std::nothrow) BlockChunk<T>;
        if (new_block_chunk == nullptr)
            return false;

        block_manager_.block_chunks_.push_back(new_block_chunk);
        return true;
    }

    Block<T>* PopBlock()
    {  
        block_mtx_.lock();
        BlockChunk<T>* block_chunk = nullptr;
        if (!block_manager_.block_chunks_.empty()) {
            block_chunk = block_manager_.block_chunks_.back();
        }

        // 如果当前BlockChunk已耗尽，申请一个新的BlockChunk
        if (block_chunk == nullptr || block_chunk->idx_ >= kBlockChunkSize)
        {
            if (NewBlockChunk())
            {
                block_chunk = block_manager_.block_chunks_.back();
                size_t res_idx = block_chunk->idx_;
                block_chunk->idx_++;
                block_mtx_.unlock();
                return &block_chunk->blocks_[res_idx];
            }
            else
            {
                block_mtx_.unlock();
                return NULL;
            }
        }
        // 如果有空闲的Block那么直接分配
        else
        {
            size_t res_idx = block_chunk->idx_;
            block_chunk->idx_++;
            block_mtx_.unlock();
            return &block_chunk->blocks_[res_idx];
        }
        block_mtx_.unlock();
        return nullptr;
    }
};

template <typename T>
class __declspec(align(64)) LocalPool
{
public:
    GlobalPool<T> *global_pool_ = nullptr;
    Block<T> *block_ = nullptr;
    FreeSlots<T> freeslots_;

    LocalPool(GlobalPool<T> *gp) {
        global_pool_ = gp;
        block_ = global_pool_->PopBlock();
        freeslots_.head_ = nullptr;
        freeslots_.length_ = 0;
    }

    T* GetObject()
    {
        // 如果freeslots还有可用空间
        if (freeslots_.head_ != nullptr)
        {
            Slot<T> *res = freeslots_.head_;
            freeslots_.head_ = res->next_;
            freeslots_.length_--;
            return (T*)res;
        }
        // 如果global pool中有可用的freeslots
        else if (global_pool_->PopFreeSlots(freeslots_))
        {
            Slot<T> *res = freeslots_.head_;
            freeslots_.head_ = res->next_;
            freeslots_.length_--;
            return (T*)res;
        }
        // 如果local pool的block还有可用空间
        else if (block_->idx_ < kBlockSize)
        {
            return (T*)&block_->slots_[block_->idx_++];
        }
        // 如果global pool还有可用的block
        else if (block_ = global_pool_->PopBlock())
        {
            return (T*)&block_->slots_[block_->idx_++];
        }
        // 没有可用的空间
        return nullptr;
    }

    void ReturnObject(T *obj)
    {
        // 如果freeslots还剩最后一个slot的回收空间
        ((Slot<T>*)obj)->next_ = freeslots_.head_;
        freeslots_.head_ = (Slot<T>*)obj;
        freeslots_.length_++;


        // 如果freeslots_中的长度满足条件，回收到global pool中
        if (freeslots_.length_ == kFreeSlotsSize) {
            global_pool_->PushFreeSlots(freeslots_);
        }
    }
};


// ===================================================
// API
template <typename T>
class ObjectPool
{
public:
    GlobalPool<T> global_pool;
    thread_local static LocalPool<T> *local_pool;

};

template <typename T>
thread_local LocalPool<T>* ObjectPool<T>::local_pool = nullptr;

// ===================================================
//example
/*
void thread_local1(ObjectPool<int> *src_op, int numItems)
{
    LocalPool<int> lp(&src_op->global_pool);
    src_op->local_pool = &lp;

    int* d;
    for (int i = 1; i <= numItems; ++i)
    {
        // get memory
        d = lp.GetObject();
        if (d == nullptr)
            printf("Get Object Fail \n");

        // use memory
        *d = i;

        // release meory
        lp.ReturnObject(d);
    }
}

int main(int argc, char *argv[])
{
    ObjectPool<int> op;
    printf("start \n");

    std::thread th1(thread_local1, &op, 100);
    th1.join();

    printf("end \n");

    return 0;
}
*/

#endif // MEM_POOL_H
