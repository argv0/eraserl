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
#include "coding_state.hpp"

void coding_state::dump(const char *message) const {
    static const char *dashline =
        "------------------------------------------------------------\n";
    static const char *params_fmt =
        "[ k=%d m=%d packetsize=%d\n blocksize=%zu origsize=%zu "
        "codedsize=%zu\n bloat=%.2f%% (%zu bytes)]\n";
    static const char *header_fmt = "%-5s%-10s%-20s%-15s%-10s\n";
    static const char *table_fmt = "%-5d%-10s%-20p%-15zu%-10s\n";
    printf("dumping coding_state: ");
    if (message)
        printf("(%s)", message);
    printf("\n%s", dashline);
    size_t coded_size = (blocksize_ * handle_->num_blocks);
    double bloat = (double(orig_size_ * 100) / double(coded_size)) + 100;
    printf(params_fmt, handle_->k, handle_->m, handle_->packetsize, blocksize_,
           orig_size_, coded_size, bloat, (coded_size - orig_size_));
    printf("%s", dashline);
    printf(header_fmt, "#", "type", "addr", "size", "was_erased");
    printf("%s", dashline);
    for (int i = 0; i < handle_->num_blocks; i++) {
        printf(table_fmt, i, (i >= handle_->k) ? "coding" : "data", data_[i],
               // dstate_->data[i].size,
               blocksize_, (erased_[i] ? "true" : "false"));
    }
    printf("%s\n", dashline);
}
