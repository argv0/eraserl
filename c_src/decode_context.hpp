#ifndef __DECODE_CONTEXT_HPP_
#define __DECODE_CONTEXT_HPP_

#include <cstdint>
#include <cassert>
#include <vector>
#include "erasuerl.hpp"
#include "erasuerl_handle.hpp"
#include "erasuerl_nifs.h"

struct decode_data 
{
    decode_data(std::size_t num_blocks)
        :
        data_(num_blocks, nullptr),
        erasures_(num_blocks, -1),
        erased_(num_blocks, false) 
    {
    }
protected:
    std::vector<char *> data_;
    std::vector<int> erasures_;
    std::vector<char> erased_;
};

struct decode_state : public decode_data
{
    decode_state(erasuerl_handle *handle,
                 std::size_t blocksize, 
                 std::size_t orig_size)
        : decode_data(handle->num_blocks),
          handle_(handle),
          orig_size_(orig_size),
          blocksize_(blocksize)
    {
    }

    char **data_blocks()  { 
        return data_.data();
    }

    char *data_block(size_t idx) const { 
        return data_[idx];
    }

    char **code_blocks() { 
        return &(data_[handle_->k]);
    }

    std::size_t blocksize() const { 
        return blocksize_;
    }

    int* erasures()  { 
        return erasures_.data();
    }
    
    char* erased() { 
        return erased_.data();
    }

    size_t original_size() const { 
        return orig_size_;
    }
    
    erasuerl_handle* handle() const {
        return handle_;
        
    }
    void data_block(size_t idx, char *block)
    {
        data_[idx] = block;
    }

    void erasure(size_t idx) { 
        erasures_[num_erased_++] = idx;
        erased_[idx] = true;
    }

    std::size_t num_erased() const { 
        return num_erased_;
    }

    void dump(const char *message=nullptr) const;


private:
    erasuerl_handle *handle_ = nullptr;
    std::size_t orig_size_ = 0;
    std::size_t blocksize_ = 0;
    std::size_t num_erased_ = 0;
};

inline bool decode(decode_state& ds)
{
    if (ds.handle()->decode(ds.blocksize(), ds.data_blocks(),
                            ds.code_blocks(), ds.erasures()))
        return false;
    return true;
}

template <class T> 
struct decoder 
{
    typedef T block_type;

    decoder(erasuerl_handle* h, std::vector<T>& user_blocks, std::size_t blocksize,
            std::size_t origsize)
        :  user_blocks_(user_blocks),
           state_(new decode_state(h, blocksize, origsize))
    {           
        for (size_t i=0; i < h->num_blocks; i++) {
            block_type& block(user_blocks_.at(i));
            if (erasuerl_is_erasure(block)) add_erasure(i);
            state_->data_block(i, erasuerl_block_address(block));
        }
    }

    ~decoder() 
    {
        for (size_t i=0; i < user_blocks_.size(); ++i)
            if (state_->erased()[i])
                erasuerl_free_block(user_blocks_[i]);
    }
    void add_erasure(size_t idx) 
    {
        state_->erasure(idx);
        user_blocks_[idx] = erasuerl_new_block<T>(state_->blocksize());
    }
    
    bool operator()() 
    {
        if (!state_->num_erased()) return true;
        state_->dump("pre");
        auto ret =  ::decode(*state_);
        state_->dump("post");
        return ret;
    }
private:
    std::vector<T>& user_blocks_;
    decode_state *state_;
};

#endif // include guard
