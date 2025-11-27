#include <cstddef>
#include<memory>
#include<iostream>
template <class T, class Deleter = std::default_delete<T>>
class unique_ptr
{
private:
    T* _ptr;
    Deleter _del;
public:
    explicit unique_ptr(T * ptr = nullptr, Deleter del = Deleter()) : _ptr(ptr), _del(del)
    {

    }

    //禁止拷贝
    unique_ptr(const unique_ptr&) = delete;

    //允许移动
    unique_ptr(unique_ptr&& other) noexcept:
        _del(other._del),
        _ptr(other._ptr)
    {
        other._ptr = nullptr;
    }

    unique_ptr& operator=(unique_ptr&& other) noexcept{
        if(this != &other)
        {
            //先释放旧的资源防止泄露
            _del(_ptr);

            _del = other._del;
            _ptr = other._ptr;
            other._ptr = nullptr;
        }
        return *this;
    }

    ~unique_ptr()
    {
        if(_ptr)
        _del(_ptr);
    }

    T& operator*() const
    {
        return *_ptr;
    }

    T* get() const
    {
        return _ptr;
    }

    T* operator->() const
    {
        return _ptr;
    }

    T* release() noexcept
    {
        T* ptr = _ptr;
        _ptr = nullptr;
        return ptr;
    }

    //重置管理对象
    void reset(T* ptr = nullptr)
    {
        if(_ptr) _del(_ptr);
        _ptr = ptr;
    }
};