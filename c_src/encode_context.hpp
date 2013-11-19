#ifndef __ENCODE_CONTEXT_HPP_
#define __ENCODE_CONTEXT_HPP_

#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <sys/uio.h>
#include "erasuerl_handle.hpp"
#include "erasuerl.hpp"
#include "erasuerl_nifs.h"

size_t round_up_size(size_t origsize, erasuerl_handle *handle) 
{
    size_t newsize = origsize;
    size_t packetsize = handle->packetsize;
    size_t k = handle->k;
    size_t w = handle->w;
    if (origsize % (k * w * packetsize * sizeof(size_t)) != 0)
        while (newsize % (k * w * packetsize * sizeof(size_t)) != 0)
            newsize++;
    return newsize;
}

class encode_context {
public:
    encode_context(iovec* iov, erasuerl_handle *handle) :
        iov_(iov), 
        h(handle),
        orig_size(iov->iov_len),
        newsize(round_up_size(iov->iov_len, handle)),
        blocksize(newsize/handle->k),
        coding(handle->m),
        data(handle->k)
    {
        for (int i=0;i<handle->k;i++) 
            data[i] = reinterpret_cast<char *>((char *)iov_->iov_base+(i*blocksize));
        for (int i=0;i<handle->m;i++) 
            coding[i] = new char[blocksize];
    }

    ~encode_context() 
    {
        for (int i=0;i<h->m; i++)
            delete[] coding[i];
    }
public:
    void encode() 
    {
        h->encode(blocksize, data.data(), coding.data());
    }
    
    ERL_NIF_TERM 
    metadata(ErlNifEnv* env) const 
    { 
        return enif_make_list6(env,
                   enif_make_tuple2(env, ATOM_ORIG_SIZE, enif_make_int(env, orig_size)),
                   enif_make_tuple2(env, ATOM_K, enif_make_int(env, h->k)),
                   enif_make_tuple2(env, ATOM_M, enif_make_int(env, h->m)),
                   enif_make_tuple2(env, ATOM_W, enif_make_int(env, h->w)),
                   enif_make_tuple2(env, ATOM_PACKETSIZE, enif_make_int(env, 
                                                                        h->packetsize)),
                   enif_make_tuple2(env, ATOM_BLOCKSIZE, enif_make_int(env,
                                                                       blocksize)));
                                                                                   
                                    
        
    }

    ERL_NIF_TERM 
    get_data_blocks(ErlNifEnv *env) const 
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

    ERL_NIF_TERM 
    code_blocks(ErlNifEnv *env) const 
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
    iovec *iov_ = nullptr;
    erasuerl_handle* h;
    std::size_t orig_size = 0;
    std::size_t newsize = 0;
    std::size_t blocksize = 0;
    std::vector<char *> coding;
    std::vector<char *> data;
};

#endif // include guard
