#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MAX_ARRAY_SIZE 1024
#define NUM_ARRAYS 5
#define TOTAL_BITS (MAX_ARRAY_SIZE * NUM_ARRAYS)

/**
 * @brief Structure to manage bit input packing across multiple arrays.
 */
typedef struct {
    uint8_t arrays[NUM_ARRAYS][MAX_ARRAY_SIZE]; ///< Storage arrays for bits.
    size_t bit_position;                       ///< Current bit position across arrays.
} BitInputPacker;

BitInputPacker packer = {0};

/**
 * @brief Adds a single bit to the packer.
 * 
 * @param bit The bit to add (0 or 1).
 * @return true if the bit was added successfully, false if all arrays are full.
 */
bool add_bit(uint8_t bit) {
    if (bit > 1) {
        return false; // Invalid bit value.
    }

    size_t byte_index = packer.bit_position / 8;
    size_t bit_index = packer.bit_position % 8;
    size_t array_index = byte_index / MAX_ARRAY_SIZE;
    size_t local_byte_index = byte_index % MAX_ARRAY_SIZE;

    if (array_index >= NUM_ARRAYS) {
        // All arrays are full, trigger automatic flush.
        printf("Error: All arrays are full. Flushing automatically.\n");
        flush_and_convert_to_ascii();
        return false;
    }

    if (bit == 1) {
        packer.arrays[array_index][local_byte_index] |= (1 << bit_index);
    } else {
        packer.arrays[array_index][local_byte_index] &= ~(1 << bit_index);
    }

    packer.bit_position++;
    return true;
}

/**
 * @brief Flushes all arrays and converts their content to ASCII.
 */
void flush_and_convert_to_ascii() {
    size_t total_bytes = (packer.bit_position + 7) / 8;
    size_t array_index = 0;
    size_t byte_index = 0;

    printf("Flushing content:\n");
    for (size_t i = 0; i < total_bytes; i++) {
        if (byte_index >= MAX_ARRAY_SIZE) {
            byte_index = 0;
            array_index++;
        }
        printf("%02X ", packer.arrays[array_index][byte_index]);
        byte_index++;
    }
    printf("\n");

    // Reset the packer.
    memset(packer.arrays, 0, sizeof(packer.arrays));
    packer.bit_position = 0;
}

/**
 * @brief Example usage of the BitInputPacker.
 */
int main() {
    // Example: Add some bits.
    for (int i = 0; i < TOTAL_BITS; i++) {
        if (!add_bit(i % 2)) {
            break;
        }
    }

    // Manually flush and convert to ASCII.
    flush_and_convert_to_ascii();

    return 0;
}

#ifdef __cplusplus
}
#endif