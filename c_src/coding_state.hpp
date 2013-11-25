// -------------------------------------------------------------------
//
// Copyright (c) 2007-2013 Basho Technologies, Inc. All Rights Reserved.
//
// This file is provided to you under the Apache License,
// Version 2.0 (the "License"); you may not use this file
// except in compliance with the License.  You may obtain
// a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.
//
// -------------------------------------------------------------------
#ifndef __CODING_STATE_HPP_
#define __CODING_STATE_HPP_

#include <cstring>
#include <vector>
#include "eraserl_handle.hpp"

struct coding_state {
    coding_state(eraserl_handle *handle, std::size_t blocksize,
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

    eraserl_handle *handle() const { return handle_; }
    void data_block(size_t idx, char *block) { data_[idx] = block; }

    void erasure(size_t idx) {
        erasures_[num_erased_++] = idx;
        erased_[idx] = true;
    }

    std::size_t num_erased() const { return num_erased_; }

    void dump(const char *message = nullptr) const;

  private:
    eraserl_handle *handle_ = nullptr;
    std::vector<char *> data_;
    std::vector<int> erasures_;
    std::vector<char> erased_;
    std::size_t orig_size_ = 0;
    std::size_t blocksize_ = 0;
    std::size_t num_erased_ = 0;
};

#endif // include guard
