#include "decode_context.hpp"

void decode_context::dump_data(const char *message) const
{
    static const char *dashline = "-------------------------------------------"
        "-----------------\n";
    printf("dumping decode_context: ");
    if (message)
        printf("(%s)\n", message);
    printf("%s\n", dashline);
    size_t coded_size = (dstate_->blocksize * handle_->num_blocks);
    double bloat = (double(dstate_->orig_size*100) / double(coded_size)) + 100;
    printf("[ k=%d m=%d packetsize=%d\n  blocksize=%zu origsize=%zu codedsize=%zu\n"
           "  bloat=%.2f%% ]\n",
           handle_->k, handle_->m, handle_->packetsize, dstate_->blocksize,
           coded_size, dstate_->orig_size, bloat);
    printf("%s\n" ,dashline);
    printf("%-5s%-10s%-20s%-15s%-10s\n", 
           "#", "type", "addr", "size", "was_erased");
    printf("%s", dashline);
    for (int i = 0; i < handle_->num_blocks; i++)  { 
        printf("%-5d%-10s%-20p%-15zu%-10s\n", 
               i, 
               (i >= handle_->k) ? "coding" : "data",
               dstate_->data[i],
               //dstate_->data[i].size, 
               dstate_->blocksize,
               (dstate_->erased[i] ? "true" : "false"));
        }
    printf("%s\n", dashline);
         }
