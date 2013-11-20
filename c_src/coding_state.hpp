#ifndef __CODING_STATE_HPP_
#define __CODING_STATE_HPP_

#include <cstring>
#include <vector>
#include "erasuerl_handle.hpp"

struct coding_state {
    coding_state(erasuerl_handle *handle, std::size_t blocksize,
                 std::size_t orig_size = 0)
        : handle_(handle), data_(handle->num_blocks, nullptr),
          erasures_(handle->num_blocks, -1), erased_(handle->num_blocks, false),
          orig_size_(orig_size), blocksize_(blocksize) {}

    char **data_blocks() { return data_.data(); }

    char *data_block(size_t idx) const { return data_[idx]; }

    char **code_blocks() { return &(data_[handle_->k]); }

    std::size_t blocksize() const { return blocksize_; }

    int *erasures() { return erasures_.data(); }

    char *erased() { return erased_.data(); }

    size_t original_size() const { return orig_size_; }

    erasuerl_handle *handle() const { return handle_; }
    void data_block(size_t idx, char *block) { data_[idx] = block; }

    void erasure(size_t idx) {
        erasures_[num_erased_++] = idx;
        erased_[idx] = true;
    }

    std::size_t num_erased() const { return num_erased_; }

    void dump(const char *message = nullptr) const;

  private:
    erasuerl_handle *handle_ = nullptr;
    std::vector<char *> data_;
    std::vector<int> erasures_;
    std::vector<char> erased_;
    std::size_t orig_size_ = 0;
    std::size_t blocksize_ = 0;
    std::size_t num_erased_ = 0;
};

#endif // include guard
