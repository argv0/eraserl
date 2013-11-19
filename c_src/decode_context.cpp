#include "decode_context.hpp"

void decode_state::dump(const char *message) const
{
    static const char *dashline = "-------------------------------------------"
        "-----------------\n";
    printf("dumping decode_context: ");
    if (message)
        printf("(%s)\n", message);
    printf("%s\n", dashline);
    size_t coded_size = (blocksize_ * handle_->num_blocks);
    double bloat = (double(orig_size_*100) / double(coded_size)) + 100;
    printf("[ k=%d m=%d packetsize=%d\n  blocksize=%zu origsize=%zu codedsize=%zu\n"
           "  bloat=%.2f%% (%zu bytes)]\n",
           handle_->k, handle_->m, handle_->packetsize, blocksize_,
           orig_size_, coded_size, bloat, (coded_size - orig_size_));
    printf("%s\n" ,dashline);
    printf("%-5s%-10s%-20s%-15s%-10s\n", 
           "#", "type", "addr", "size", "was_erased");
    printf("%s", dashline);
    for (int i = 0; i < handle_->num_blocks; i++)  { 
        printf("%-5d%-10s%-20p%-15zu%-10s\n", 
               i, 
               (i >= handle_->k) ? "coding" : "data",
               data_[i],
               //dstate_->data[i].size, 
               blocksize_,
               (erased_[i] ? "true" : "false"));
        }
    printf("%s\n", dashline);
 }
