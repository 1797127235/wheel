#pragma once
#include<atomic>
#include <cstddef>
struct Manager{
    Manager():
        _share_count(0),
        _weak_count(0)
    {}
    std::atomic<long> _share_count;
    std::atomic<long> _weak_count;

};

template<class T> class weak_ptr;

template<class T>
class shared_ptr
{
public:
    friend class weak_ptr<T>;
private:
    T* _ptr;
    Manager* manager;

    void release()
    {
        if(!manager) return;

        //保证释放和获取语义
        long old = manager->_share_count.fetch_sub(1,std::memory_order_acq_rel);
        if(old == 1)
        {
            delete _ptr;
            _ptr = nullptr;

            if(manager->_weak_count.load(std::memory_order_acquire) == 0)
            {
                delete manager;

                manager = nullptr;
            }
        }
    }
public:
    shared_ptr(T* p = nullptr):
        _ptr(p)
    {
        if(_ptr){
            manager = new Manager();
            ++manager->_share_count;
        }
        else {
            manager = nullptr;
        }
    }

    //拷贝构造
    shared_ptr(const shared_ptr<T>& other)
    {
        _ptr = other._ptr;
        manager = other.manager;
        if(manager)
        {
            ++manager->_share_count;
        }
    }

    //赋值运算
    shared_ptr<T>& operator=(const shared_ptr<T>& other)
    {
        if(this != &other)
        {
            release();
            _ptr = other._ptr;
            manager = other.manager;
            if(manager)
            {
                ++manager->_share_count;
            }
        }
        return *this;
    }

    ~shared_ptr()
    {
        release();
    }

    //常用运算符

    T& operator*() const
    {
        return *_ptr;
    }

    T* operator->() const
    {
        return _ptr;
    }

    T* get() const
    {
        return _ptr;
    }

    int use_count() const
    {
        return manager->_share_count;
    }

    bool unique() const
    {
        return manager->_share_count == 1;
    }

};

template<class T>
class weak_ptr
{
private:
    T* _ptr;
    Manager* manager;

    void release()
    {
        if(!manager) return;

        long old = manager->_weak_count.fetch_sub(1,std::memory_order_acq_rel);
        if(old == 1)
        {
            if(manager->_share_count.load(std::memory_order_acquire) == 0)
            {
                delete manager;
                manager = nullptr;
            }
        }
    }
public:
    weak_ptr() noexcept : _ptr(nullptr), manager(nullptr) {}

    weak_ptr(const shared_ptr<T>& other) noexcept
    {
        _ptr = other._ptr;
        manager = other.manager;
        if(manager)
        {
            ++manager->_weak_count;
        }
    }

    ~weak_ptr()
    {
        if(manager)
        {
            release();
        }
    }

    //拷贝构造
    weak_ptr(const weak_ptr<T>& other) noexcept
    {
        _ptr = other._ptr;
        manager = other.manager;
        if(manager)
        {
            ++manager->_weak_count;
        }
    }

    //赋值运算符
    weak_ptr<T>& operator=(const weak_ptr<T>& other) noexcept
    {
        if(this != &other)
        {
            release();
            _ptr = other._ptr;
            manager = other.manager;
            if(manager)
            {
                ++manager->_weak_count;
            }
        }
        return *this;
    }

    /*
        如果lock()写成
        if(shared_count > 0) ++shared_count;

        判断时shared_count = 1
        被切走
        别的线程把对象delete了
        回来执行++shared_count把 0变成1->对象复活了
        所以必须把判断>0和++share_count变成一个不可分割的原子操作

        使用CAS保证原子性
    */
    shared_ptr<T> lock() const
    {
        Manager* m = manager;
        if(!m) return shared_ptr<T>(nullptr);

        //我期望读的
        long cnt = m->_share_count.load(std::memory_order_acquire);

        while(cnt > 0)
        {   

            if(m->_share_count.compare_exchange_weak(cnt,cnt + 1,std::memory_order_acq_rel,std::memory_order_acquire))
            {
                shared_ptr<T> ps;
                ps._ptr = _ptr;
                ps.manager = m;
                return ps;
            }
        }
        return shared_ptr<T>(nullptr);
    }

};
