#ifndef __ERASUERL_BLOCKS_HPP_
#define __ERASUERL_BLOCKS_HPP_

#include <cstdint>
#include "erasuerl.h"

struct coding_block { 
    coding_block(std::size_t m, const std::size_t blocksize)
        : data_(new char*[m]),
          m_(m),
          blocksize_(blocksize)
    {
        for (int i=0; i<m_; i++) 
            data_[i] = new char[blocksize_];
            
    }

    ~coding_block()
    {
        for (int i=0; i<m_; i++)
            delete data_[i];
        delete data_;
    }

    char * operator[](const std::size_t index) const
    {
        return data_[index];
    }

    operator char**() const 
    { 
        return data_;
    }
private:
    char **data_;
    std::size_t m_;
    std::size_t blocksize_;
};

struct data_blocks { 

    data_blocks(const std::size_t k) 
        : 
        data_(new char*[k]),
        k_(k)
    {
    }

    void set_data(char *data, const std::size_t blocksize) 
    {
        for (int i=0;i<k_;i++)
            data_[i] = data+(i*blocksize);        
    }

    ~data_blocks() 
    {
        delete data_;
    }

    char * operator[](const std::size_t index) const
    {
        return data_[index];
    }

    char *& operator[](const std::size_t index)
    {
        return data_[index];
    }

    operator char**() const 
    { 
        return data_;
    }               
private:
    char **data_;
    std::size_t k_;
};

#endif // include guard
