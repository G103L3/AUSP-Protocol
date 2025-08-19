#ifndef BIT_OUTPUT_PACKER_H
#define BIT_OUTPUT_PACKER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "global_parameters.h"
#include "bit_freq_codec.h"

#define BOP_MAX_PAIRS 2048

typedef struct {
    struct_out_tones pairs[BOP_MAX_PAIRS];
    size_t pair_count;
} BitOutputPacker;

void bit_output_packer_init(BitOutputPacker* packer);
struct_out_tones* bit_output_packer_pack(BitOutputPacker* packer, const char* text, int role);

#ifdef __cplusplus
}
#endif

#endif // BIT_OUTPUT_PACKER_H

