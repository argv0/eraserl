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
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include "eraserl.hpp"
#include "eraserl_nifs.h"

typedef std::vector<ErlNifBinary> nif_bin_vector;

static ErlNifResourceType *eraserl_RESOURCE;

using std::size_t;

ERL_NIF_TERM ATOM_TRUE;
ERL_NIF_TERM ATOM_FALSE;
ERL_NIF_TERM ATOM_OK;
ERL_NIF_TERM ATOM_ERROR;
ERL_NIF_TERM ATOM_EMPTY;
ERL_NIF_TERM ATOM_VALUE;
ERL_NIF_TERM ATOM_K;
ERL_NIF_TERM ATOM_M;
ERL_NIF_TERM ATOM_W;
ERL_NIF_TERM ATOM_ORIG_SIZE;
ERL_NIF_TERM ATOM_PACKETSIZE;
ERL_NIF_TERM ATOM_BLOCKSIZE;
ERL_NIF_TERM ATOM_NO_ERASURES;
ERL_NIF_TERM ATOM_UNRECOVERABLE;
ERL_NIF_TERM ATOM_XOR_BYTES;
ERL_NIF_TERM ATOM_GF_BYTES;
ERL_NIF_TERM ATOM_COPIED_BYTES;

static ErlNifFunc nif_funcs[] = { { "new", 4, eraserl_new },
                                  { "encode", 2, eraserl_encode },
                                  { "decode", 4, eraserl_decode },
                                  { "stats", 0, eraserl_stats } };

struct eraserl_res {
    eraserl_handle *handle;
};

ERL_NIF_TERM eraserl_new(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    int k, m, w, packetsize;
    if (!enif_get_int(env, argv[0], &k) || !enif_get_int(env, argv[1], &m) ||
        !enif_get_int(env, argv[2], &w) ||
        !enif_get_int(env, argv[3], &packetsize))
        return enif_make_badarg(env);
    eraserl_res *r = (eraserl_res *)enif_alloc_resource_compat(
        env, eraserl_RESOURCE, sizeof(eraserl_res));
    memset(r, '\0', sizeof(eraserl_res));
    r->handle = new eraserl_handle(k, m, w, packetsize);
    ERL_NIF_TERM result = enif_make_resource(env, r);
    enif_release_resource_compat(env, r);
    return enif_make_tuple2(env, ATOM_OK, result);
}

ERL_NIF_TERM eraserl_stats(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    double stats[3];
    jerasure_get_stats(&stats[0]);
    return enif_make_tuple3(env, 
                            enif_make_tuple2(env, ATOM_XOR_BYTES, enif_make_uint64(env, stats[0])),
                            enif_make_tuple2(env, ATOM_GF_BYTES, enif_make_uint64(env, stats[1])),
                            enif_make_tuple2(env, ATOM_COPIED_BYTES, enif_make_uint64(env, stats[2])));
}

ERL_NIF_TERM parse_decode_option(ErlNifEnv *env, ERL_NIF_TERM item,
                                 decode_options &opts) {
    int arity;
    const ERL_NIF_TERM *option;
    if (enif_get_tuple(env, item, &arity, &option)) {
        if (option[0] == ATOM_ORIG_SIZE)
            enif_get_int(env, option[1], &opts.orig_size);
        else if (option[0] == ATOM_PACKETSIZE)
            enif_get_int(env, option[1], &opts.packetsize);
        else if (option[0] == ATOM_BLOCKSIZE)
            enif_get_int(env, option[1], &opts.blocksize);
        else if (option[0] == ATOM_K)
            enif_get_int(env, option[1], &opts.k);
        else if (option[0] == ATOM_M)
            enif_get_int(env, option[1], &opts.m);
        else if (option[0] == ATOM_W)
            enif_get_int(env, option[1], &opts.w);
    }
    return ATOM_OK;
}

template <> bool eraserl_is_erasure<ErlNifBinary>(const ErlNifBinary &item) {
    return item.size == 0;
}
template <>
char *eraserl_block_address<ErlNifBinary>(const ErlNifBinary &item) {
    return reinterpret_cast<char *>(item.data);
}

template <> size_t eraserl_block_size<ErlNifBinary>(const ErlNifBinary &item) {
    return item.size;
}

template <> ErlNifBinary eraserl_new_block<ErlNifBinary>(size_t size) {
    ErlNifBinary bin;
    assert(enif_alloc_binary(size, &bin));
    return bin;
}

template <>
void eraserl_realloc_block<ErlNifBinary>(ErlNifBinary &item, size_t newsize) {
    enif_realloc_binary(&item, newsize);
}

std::vector<ErlNifBinary> to_binvec(ErlNifEnv *env,
                                    ERL_NIF_TERM list) {
    std::vector<ErlNifBinary> binvec;
    ERL_NIF_TERM head, tail = list;
    while (enif_get_list_cell(env, tail, &head, &tail)) {
        ErlNifBinary bin;
        assert(enif_inspect_binary(env, head, &bin));
        binvec.push_back(bin);
    }
    return binvec;
}

ERL_NIF_TERM to_termlist(ErlNifEnv *env, const nif_bin_vector &vec,
                         std::size_t blocksize, std::size_t size) {
    std::vector<ERL_NIF_TERM> result;
    std::size_t left = size;
    auto it = begin(vec);
    do
        result.push_back(
            enif_make_binary(env, const_cast<ErlNifBinary *>(&*(it++))));
    while ((left -= blocksize) >= blocksize);
    if (left)
        result.push_back(enif_make_sub_binary(
            env, enif_make_binary(env, const_cast<ErlNifBinary *>(&*(it))), 0,
            left));
    return enif_make_list_from_array(env, result.data(), result.size());
}

ERL_NIF_TERM eraserl_decode(ErlNifEnv *env, int argc,
                             const ERL_NIF_TERM argv[]) {
    eraserl_res *r = nullptr;
    if (!enif_get_resource(env, argv[0], eraserl_RESOURCE, (void **)&r) ||
        !enif_is_list(env, argv[1]) || !enif_is_list(env, argv[2]) ||
        !enif_is_list(env, argv[3]))
        return enif_make_badarg(env);

    decode_options opts;
    fold(env, argv[1], parse_decode_option, opts);
    nif_bin_vector data_blocks(to_binvec(env, argv[2]));
    nif_bin_vector code_blocks(to_binvec(env, argv[3]));
    data_blocks.insert(end(data_blocks), begin(code_blocks), end(code_blocks));
    auto state = make_decode_state(r->handle, data_blocks, opts.blocksize,
                                   opts.orig_size);
    if (state->num_erased() > r->handle->m)
        return enif_make_tuple2(env, ATOM_ERROR, ATOM_UNRECOVERABLE);
    state->dump("decode");
    if (!state->num_erased())
        return enif_make_tuple2(env, ATOM_ERROR, ATOM_NO_ERASURES);
    if (!decode(*state))
        return ATOM_ERROR;
    return to_termlist(env, data_blocks, opts.blocksize,
                       opts.orig_size);
}

ERL_NIF_TERM eraserl_encode(ErlNifEnv *env, int argc,
                             const ERL_NIF_TERM argv[]) {
    eraserl_res *r = nullptr;
    ErlNifBinary item;
    if (!enif_get_resource(env, argv[0], eraserl_RESOURCE, (void **)&r) ||
        !enif_inspect_binary(env, argv[1], &item))
        return enif_make_badarg(env);

    auto state = make_encode_state(r->handle, item);
    encode(*state);
    state->dump("encode");
    nif_bin_vector data_blocks, code_blocks;
    for (size_t i = 0; i < r->handle->k; i++)
        data_blocks.push_back(
            { state->blocksize(),
              reinterpret_cast<unsigned char *>(state->data_block(i)) });
    for (size_t i = 0; i < r->handle->m; i++)
        code_blocks.push_back(
            { state->blocksize(), reinterpret_cast<unsigned char *>(
                                      state->data_block(i + r->handle->k)) });

    ERL_NIF_TERM metadata = enif_make_list6(
        env, enif_make_tuple2(env, ATOM_ORIG_SIZE,
                              enif_make_int(env, state->original_size())),
        enif_make_tuple2(env, ATOM_K, enif_make_int(env, r->handle->k)),
        enif_make_tuple2(env, ATOM_M, enif_make_int(env, r->handle->m)),
        enif_make_tuple2(env, ATOM_W, enif_make_int(env, r->handle->w)),
        enif_make_tuple2(env, ATOM_PACKETSIZE,
                         enif_make_int(env, r->handle->packetsize)),
        enif_make_tuple2(env, ATOM_BLOCKSIZE,
                         enif_make_int(env, state->blocksize())));
    return enif_make_tuple3(
        env, metadata,
        to_termlist(env, data_blocks, state->blocksize(),
                    state->blocksize() * r->handle->k),
        to_termlist(env, code_blocks, state->blocksize(),
                    state->blocksize() * (r->handle->m)));
}

static void eraserl_resource_cleanup(ErlNifEnv *env, void *arg) {
    eraserl_res *r = (eraserl_res *)arg;
    delete r->handle;
}

static int on_load(ErlNifEnv *env, void **priv_data, ERL_NIF_TERM load_info) {
    ErlNifResourceFlags flags =
        (ErlNifResourceFlags)(ERL_NIF_RT_CREATE | ERL_NIF_RT_TAKEOVER);
    eraserl_RESOURCE = enif_open_resource_type_compat(
        env, "eraserl", &eraserl_resource_cleanup, flags, NULL);
    if (!eraserl_RESOURCE)
        return 0;
    // Initialize common atoms
    ATOM(ATOM_OK, "ok");
    ATOM(ATOM_ERROR, "error");
    ATOM(ATOM_TRUE, "true");
    ATOM(ATOM_FALSE, "false");
    ATOM(ATOM_EMPTY, "empty");
    ATOM(ATOM_VALUE, "value");
    ATOM(ATOM_ORIG_SIZE, "orig_size");
    ATOM(ATOM_K, "k");
    ATOM(ATOM_M, "m");
    ATOM(ATOM_W, "w");
    ATOM(ATOM_PACKETSIZE, "packetsize");
    ATOM(ATOM_BLOCKSIZE, "blocksize");
    ATOM(ATOM_NO_ERASURES, "no_erasures");
    ATOM(ATOM_UNRECOVERABLE, "unrecoverable");
    ATOM(ATOM_XOR_BYTES, "xor_bytes");
    ATOM(ATOM_GF_BYTES, "gf_bytes");
    ATOM(ATOM_COPIED_BYTES, "copied_bytes");

    return 0;
}

extern "C" { ERL_NIF_INIT(eraserl, nif_funcs, &on_load, NULL, NULL, NULL); }
