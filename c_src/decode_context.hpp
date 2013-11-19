#ifndef __DECODE_CONTEXT_HPP_
#define __DECODE_CONTEXT_HPP_

#include <cstdint>
#include <cassert>
#include <vector>
#include "erasuerl.hpp"
#include "erasuerl_handle.hpp"
#include "coding_state.hpp"

inline size_t round_up_size(size_t origsize, erasuerl_handle *handle) 
{
    size_t newsize = origsize;
    size_t packetsize = handle->packetsize;
    size_t k = handle->k;
    size_t w = handle->w;
    if (origsize % (k * w * packetsize * sizeof(size_t)) != 0)
        while (newsize % (k * w * packetsize * sizeof(size_t)) != 0)
            newsize++;
    return newsize;
}

inline bool decode(coding_state& state)
{
    if (state.handle()->decode(state.blocksize(), state.data_blocks(),
                               state.code_blocks(), state.erasures()))
        return false;
    return true;
}

inline bool encode(coding_state& state)
{
    state.handle()->encode(state.blocksize(), state.data_blocks(),
                           state.code_blocks());
    return true;
}

template <typename T>
std::shared_ptr<coding_state> make_encode_state(erasuerl_handle* h, const T& data)
{
    
    size_t new_size = round_up_size(erasuerl_block_size(data), h);
    size_t block_size = new_size / h->k;
    auto state = std::make_shared<coding_state>(h, block_size);
    char *data_ptr = erasuerl_block_address(data);
    for (size_t i=0; i < h->k; i++)
        state->data_blocks()[i] = reinterpret_cast<char *>(data_ptr + (i * block_size));
    for (size_t i=0; i < h->m; i++) {
        T block(erasuerl_new_block<T>(block_size));
        state->code_blocks()[i] = erasuerl_block_address(block);
    }
    return state;
}

template <typename T>
std::shared_ptr<coding_state> make_decode_state(erasuerl_handle* h, 
                                std::vector<T>& user_blocks,
                                std::size_t blocksize,
                                std::size_t origsize)
{
    auto state = std::make_shared<coding_state>(h, blocksize, origsize);
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
