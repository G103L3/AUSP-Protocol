#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "serial_bridge.h"
#include "bit_input_packer.h"
#include "global_parameters.h"

#define TOTAL_BITS (MAX_ARRAY_SIZE * NUM_ARRAYS * 8)

BitPacker master_packer = {0};
BitPacker slave_packer = {0};
BitPacker config_packer = {0};

char master_ascii_packet[ASCII_PACKET_SIZE] = {0};
char slave_ascii_packet[ASCII_PACKET_SIZE] = {0};
char config_ascii_packet[ASCII_PACKET_SIZE] = {0};

static char* buffer_for_packer(BitPacker* packer) {
    if (packer == &master_packer) return master_ascii_packet;
    if (packer == &slave_packer) return slave_ascii_packet;
    return config_ascii_packet;
}

char* add_bit(BitPacker* packer, uint8_t bit, const char* label) {
    char* out = buffer_for_packer(packer);
    out[0] = '\0';
    if (bit > 1) return out;

    size_t array_index_ = packer->array_index;
    size_t bit_index = packer->bit_position;

    serial_write_formatted("Info: Array_index: %d\n", array_index_);

    if (bit == 1) {
        packer->arrays[array_index_][bit_index] = 1;
        packer->consecutive_ones++;
        if (packer->consecutive_ones >= MAX_CONSECUTIVE_ONES) {
            printf("%s: %d consecutive 1s. Auto flush.\n", label, MAX_CONSECUTIVE_ONES);
            return flush_and_convert_to_ascii(packer, label);
        }
    } else {
        packer->arrays[array_index_][bit_index] = 0;
        packer->consecutive_ones = 0;
    }

    packer->bit_position++;
    if (packer->bit_position >= MAX_ARRAY_SIZE) {
        packer->bit_position = 0;
        packer->array_index++;
    }
    if (packer->array_index >= NUM_ARRAYS) {
        printf("Warning: %s arrays full. Auto flush.\n", label);
        return flush_and_convert_to_ascii(packer, label);
    }
    return out;
}

char* flush_and_convert_to_ascii(BitPacker* packer, const char* label) {
    size_t total_bits = 0;
    if (packer->bit_position == 0){
        total_bits = packer->array_index * MAX_ARRAY_SIZE;
    } else if (packer->array_index > 0){
        total_bits = packer->array_index * packer->bit_position;
    } else if (packer->array_index == 0 && packer->bit_position > 0){
        total_bits = packer->bit_position;
    }
    size_t total_bytes = total_bits / 8;

    serial_write_formatted("Info: Flushing %s packer with %zu bits (%zu bytes)\n",
                           label, packer->bit_position, total_bytes);

    char* buffer = buffer_for_packer(packer);
    memset(buffer, 0, ASCII_PACKET_SIZE);
    size_t array_index = 0;
    size_t byte_index = 0;
    size_t buf_idx = 0;
    for (size_t i = 0; i < total_bytes && buf_idx < ASCII_PACKET_SIZE - 1; i++) {
        if (byte_index + 8 > MAX_ARRAY_SIZE) {
            byte_index = 0;
            array_index++;
        }
        char bits[9];
        for (size_t j = 0; j < 8; j++) {
            bits[j] = packer->arrays[array_index][byte_index + j] ? '1' : '0';
        }
        bits[8] = '\0';

        unsigned long value = strtoul(bits, NULL, 2);
        buffer[buf_idx++] = (char)value;

        byte_index += 8;
        if (byte_index >= MAX_ARRAY_SIZE) {
            byte_index = 0;
            array_index++;
        }
    }
    buffer[buf_idx] = '\0';

    packer->array_index = 0;
    packer->bit_position = 0;
    memset(packer->arrays, 0, sizeof(packer->arrays));
    packer->consecutive_ones = 0;
    return buffer;
}

void process_tone_bits(struct_tone_bits input) {
    add_bit(&master_packer, input.master, "MASTER");
    add_bit(&slave_packer, input.slave, "SLAVE");
    add_bit(&config_packer, input.configuration, "CONFIG");
}

#ifdef __cplusplus
}
#endif

