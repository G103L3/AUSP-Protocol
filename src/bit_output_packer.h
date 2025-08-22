#ifndef BIT_OUTPUT_PACKER_H
#define BIT_OUTPUT_PACKER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "global_parameters.h"

/* Maximum number of ASCII characters that can be packed. */
#define BOP_MAX_CHARS 2048
/* Total number of bit pairs generated from the characters. */
#define BOP_MAX_BITS (BOP_MAX_CHARS * 7)

/**
 * @brief Holds the frequency pairs generated from a text message.
 */
typedef struct {
    struct_out_tones *pairs; /**< Dynamically allocated array of frequency pairs. */
    size_t pair_count;       /**< Number of valid pairs in @c pairs. */

} BitOutputPacker;

/**
 * @brief Reset the internal state of the packer.
 */
void bit_output_packer_init(BitOutputPacker* packer);

/**
 * @brief Release any memory held by the packer.
 */
void bit_output_packer_free(BitOutputPacker* packer);

/**
 * @brief Convert @p text into frequency pairs for transmission.
 *
 * Each bit of the ASCII characters is converted into a pair of
 * frequencies via @c frequency_coder.
 *
 * @param packer Packer instance to fill.
 * @param text   Null-terminated string to convert.
 * @param role   Role passed to @c frequency_coder (0 master, 1 slave, 2 config).
 * @return Pointer to the internal array of frequency pairs or NULL on error.
 */
struct_out_tones* bit_output_packer_pack(BitOutputPacker* packer, const char* text, int role);

#ifdef __cplusplus
}
#endif

#endif /* BIT_OUTPUT_PACKER_H */

