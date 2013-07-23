#include "erl_nif_compat.h"

extern "C" { 
#include "Jerasure/include/cauchy.h"
#include "Jerasure/include/jerasure.h"
}


struct encode_context {
    encode_context(ErlNifEnv *env, ErlNifBinary* bin, erasuerl_handle *handle) :
        env(env),
        item(bin), 
        h(handle), 
        size(item->size), 
        newsize(round_up_size(item->size, handle->k, handle->w, handle->packetsize)),
        blocksize(newsize/handle->k), 
        data(new char*[handle->k]), 
        coding(new char*[handle->m])
    {
        enif_realloc_binary(item, newsize);
        for (int i=0;i<h->k;i++)
            data[i] = (char *)item->data+(i*blocksize);
        for (int i=0;i<h->m;i++)
            coding[i] = new char[blocksize];
    }
    ~encode_context() 
    {
        delete[] data;
        for (int i=0;i<h->m;i++)
            delete coding[i];
        delete[] coding;
    }

    void encode() 
    {
        jerasure_schedule_encode(h->k, h->m, h->w, h->schedule, 
                                 data, coding, blocksize, h->packetsize);
    }
    
    ERL_NIF_TERM metadata() const 
    { 
        return enif_make_list5(env,
                   enif_make_tuple2(env, ATOM_SIZE, enif_make_int(env, size)),
                   enif_make_tuple2(env, ATOM_K, enif_make_int(env, h->k)),
                   enif_make_tuple2(env, ATOM_M, enif_make_int(env, h->m)),
                   enif_make_tuple2(env, ATOM_W, enif_make_int(env, h->w)),
                   enif_make_tuple2(env, ATOM_PACKETSIZE, enif_make_int(env, 
                                                                        h->packetsize)));
    }

    ERL_NIF_TERM data_blocks() const 
    {
        ERL_NIF_TERM ret[h->k];
        for (int i=0; i < h->k; i++)
        {
            ErlNifBinary b;
            enif_alloc_binary(blocksize, &b);
            memcpy(b.data, data[i], blocksize);
            ret[i] = enif_make_binary(env, &b);
        }
        return enif_make_list_from_array(env, ret, h->k);
    }

    ERL_NIF_TERM code_blocks() const 
    { 
        ERL_NIF_TERM ret[h->m];
        for (int i=0; i < h->m; i++)
        {
            ErlNifBinary b;
            enif_alloc_binary(blocksize, &b);
            memcpy(b.data, coding[i], blocksize);
            ret[i] = enif_make_binary(env, &b);
        }
        return enif_make_list_from_array(env, ret, h->m);
    }
private:
    ErlNifEnv *env;
    ErlNifBinary *item;
    erasuerl_handle* h;
    size_t size;
    size_t newsize;
    size_t blocksize;
    char **data;
    char **coding;
    
};
