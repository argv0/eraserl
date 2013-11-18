#ifndef __ERASUERL_HANDLE_HPP
#define __ERASUERL_HANDLE_HPP

extern "C" { 
#include "Jerasure/include/cauchy.h"
#include "Jerasure/include/jerasure.h"
}

struct erasuerl_handle 
{
    int *matrix;
    int *bitmatrix;
    int **schedule;  
    int k; // = 9;
    int m; // = 4;
    int w;  // = 4;
    int packetsize;
    size_t num_blocks; 

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
};

#endif // include guard
