#ifndef OBJECTPOOL2_H
#define OBJECTPOOL2_H

#include <iostream>
#include <vector>
#include <mutex>

typedef std::chrono::high_resolution_clock stdget_time;
typedef std::chrono::duration<double> std_count_sec;
typedef std::chrono::duration<double, std::milli> std_count_milli;

namespace  Customize{

#define kNodeArrayMaxSize 10

template <typename T>
struct Node {
    Node* next_ = nullptr;
    T* val_ = nullptr;
};

template <typename T>
struct T_Array {
    T t_arr[kNodeArrayMaxSize];
    size_t idx_ = 0;
};

template <typename T>
struct NodeArray {
    Node<T> node_arr[kNodeArrayMaxSize];
    size_t idx_ = 0;
};


template <typename T>
struct NodeLinkList {
    Node<T>* head_ = nullptr;
    size_t length_;
};

template <typename T>
struct T_Manager {
    std::vector<T_Array<T>*> T_array_ptr;
};

template <typename T>
struct Node_Manager {
    std::vector<NodeArray<T>*> Node_array_ptr;
};

template <typename T>
struct DataManager {
    size_t free_num = 0;
    std::vector<Node<T>*> NodeLinkLists_ptr;
};

template <typename T>
struct DataFreeManager {
    size_t free_num = 0;
    std::vector<Node<T>*> FreeNodeLists_ptr;
};

// ===================================================
template <typename T>
class __declspec(align(64)) GlobalPool
{
private:
    DataManager<T> data_manager_;
    DataFreeManager<T> data_free_manager_;
    T_Manager<T> t_manager_;
    Node_Manager<T> node_manager_;

    std::mutex t_mtx;
    std::mutex node_mtx;
    std::mutex free_mtx_;
    std::mutex data_mtx_;

public:
    ~GlobalPool()
    {
        printf("~GlobalPool \n");

        for (int i=0; i<node_manager_.Node_array_ptr.size(); i++) {
            delete node_manager_.Node_array_ptr[i];
        }

        for (int i=0; i<t_manager_.T_array_ptr.size(); i++) {
            delete t_manager_.T_array_ptr[i];
        }
    }

    void CreateIdleDataList(size_t s)
    {
        NodeLinkList<T>* tmp = new NodeLinkList<T>[s];
        for (size_t i=0; i<s; i++) {
            NewNodeList(tmp[i]);
        }

        for (size_t i=0; i<s; i++) {
            PushDataLists(tmp[i]);
        }
        delete[] tmp;
    }

    void CreateIdleFreeList(size_t s)
    {
        NodeLinkList<T>* tmp = new NodeLinkList<T>[s];
        for (size_t i=0; i<s; i++) {
            NewFreeNodeList(tmp[i]);
        }

        for (size_t i=0; i<s; i++) {
            PushFreeLists(tmp[i]);
        }
        delete[] tmp;
    }

    //------------------------------------------------
    bool NewFreeNodeList(NodeLinkList<T> &freelist)
    {       
        node_mtx.lock();
        NodeArray<T>* node_array = new NodeArray<T>;
        if (node_array == nullptr) {
            node_mtx.unlock();
            return false;
        }
        node_manager_.Node_array_ptr.push_back(node_array);
        node_mtx.unlock();

        Node<T>* listhead = nullptr;
        for (int i=0; i<kNodeArrayMaxSize; i++)
        {
            node_array->node_arr[i].val_ = nullptr;
            node_array->node_arr[i].next_ = listhead;
            listhead = &node_array->node_arr[i];
        }
        freelist.head_ = listhead;
        freelist.length_ = kNodeArrayMaxSize;
        return true;
    }

    bool PopFreeLists(NodeLinkList<T> &freelist)
    {
        free_mtx_.lock();
        // 如果Global Pool中有可用的空閒鏈表
        if (data_free_manager_.free_num > 0)
        {
            freelist.head_ = data_free_manager_.FreeNodeLists_ptr[--data_free_manager_.free_num];
            free_mtx_.unlock();
            freelist.length_ = kNodeArrayMaxSize;
            return true;
        }
        // 建立新的空閒鏈表
        if (NewFreeNodeList(freelist))
        {
            free_mtx_.unlock();
            return true;
        }
        free_mtx_.unlock();
        return false;
    }

    bool PushFreeLists(NodeLinkList<T> &freelist)
    {
        free_mtx_.lock();

        // 如果data_free_manager_中存储的空闲链表的指针位置不够用，增加1000个位置
        if (data_free_manager_.free_num >= data_free_manager_.FreeNodeLists_ptr.size()) {
            data_free_manager_.FreeNodeLists_ptr.resize(data_free_manager_.FreeNodeLists_ptr.size() + 1000);
        }

        // 将Local Pool的空闲链表的队头指针存储到data_free_manager_中
        data_free_manager_.FreeNodeLists_ptr[data_free_manager_.free_num++] = freelist.head_;
        free_mtx_.unlock();

        // 重置Local Pool中空闲链表的信息
        freelist.head_ = nullptr;
        freelist.length_ = 0;
        return true;
    }

    //------------------------------------------------
    bool NewNodeList(NodeLinkList<T>& datalist)
    {
        // 申請T陣列空間
        t_mtx.lock();
        T_Array<T>* t_array = new T_Array<T>;
        if (t_array == nullptr) {
            t_mtx.unlock();
            return false;
        }
        t_manager_.T_array_ptr.push_back(t_array);
        t_mtx.unlock();

        // 申請Node陣列空間
        node_mtx.lock();
        NodeArray<T>* node_array = new NodeArray<T>;
        if (node_array == nullptr) {
            node_mtx.unlock();
            return false;
        }
        node_manager_.Node_array_ptr.push_back(node_array);
        node_mtx.unlock();

        //建立新鏈表
        Node<T>* listhead = nullptr;
        for (int i=0; i<kNodeArrayMaxSize; i++)
        {
            node_array->node_arr[i].val_ = &(t_array->t_arr[i]);
            node_array->node_arr[i].next_ = listhead;
            listhead = &node_array->node_arr[i];
        }
        datalist.head_ = listhead;
        datalist.length_ = kNodeArrayMaxSize;
        return true;
    }

    bool PopDataLists(NodeLinkList<T>& datalist)
    {
        data_mtx_.lock();
        // 如果Global Pool中有可用的空閒鏈表
        if (data_manager_.free_num > 0)
        {
            datalist.head_ = data_manager_.NodeLinkLists_ptr[--data_manager_.free_num];
            data_mtx_.unlock();
            datalist.length_ = kNodeArrayMaxSize;
            return true;
        }
        // 建立新的空閒鏈表
        if (NewNodeList(datalist))
        {
            data_mtx_.unlock();
            return true;
        }
        data_mtx_.unlock();
        return false;
    }

    bool PushDataLists(NodeLinkList<T>& datalist)
    {
        data_mtx_.lock();
        // 如果data_manager_中存储的空闲链表的指针位置不够用，增加1000个位置
        if (data_manager_.free_num >= data_manager_.NodeLinkLists_ptr.size()) {
            data_manager_.NodeLinkLists_ptr.resize(data_manager_.NodeLinkLists_ptr.size() + 1000);
        }

        // 将Local Pool的空闲链表的队头指针存储到data_manager_中
        data_manager_.NodeLinkLists_ptr[data_manager_.free_num++] = datalist.head_;
        data_mtx_.unlock();

        // 重置Local Pool中空闲链表的信息
        datalist.head_ = nullptr;
        datalist.length_ = 0;
        return true;
    }

    //------------------------------------------------
    size_t get_data_size()
    {
        return t_manager_.T_array_ptr.size();
    }

};

// ===================================================
template <typename T>
class __declspec(align(64)) LocalPool
{
public:
    GlobalPool<T> *global_pool_ = nullptr;
    NodeLinkList<T> data_list;
    NodeLinkList<T> free_list;

    LocalPool(GlobalPool<T>*gp) {
        global_pool_ = gp;
        data_list.head_ = nullptr;
        data_list.length_ = 0;
        free_list.head_ = nullptr;
        free_list.length_ = 0;
    }

    T* GetObject()
    {
        T* res = nullptr;
        // 如果datalist沒有空間,向global pool申請新的datalist空間
        if (data_list.head_ == nullptr) {
            // 没有可用的空间
            if (!global_pool_->PopDataLists(data_list)) {
                return res;
            }
        }

        Node<T>* n = data_list.head_;
        data_list.head_ = n->next_;
        data_list.length_--;

        res = n->val_;
        n->val_ = nullptr;

        n->next_ = free_list.head_;
        free_list.head_ = n;
        free_list.length_++;

        // 如果freeslots_中的长度满足条件，回收到global pool中
        if (free_list.length_ == kNodeArrayMaxSize) {
            global_pool_->PushFreeLists(free_list);
        }
        return res;
    }

    void ReturnObject(T *obj)
    {
        // 如果freelist沒有空間,向global pool申請新的freelist空間
        if (free_list.head_ == nullptr) {
            // 没有可用的空间(記憶體洩漏)
            if (!global_pool_->PopFreeLists(free_list)) {
                return;
            }
        }
        Node<T>* n = free_list.head_;
        free_list.head_ = n->next_;
        free_list.length_--;

        n->val_ = obj;

        n->next_ = data_list.head_;
        data_list.head_ = n;
        data_list.length_++;

        // 如果datalist_中的长度满足条件，回收到global pool中
        if (data_list.length_ == kNodeArrayMaxSize) {
            global_pool_->PushDataLists(data_list);
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

}

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


#endif // OBJECTPOOL2_H
