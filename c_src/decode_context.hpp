#ifndef __DECODE_CONTEXT_HPP_
#define __DECODE_CONTEXT_HPP_

#include <cstdint>
#include <cassert>
#include <vector>
#include "erasuerl.hpp"
#include "erasuerl_handle.hpp"
#include "erasuerl_nifs.h"

inline size_t find_blocksize(ErlNifEnv *env, ERL_NIF_TERM binlist) 
{
    ERL_NIF_TERM head, tail = binlist;
    ErlNifBinary bin;
    while (enif_get_list_cell(env, tail, &head, &tail)) {
        if (enif_inspect_binary(env, head, &bin))
            if (bin.size) return bin.size;
    }
    return 0;
}

struct decode_state
{
    decode_state(erasuerl_handle *handle, char *blocks[], size_t blocksize, std::size_t orig_size)
        : handle_(handle),
          orig_size_(orig_size),
          blocksize_(blocksize),
          data_(handle->num_blocks, nullptr),
          erasures_(handle->num_blocks, -1),
          erased_(handle->num_blocks, false)
    {
        for (size_t i=0; i < handle_->num_blocks; i++) { 
            if (blocks[i]) 
                data_[i] = blocks[i];
            else {
                data_[i] = new char[blocksize];
                erased_[i] = true;
                erasures_[num_erased_++] = i;
            }
        }
    }

    ~decode_state() 
    {
        for (size_t i=0; i < handle_->num_blocks; i++)
            if (erased_[i])
                delete[] data_[i];
    }

    char **data_blocks() const { 
        return data_;
    }

    char *data_block(size_t idx) const { 
        return data_[idx];
    }

    char **code_blocks() const { 
        return &(data_[handle_->k]);
    }
    
    std::size_t blocksize() const { 
        return blocksize_;
    }

    int* erasures() const { 
        return erasures_;
    }
    
    bool* erased() const { 
        return erased_;
    }

    size_t original_size() const { 
        return orig_size_;
    }

    size_t num_erased() const { 
        return num_erased_;
    }
    
    erasuerl_handle* handle() const {
        return handle_;
    }
    
    void dump(const char *message=nullptr) const;
private:
    erasuerl_handle *handle_ = nullptr;
    std::size_t orig_size_ = 0;
    std::size_t num_erased_ = 0;
    std::size_t blocksize_ = 0;
    unique_array<char *> data_;
    unique_array<int> erasures_;
    unique_array<bool> erased_;
};

inline bool decode(decode_state& ds)
{
    if (ds.handle()->decode(ds.blocksize(), ds.data_blocks(),
                            ds.code_blocks(), ds.erasures()))
        return false;
    return true;
}

struct decode_context
{
public:    
    explicit decode_context(erasuerl_handle *handle)
       : handle_(handle),
         dstate_(nullptr)
    {
    }
    ~decode_context()
    {
        if (dstate_)
            delete dstate_;
    }
    bool decode(ErlNifEnv *env, ERL_NIF_TERM data_blocks, ERL_NIF_TERM coding_blocks, size_t orig_size) 
    {
        ERL_NIF_TERM head, tail = data_blocks;
        unique_array<char *>blocks(handle_->num_blocks, nullptr);
        size_t idx = 0;
        auto update = [&idx, &blocks, &env](ERL_NIF_TERM item) {
            ErlNifBinary bin = {0, 0};
            enif_inspect_binary(env, item,  &bin);
            blocks[idx++] = (bin.size == 0) ? nullptr : (char *)bin.data;
        };
        while (enif_get_list_cell(env, tail, &head, &tail)) 
            update(head);
        tail = coding_blocks;
        while (enif_get_list_cell(env, tail, &head, &tail)) 
            update(head);
        dstate_ = new decode_state(handle_, blocks.data(), find_blocksize(env, data_blocks),
                                   orig_size);
        return ::decode(*dstate_);
    }

    ERL_NIF_TERM get_blocks(ErlNifEnv *env) 
    {
        ERL_NIF_TERM result[handle_->k];
        std::size_t left = dstate_->original_size();
        size_t i = 0;
        dstate_->dump("pre");
        do { 
            ErlNifBinary b = {0, 0};
            enif_alloc_binary(dstate_->blocksize(), &b);
            memcpy(b.data, dstate_->data_block(i), dstate_->blocksize());
            result[i] = enif_make_binary(env, &b);
            i++;
        } while ((left -= dstate_->blocksize()) >= dstate_->blocksize());
        if (left) { 
            ErlNifBinary b = {0, 0};
            enif_alloc_binary(left, &b);
            memcpy(b.data, dstate_->data_block(i), left);
            result[i] = enif_make_binary(env, &b);
        }
        dstate_->dump("post");
        return enif_make_list_from_array(env, result, handle_->k);
    }

private:
    erasuerl_handle* handle_ = nullptr;
    decode_state* dstate_;
};

#endif // include guard
