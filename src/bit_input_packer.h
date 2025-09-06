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
#define MAX_CONSECUTIVE_ZEROS 21
#define ASCII_ARRAY_SIZE 256
#define ASCII_NUM_ARRAYS 8
#define ASCII_PACKET_SIZE (ASCII_ARRAY_SIZE * ASCII_NUM_ARRAYS)
#define CODE_ARRAY_SIZE 256
#define CODE_NUM_ARRAYS 8
#define CODE_PACKET_SIZE (CODE_ARRAY_SIZE * CODE_NUM_ARRAYS)




/**
 * @brief Packer for one logical category of bits.
 */
typedef struct {
    uint8_t arrays[NUM_ARRAYS][MAX_ARRAY_SIZE];
    size_t bit_position;
    size_t array_index;
    size_t ascii_char_index;
    size_t ascii_array_index;
    size_t code_index;
    size_t code_array_index;
    int codes_temp[CODE_PACKET_SIZE];
    size_t codes_temp_len;
} BitPacker;

/**
 * @brief Adds bits from a tone_bits struct to the respective packers.
 * 
 * @param input Struct with bits to be processed.
 */
bool process_tone_bits(struct_tone_bits input);

/**
 * @brief Flushes content of a packer and prints it in ASCII hex.
 * 
 * @param packer Pointer to the BitPacker to flush.
 * @param label Label used for printing/debug.
 */
bool flush_and_convert_to_ascii(BitPacker* packer, const char* label);

bool add_bit(BitPacker* packer, uint8_t signal_code, const char* label);

/**
 * @brief Exposed packers (global state).
 */
extern BitPacker master_packer;
extern BitPacker slave_packer;
extern BitPacker config_packer;

extern char master_ascii_arrays[ASCII_NUM_ARRAYS][ASCII_ARRAY_SIZE];
extern char slave_ascii_arrays[ASCII_NUM_ARRAYS][ASCII_ARRAY_SIZE];
extern char config_ascii_arrays[ASCII_NUM_ARRAYS][ASCII_ARRAY_SIZE];
extern int master_codes_arrays[CODE_NUM_ARRAYS][CODE_ARRAY_SIZE];
extern int slave_codes_arrays[CODE_NUM_ARRAYS][CODE_ARRAY_SIZE];
extern int config_codes_arrays[CODE_NUM_ARRAYS][CODE_ARRAY_SIZE];

size_t bit_input_packer_get_codes_sections(BitPacker* packer);
bool bit_input_packer_get_codes_section(BitPacker* packer, size_t section_index, int* out_codes, size_t* out_len);
bool bit_input_packer_remove_codes_section(BitPacker* packer, size_t section_index);




#ifdef __cplusplus
}
#endif

#endif // BIT_PACKER_H
