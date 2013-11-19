#ifndef __ERASUERL_HPP_
#define __ERASUERL_HPP_

#include <algorithm>

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
void   erasuerl_free_block(T& block) {} 

#endif // include guard
