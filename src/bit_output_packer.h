#ifndef BIT_OUTPUT_PACKER_H
#define BIT_OUTPUT_PACKER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define BOP_MAX_ARRAY_SIZE 1024
#define BOP_NUM_ARRAYS 10
#define BOP_TOTAL_BITS (BOP_MAX_ARRAY_SIZE * BOP_NUM_ARRAYS)

typedef struct {
    int arrays[BOP_NUM_ARRAYS][BOP_MAX_ARRAY_SIZE];
    size_t array_index;
    size_t bit_position;
    size_t total_bits;
} BitOutputPacker;

void bit_output_packer_init(BitOutputPacker* packer);
bool bit_output_packer_load(BitOutputPacker* packer, const char* text);
size_t bit_output_packer_flatten(BitOutputPacker* packer, int* out_bits, size_t max_len);

#ifdef __cplusplus
}
#endif

#endif // BIT_OUTPUT_PACKER_H
