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
#include "erasuerl.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

static ErlNifResourceType* erasuerl_RESOURCE;

struct erasuerl_handle {};


// Atoms (initialized in on_load)
static ERL_NIF_TERM ATOM_TRUE;
static ERL_NIF_TERM ATOM_FALSE;
static ERL_NIF_TERM ATOM_OK;
static ERL_NIF_TERM ATOM_ERROR;
static ERL_NIF_TERM ATOM_EMPTY;
static ERL_NIF_TERM ATOM_VALUE;

static ErlNifFunc nif_funcs[] =
{
    {"new", 0, erasuerl_new},
};

#define ATOM(Id, Value) { Id = enif_make_atom(env, Value); }


ERL_NIF_TERM erasuerl_new(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    erasuerl_handle *handle = 
        (erasuerl_handle *)enif_alloc_resource_compat(env, erasuerl_RESOURCE,
                                                   sizeof(erasuerl_handle));
    //memset(handle, '\0', sizeof(erasuerl_handle));
    //handle->q = new lockfree_queue<ErlNifBinary>(2000000);
    ERL_NIF_TERM result = enif_make_resource(env, handle);
    enif_release_resource_compat(env, handle);
    return enif_make_tuple2(env, ATOM_OK, result);
}

static void erasuerl_resource_cleanup(ErlNifEnv* env, void* arg)
{
    //erasuerl_handle* handle = (erasuerl_handle*)arg;
    //delete handle->q;
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
    return 0;
}

extern "C" {
    ERL_NIF_INIT(erasuerl, nif_funcs, &on_load, NULL, NULL, NULL);
}