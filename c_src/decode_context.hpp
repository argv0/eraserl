#ifndef __DECODE_CONTEXT_HPP_
#define __DECODE_CONTEXT_HPP_

#include <array>
#include <cstdint>
#include "erasuerl.h"
#include "erasuerl_handle.hpp"

struct decode_options 
{
    int size;
    int packetsize;
};

ERL_NIF_TERM parse_decode_option(ErlNifEnv* env, ERL_NIF_TERM item, decode_options& opts)
{
    int arity;
    const ERL_NIF_TERM* option;
    if (enif_get_tuple(env, item, &arity, &option))
    {
        if (option[0] == ATOM_SIZE)
        {
            enif_get_int(env, option[1], &opts.size);
        }
        else if (option[0] == ATOM_PACKETSIZE)
        {
            enif_get_int(env, option[1], &opts.packetsize);
        }
    }
    return ATOM_OK;
}

typedef std::array<char *, MAX_K> data_ptrs;
typedef std::array<char *, MAX_M> code_ptrs;
typedef std::array<int, MAX_M>    erasure_list;

struct decode_context
{
public:    
    decode_context(ErlNifEnv *env, erasuerl_handle* handle, ERL_NIF_TERM metadata)
        : env_(env),
          handle_(handle) 
    {
        fold(env, metadata, parse_decode_option, opts_);
    }

    ~decode_context() 
    {
        for (int i=0; i < num_erased; i++) 
            if (erasures[i] < handle_->k) 
                enif_free(data_[erasures[i]]);
            else 
                enif_free(coding_[erasures[i]]);        
    }

    decode_options options() const { return opts_; }

    std::size_t    blocksize() const { return blocksize_; }

    const data_ptrs & data() const  { return data_;  }

    data_ptrs &       data() { return data_; }

    const code_ptrs& coding() const  { return coding_; }

    code_ptrs&       coding() { return coding_;  }

    void find_erasures(ERL_NIF_TERM data_blocks, ERL_NIF_TERM coding_blocks)
    {
        ERL_NIF_TERM head, tail = data_blocks;        
        while (enif_get_list_cell(env_, tail, &head, &tail)) {
            visit_data(head);
            idx++;
        }
        idx = 0;
        tail = coding_blocks;
        while (enif_get_list_cell(env_, tail, &head, &tail)) { 
            visit_coding(head);        
            idx++;
        }
        for (int i=0; i < num_erased; i++) {
            if (erasures[i] < handle_->k) { 
                data_[erasures[i]] = reinterpret_cast<char *>(enif_alloc(blocksize_));
            }
            else { 
                coding_[erasures[i] - handle_->k] = reinterpret_cast<char *>(enif_alloc(blocksize_));
            }
        }
        erasures[num_erased] = -1;
    }
    
    void visit_coding(ERL_NIF_TERM item)
    {
        ErlNifBinary bin;
        if (!enif_inspect_binary(env_, item, &bin))  { 
            erasures[num_erased++] = handle_->k + idx;
        }
        else { 
            coding_[idx] = reinterpret_cast<char *>(bin.data);
        }
    }

    void visit_data(ERL_NIF_TERM item)
    {
        ErlNifBinary bin;
        if (!enif_inspect_binary(env_, item, &bin)) { 
            erasures[num_erased++] = idx;
        }
        else {
            blocksize_ = bin.size;
            data_[idx] = reinterpret_cast<char *>(bin.data);
        }
    }

    int decode() 
    {
        return handle_->decode(blocksize_, data_.data(), coding_.data(), erasures.data());
    }

    ERL_NIF_TERM get_blocks() 
    {
        std::array<ERL_NIF_TERM, MAX_K> result = {{0}};
        std::size_t total = 0;

        for (int i=0; i < handle_->k; i++) 
        {
            ErlNifBinary b;
            if (total + blocksize() <= options().size)
            {
                enif_alloc_binary(blocksize(), &b);
                memcpy(b.data, data_[i], blocksize());
                total += blocksize();
            }
            else 
            {
                enif_alloc_binary(options().size - total, &b);
                for (int j=0; j < blocksize(); j++)
                {
                    if (total++ < options().size) 
                    {
                        b.data[j] = data_[i][j];
                    }
                    else 
                    {
                        break;
                    }
                }
            }
            result[i] = enif_make_binary(env_, &b);
        }
        return enif_make_list_from_array(env_, result.data(), handle_->k);
    }
private:
    erasure_list erasures = {{0}};
    data_ptrs data_ = {{0}};
    code_ptrs coding_ = {{0}};
    std::size_t num_erased = 0;
    ErlNifEnv *env_ = nullptr;
    erasuerl_handle* handle_ = nullptr;
    int idx = 0;
    std::size_t blocksize_ = 0;
    decode_options opts_;
};




#endif // include guard
