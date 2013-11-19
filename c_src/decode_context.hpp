#ifndef __DECODE_CONTEXT_HPP_
#define __DECODE_CONTEXT_HPP_

#include <cstdint>
#include <cassert>
#include <vector>
#include "erasuerl.hpp"
#include "erasuerl_handle.hpp"
#include "erasuerl_nifs.h"

struct decode_state 
{
    decode_state(erasuerl_handle *handle,
                 std::size_t blocksize, 
                 std::size_t orig_size)
        :
          handle_(handle),
          data_(handle->num_blocks, nullptr),
          erasures_(handle->num_blocks, -1),
          erased_(handle->num_blocks, false),
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
    std::vector<char *> data_;
    std::vector<int> erasures_;
    std::vector<char> erased_;
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

template <typename T>
decode_state* make_decode_state(erasuerl_handle* h, 
                                std::vector<T>& user_blocks,
                                std::size_t blocksize,
                                std::size_t origsize)
{
    decode_state* state = new decode_state(h, blocksize, origsize);
    for (size_t i=0; i < h->num_blocks; i++) {
        T& block(user_blocks.at(i));
        if (erasuerl_is_erasure(block)) 
        {
            state->erasure(i);
            user_blocks[i] = erasuerl_new_block<T>(blocksize);
        }
        state->data_block(i, erasuerl_block_address(block));
    }
    return state;
}

#endif // include guard
