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
#include "erasuerl_handle.hpp"
#include "decode_context.hpp"
#include "encode_context.hpp"

static ErlNifResourceType* erasuerl_RESOURCE;

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

static ErlNifFunc nif_funcs[] =
{
    {"new", 4, erasuerl_new},
    {"encode", 2, erasuerl_encode},
    {"decode", 4, erasuerl_decode}
};

#define ATOM(Id, Value) { Id = enif_make_atom(env, Value); }

struct erasuerl_res
{
    erasuerl_handle* handle;
};

ERL_NIF_TERM erasuerl_new(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    int k, m, w, packetsize;
    if (!enif_get_int(env, argv[0], &k) ||
        !enif_get_int(env, argv[1], &m) ||
        !enif_get_int(env, argv[2], &w) ||
        !enif_get_int(env, argv[3], &packetsize)) 
        return enif_make_badarg(env);
    erasuerl_res *r = 
        (erasuerl_res *)enif_alloc_resource_compat(env, erasuerl_RESOURCE,
                                                   sizeof(erasuerl_res));
    memset(r, '\0', sizeof(erasuerl_res));
    r->handle = new erasuerl_handle(k, m, w, packetsize);
    ERL_NIF_TERM result = enif_make_resource(env, r);
    enif_release_resource_compat(env, r);
    return enif_make_tuple2(env, ATOM_OK, result);
}

ERL_NIF_TERM erasuerl_encode(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    erasuerl_res *r = nullptr;
    ErlNifBinary item;
    if (!enif_get_resource(env,argv[0],erasuerl_RESOURCE,(void**)&r) ||
        !enif_inspect_binary(env, argv[1], &item))
        return enif_make_badarg(env);
    struct iovec iov = {item.data, item.size};
    size_t newsize = round_up_size(item.size, r->handle);
    enif_realloc_binary(&item, newsize);    
    encode_context ctx(&iov, r->handle);
    ctx.encode();
    return enif_make_tuple3(env, ctx.metadata(env), ctx.get_data_blocks(env), ctx.code_blocks(env));
}


ERL_NIF_TERM parse_decode_option(ErlNifEnv* env, ERL_NIF_TERM item, decode_options& opts)
{
    int arity;
    const ERL_NIF_TERM* option;
    if (enif_get_tuple(env, item, &arity, &option))
    {
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

template <> 
bool erasuerl_is_erasure<ErlNifBinary>(const ErlNifBinary& item) 
{ 
    return item.size == 0;
}
template <> 
char *erasuerl_block_address<ErlNifBinary>(const ErlNifBinary& item) 
{ 
    return reinterpret_cast<char *>(item.data);
}

template <> 
size_t erasuerl_block_size<ErlNifBinary>(const ErlNifBinary& item) 
{ 
    return item.size;
}

template <>
ErlNifBinary erasuerl_new_block<ErlNifBinary>(size_t size)
{
    ErlNifBinary bin;
    assert(enif_alloc_binary(size, &bin));
    return bin;
}

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


void fill_binvec(ErlNifEnv *env, ERL_NIF_TERM list, std::vector<ErlNifBinary>& vec)
{
    ERL_NIF_TERM head, tail = list;
    while (enif_get_list_cell(env, tail, &head, &tail)) {
        ErlNifBinary bin;
        assert(enif_inspect_binary(env, head, &bin));
        vec.push_back(bin);
    }
}

ERL_NIF_TERM erasuerl_decode(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    erasuerl_res *r = nullptr;
    if (!enif_get_resource(env,argv[0],erasuerl_RESOURCE,(void**)&r) ||
        !enif_is_list(env, argv[1]) ||
        !enif_is_list(env, argv[2]) ||
        !enif_is_list(env, argv[3]))
        return enif_make_badarg(env);

    std::vector<ErlNifBinary> blocks;
    decode_options opts;
    fold(env, argv[1], parse_decode_option, opts);    
    fill_binvec(env, argv[2] /* data ptrs */, blocks);
    fill_binvec(env, argv[3] /* code ptrs */, blocks);
    
    decode_state* ds = make_decode_state(r->handle, blocks, opts.blocksize,
                                         opts.orig_size);
    ds->dump();

    if (!ds->num_erased())
        return enif_make_tuple2(env, ATOM_ERROR, ATOM_NO_ERASURES);
    
    if (!decode(*ds)) return ATOM_ERROR;

    ds->dump();

    // convert the decoded ErlNifBinaries back into a ERL_NIF_TERM list
    std::vector<ERL_NIF_TERM> result;
    std::size_t left = opts.orig_size;
    auto block_it = begin(blocks);
    do { 
        result.push_back(enif_make_binary(env, &(*block_it)));
        ++block_it;
    }
    while ((left -= opts.blocksize) >= opts.blocksize);
    if (left) 
        result.push_back(
            enif_make_sub_binary(
                env, enif_make_binary(env, &(*block_it)), 
                0, left));
    return enif_make_list_from_array(env, result.data(), r->handle->k);
}
   
static void erasuerl_resource_cleanup(ErlNifEnv* env, void* arg)
{
    erasuerl_res* r = (erasuerl_res*)arg;
    delete r->handle;
}

static int on_load(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info)
{
    ErlNifResourceFlags flags = (ErlNifResourceFlags)
        (ERL_NIF_RT_CREATE | ERL_NIF_RT_TAKEOVER);
    erasuerl_RESOURCE = enif_open_resource_type_compat(env, 
                                                    "erasuerl",
                                                    &erasuerl_resource_cleanup,
                                                    flags, 
                                                    NULL);
    if (!erasuerl_RESOURCE)
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
    return 0;
}

extern "C" 
{
    ERL_NIF_INIT(erasuerl, nif_funcs, &on_load, NULL, NULL, NULL);
}
