#ifndef BIT_PACKER_H
#define BIT_PACKER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "global_parameters.h"

#define MAX_ARRAY_SIZE 1024
#define NUM_ARRAYS 10
#define MAX_CONSECUTIVE_ONES 24
#define ASCII_PACKET_SIZE 2048



/**
 * @brief Packer for one logical category of bits.
 */
typedef struct {
    uint8_t arrays[NUM_ARRAYS][MAX_ARRAY_SIZE];
    size_t bit_position;
    size_t consecutive_ones;
    size_t array_index;
} BitPacker;

/**
 * @brief Adds bits from a tone_bits struct to the respective packers.
 * 
 * @param input Struct with bits to be processed.
 */
void process_tone_bits(struct_tone_bits input);

/**
 * @brief Flushes content of a packer and prints it in ASCII hex.
 * 
 * @param packer Pointer to the BitPacker to flush.
 * @param label Label used for printing/debug.
 */
char* flush_and_convert_to_ascii(BitPacker* packer, const char* label);

char* add_bit(BitPacker* packer, uint8_t bit, const char* label);

/**
 * @brief Exposed packers (global state).
 */
extern BitPacker master_packer;
extern BitPacker slave_packer;
extern BitPacker config_packer;

extern char master_ascii_packet[ASCII_PACKET_SIZE];
extern char slave_ascii_packet[ASCII_PACKET_SIZE];
extern char config_ascii_packet[ASCII_PACKET_SIZE];

#ifdef __cplusplus
}
#endif

#endif // BIT_PACKER_H
