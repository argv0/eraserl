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
