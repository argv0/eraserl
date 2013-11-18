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
#ifndef ERL_RS_NIFS_H_
#define ERL_RS_NIFS_H_

extern "C" {

#include "erl_nif_compat.h"

// exported NIF function
ERL_NIF_TERM erasuerl_new(ErlNifEnv*, int, const ERL_NIF_TERM[]);
ERL_NIF_TERM erasuerl_encode(ErlNifEnv*, int, const ERL_NIF_TERM[]);
ERL_NIF_TERM erasuerl_decode(ErlNifEnv*, int, const ERL_NIF_TERM[]);

// Atoms (initialized in on_load)
extern ERL_NIF_TERM ATOM_TRUE;
extern ERL_NIF_TERM ATOM_FALSE;
extern ERL_NIF_TERM ATOM_OK;
extern ERL_NIF_TERM ATOM_ERROR;
extern ERL_NIF_TERM ATOM_EMPTY;
extern ERL_NIF_TERM ATOM_VALUE;
extern ERL_NIF_TERM ATOM_K;
extern ERL_NIF_TERM ATOM_M;
extern ERL_NIF_TERM ATOM_W;
extern ERL_NIF_TERM ATOM_SIZE;
extern ERL_NIF_TERM ATOM_PACKETSIZE;



} // extern "C"



template <typename Acc> 
ERL_NIF_TERM fold(ErlNifEnv* env, ERL_NIF_TERM list,
                  ERL_NIF_TERM(*fun)(ErlNifEnv*, ERL_NIF_TERM, Acc&),
                  Acc& acc)
{
    ERL_NIF_TERM head, tail = list;
    while (enif_get_list_cell(env, tail, &head, &tail))
    {
        ERL_NIF_TERM result = fun(env, head, acc);
        if (result != ATOM_OK)
            return result;
    }
    return ATOM_OK;
}


#endif // include guard
