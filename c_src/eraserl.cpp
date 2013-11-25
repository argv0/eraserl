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
#include "eraserl.hpp"

size_t round_up_size(size_t origsize, eraserl_handle *handle) {
    size_t newsize = origsize;
    size_t packetsize = handle->packetsize;
    size_t k = handle->k;
    size_t w = handle->w;
    if (origsize % (k * w * packetsize * sizeof(size_t)) != 0)
        while (newsize % (k * w * packetsize * sizeof(size_t)) != 0)
            newsize++;
    return newsize;
}

bool decode(coding_state &state) {
    if (state.handle()->decode(state.blocksize(), state.data_blocks(),
                               state.code_blocks(), state.erasures()))
        return false;
    return true;
}

bool encode(coding_state &state) {
    state.handle()->encode(state.blocksize(), state.data_blocks(),
                           state.code_blocks());
    return true;
}
