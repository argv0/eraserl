#ifndef __ERASUERL_HPP_
#define __ERASUERL_HPP_

#include <algorithm>

static const size_t DEFAULT_K = 9;
static const size_t DEFAULT_M = 4;
static const size_t DEFAULT_W = 4;
static const size_t MAX_K = 255;
static const size_t MAX_M = 255;


template <typename T>
struct unique_array
{
    unique_array(std::size_t size) 
      : size_(size),
        data_(new T[size]) {}

    unique_array(std::size_t size, const T& initval) 
    : unique_array(size) 
    {
        std::fill(&data_[0], &data_[size], initval);
    }

    ~unique_array()
    {
        delete[] data_;
    }

    size_t size() const { return size_; }

    T* data() const 
    {
        return data_;
    }

    operator T*() const 
    {
        return data_;
    }

    T& operator[](std::size_t index) const
    {
        return data_[index];
    }
    
private:
    unique_array(const unique_array&) = delete;
    const unique_array& operator =(const unique_array&) = delete;
    std::size_t size_;
    T* data_;
};



#endif // include guard
