#ifndef __DECODE_CONTEXT_HPP_
#define __DECODE_CONTEXT_HPP_

#include <cstdint>
#include <cassert>
#include <vector>
#include "erasuerl.h"
#include "erasuerl_handle.hpp"

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
        : handle(handle),
          orig_size(orig_size),
          blocksize(blocksize),
          data(handle->num_blocks, nullptr),
          erasures(handle->num_blocks, -1),
          erased(handle->num_blocks, false)
    {
        for (size_t i=0; i < handle->num_blocks; i++) { 
            if (blocks[i]) 
                data[i] = blocks[i];
            else {
                data[i] = new char[blocksize];
                erased[i] = true;
                erasures[num_erased++] = i;
            }
        }
    }

    ~decode_state() 
    {
        for (size_t i=0; i < handle->num_blocks; i++)
            if (erased[i])
                delete[] data[i];
    }

    erasuerl_handle *handle = nullptr;
    std::size_t orig_size = 0;
    std::size_t num_erased = 0;
    std::size_t blocksize = 0;
    unique_array<char *> data;
    unique_array<int> erasures;
    unique_array<bool> erased;
};


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

    int decode(ErlNifEnv *env, ERL_NIF_TERM data_blocks, ERL_NIF_TERM coding_blocks, size_t orig_size) 
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
        return handle_->decode(dstate_->blocksize, dstate_->data, &dstate_->data[handle_->k], 
                               dstate_->erasures);
    }

    ERL_NIF_TERM get_blocks(ErlNifEnv *env) 
    {
        ERL_NIF_TERM result[handle_->k];
        std::size_t left = dstate_->orig_size;
        size_t i = 0;
        std::size_t totes = 0;
        dump_data("pre_get");
        do { 
            ErlNifBinary b = {0, 0};
            enif_alloc_binary(dstate_->blocksize, &b);
            totes += dstate_->blocksize;
            memcpy(b.data, dstate_->data[i], dstate_->blocksize);
            result[i] = enif_make_binary(env, &b);
            i++;
        } while ((left -= dstate_->blocksize) >= dstate_->blocksize);
        if (left) { 
            ErlNifBinary b = {0, 0};
            enif_alloc_binary(left, &b);
            memcpy(b.data, dstate_->data[i], left);
            result[i] = enif_make_binary(env, &b);
        }
        dump_data("post_get");
        return enif_make_list_from_array(env, result, handle_->k);
    }
    void dump_data(const char *message=nullptr) const;

private:
    erasuerl_handle* handle_ = nullptr;
    decode_state* dstate_;
};

#endif // include guard
