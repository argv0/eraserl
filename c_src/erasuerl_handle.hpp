#ifndef __ERASUERL_HANDLE_HPP
#define __ERASUERL_HANDLE_HPP

extern "C" { 
#include "Jerasure/include/cauchy.h"
#include "Jerasure/include/jerasure.h"
}

struct decode_options 
{
    int orig_size = 0;
    int blocksize = 0;
    int packetsize = 0;
    int k = 0;
    int m = 0;
    int w = 0;
};

class erasuerl_handle 
{
public:
    /* ec params */
    int k; // = 9;
    int m; // = 4;
    int w;  // = 4;
    int packetsize;
    size_t num_blocks; 

    erasuerl_handle(int k, int m, int w, int packetsize)
        : k(k),
          m(m),
          w(w),
          packetsize(packetsize),
          num_blocks(k+m),
          matrix(cauchy_good_general_coding_matrix(k, m, w)),
          bitmatrix(jerasure_matrix_to_bitmatrix(k, m, w, matrix)),
          schedule(jerasure_smart_bitmatrix_to_schedule(k, m, w, bitmatrix))
    {
    }

    erasuerl_handle(const decode_options& opts)
        : erasuerl_handle(opts.k, opts.m, opts.w, opts.packetsize)
    {
    }

    ~erasuerl_handle()
    {
        if (matrix) free(matrix);
        if (bitmatrix) free(bitmatrix);
        if (schedule) free(schedule);
    }

public:
    void encode(size_t blocksize, char **data, char **coding)
    {
        jerasure_schedule_encode(k, m, w, schedule, data, coding, blocksize, 
                                 packetsize);
    }

    int decode(size_t blocksize, char **data, char **coding, int *erasures) 
    {
        return jerasure_schedule_decode_lazy(k, m, w, bitmatrix, erasures, 
                                             data, coding, blocksize,
                                             packetsize, 1);
    }
private:
    int *matrix = nullptr;
    int *bitmatrix = nullptr;
    int **schedule = nullptr;

};

#endif // include guard
