#ifndef __ERASUERL_HPP_
#define __ERASUERL_HPP_

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>
#include "coding_state.hpp"
#include "erasuerl_handle.hpp"

static const size_t DEFAULT_K = 9;
static const size_t DEFAULT_M = 4;
static const size_t DEFAULT_W = 4;
static const size_t MAX_K = 255;
static const size_t MAX_M = 255;

template <typename T> 
bool   erasuerl_is_erasure(const T& block);
template <typename T> 
char*  erasuerl_block_address(const T& block);
template <typename T> 
size_t erasuerl_block_size(const T& block);
template <typename T> 
T      erasuerl_new_block(size_t size);
template <typename T> 
void   erasuerl_realloc_block(T& block, size_t size);
template <typename T> 
void   erasuerl_free_block(T& block) {} 

bool decode(coding_state& state);
bool encode(coding_state& state);



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
std::shared_ptr<coding_state> make_encode_state(erasuerl_handle* h, T& data)
{
    
    size_t orig_size = erasuerl_block_size(data);
    size_t new_size = round_up_size(orig_size, h);
    size_t block_size = new_size / h->k;
    if (new_size != erasuerl_block_size(data))
        erasuerl_realloc_block<T>(data, new_size);
    auto state = std::make_shared<coding_state>(h, block_size, orig_size);
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
