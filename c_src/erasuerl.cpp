// -------------------------------------------------------------------
//
// Copyright (c) 2007-2012 Basho Technologies, Inc. All Rights Reserved.
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
ERL_NIF_TERM ATOM_SIZE;
ERL_NIF_TERM ATOM_PACKETSIZE;

static ErlNifFunc nif_funcs[] =
{
    {"new", 4, erasuerl_new},
    {"encode", 2, erasuerl_encode},
    {"decode", 4, erasuerl_decode}
};

#define ATOM(Id, Value) { Id = enif_make_atom(env, Value); }

ERL_NIF_TERM erasuerl_new(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    int k, m, w, packetsize;
    if (!enif_get_int(env, argv[0], &k) ||
        !enif_get_int(env, argv[1], &m) ||
        !enif_get_int(env, argv[2], &w) ||
        !enif_get_int(env, argv[3], &packetsize)) 
        return enif_make_badarg(env);
    erasuerl_handle *h = 
        (erasuerl_handle *)enif_alloc_resource_compat(env, erasuerl_RESOURCE,
                                                      sizeof(erasuerl_handle));
    memset(h, '\0', sizeof(erasuerl_handle));
    h->k = k;
    h->m = m;
    h->w = w;
    h->packetsize = packetsize;
    h->matrix = cauchy_good_general_coding_matrix(h->k, h->m, h->w);
    h->bitmatrix = jerasure_matrix_to_bitmatrix(h->k, h->m, h->w, 
                                                h->matrix);
    h->schedule = jerasure_smart_bitmatrix_to_schedule(h->k, h->m, 
                                                       h->w, h->bitmatrix);
    ERL_NIF_TERM result = enif_make_resource(env, h);
    enif_release_resource_compat(env, h);
    return enif_make_tuple2(env, ATOM_OK, result);
}

ERL_NIF_TERM erasuerl_encode(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    erasuerl_handle *h;
    ErlNifBinary item;
    if (!enif_get_resource(env,argv[0],erasuerl_RESOURCE,(void**)&h) ||
        !enif_inspect_binary(env, argv[1], &item))
        return enif_make_badarg(env);

    encode_context ctx(env, &item, h);
    ctx.encode();
    return enif_make_tuple3(env, ctx.metadata(), ctx.get_data_blocks(), ctx.code_blocks());
}

ERL_NIF_TERM erasuerl_decode(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    erasuerl_handle *h;
    if (!enif_get_resource(env,argv[0],erasuerl_RESOURCE,(void**)&h) ||
        !enif_is_list(env, argv[1]) ||
        !enif_is_list(env, argv[2]) ||
        !enif_is_list(env, argv[3]))
        return enif_make_badarg(env);

    decode_context dstate(env, h, argv[1]);
    dstate.find_erasures(argv[2], argv[3]);
    if (dstate.decode()  == -1) 
        return ATOM_ERROR;
    return dstate.get_blocks();
}
   
static void erasuerl_resource_cleanup(ErlNifEnv* env, void* arg)
{
    //erasuerl_handle* h = (erasuerl_handle*)arg;
    //if (h->matrix) free(h->matrix);
    //if (h->bitmatrix) free(h->bitmatrix);
    //if (h->schedule) free(h->schedule);
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
    ATOM(ATOM_SIZE, "size");
    ATOM(ATOM_K, "k");
    ATOM(ATOM_M, "m");
    ATOM(ATOM_W, "w");
    ATOM(ATOM_PACKETSIZE, "packetsize");
    return 0;
}

extern "C" 
{
    ERL_NIF_INIT(erasuerl, nif_funcs, &on_load, NULL, NULL, NULL);
}
