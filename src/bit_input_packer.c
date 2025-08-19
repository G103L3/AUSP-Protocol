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

#define MAX_ARRAY_SIZE 1024
#define NUM_ARRAYS 10
#define TOTAL_BITS (MAX_ARRAY_SIZE * NUM_ARRAYS * 8)
#define MAX_CONSECUTIVE_ONES 24




/**
 * Three independent packers
 */
BitPacker master_packer = {0};
BitPacker slave_packer = {0};
BitPacker config_packer = {0};

/**
 * @brief Helper to add bit to the correct packer
 */
bool add_bit(BitPacker* packer, uint8_t bit, const char* label) {
    bool ret = true;
    if (bit > 1) return false;
    if (bit < 0) return false;

    size_t array_index_ = packer->array_index;
    size_t bit_index = packer->bit_position;

    serial_write_formatted("Info: Array_index: %d\n", array_index_);

    if (bit == 1) {
        packer->arrays[array_index_][bit_index] = 1;
        packer->consecutive_ones++;
        if (packer->consecutive_ones >= MAX_CONSECUTIVE_ONES) {
            printf("%s: %d consecutive 1s. Auto flush.\n", label, MAX_CONSECUTIVE_ONES);
            flush_and_convert_to_ascii(packer, label);
            ret = false;
        }
    } else {
        packer->arrays[array_index_][bit_index] = 0;
        packer->consecutive_ones = 0;
    }

    packer->bit_position++;
    if(packer->bit_position >= MAX_ARRAY_SIZE) {
        packer->bit_position = 0;
        packer->array_index++;
    }
    if (packer->array_index >= NUM_ARRAYS) {
        printf("Warning: %s arrays full. Auto flush.\n", label);
        flush_and_convert_to_ascii(packer, label);
        ret = false;
    }
    return ret;
}

/**
 * @brief Flushes and prints content for one category
 */
void flush_and_convert_to_ascii(BitPacker* packer, const char* label) {
    size_t total_bytes;
    size_t array_index = 0;
    size_t byte_index = 0;
    size_t bit_index = 0;
    size_t total_bits = 0;

    printf("packer->bit_position: %zu  packer->array_index: %zu\n", packer->bit_position, packer->array_index);
    //Set total_bytes
    if (packer->bit_position == 0){
        total_bits = packer->array_index * MAX_ARRAY_SIZE;
        printf("Info: total_bits: %zu\n", total_bits);
    }else if(packer-> array_index > 0){
        total_bits = packer->array_index * packer->bit_position;
    }else if(packer->array_index == 0 && packer->bit_position > 0){
        total_bits = packer->bit_position;
    }
    total_bytes = total_bits / 8;

    serial_write_formatted("Info: Flushing %s packer with %zu bits (%zu bytes)\n", label, packer->bit_position, total_bytes);
    printf("[%s] Flushing content: in bytes\n", label);
    for (size_t i = 0; i < total_bytes; i++) {
        serial_write_formatted("Info: Processing byte %zu\n", i);
    
        // Se servono 8 bit e non ci stanno, passa all'array successivo
        if (byte_index + 8 > MAX_ARRAY_SIZE) {
            byte_index = 0;
            array_index++;
        }
    
        // Verifica di non uscire dall'ultimo array disponibile
        // (aggiungi un controllo reale in base alla tua struttura dati)
        // if (array_index >= NUM_ARRAYS) break/return/errore;
    
        char bits[9];  // 8 bit + terminatore
        for (size_t j = 0; j < 8; j++) {
            bits[j] = packer->arrays[array_index][byte_index + j] ? '1' : '0';
        }
        bits[8] = '\0';
    
        char *endptr;
        unsigned long value = strtoul(bits, &endptr, 2);
        // opzionalmente controlla che endptr punti al terminatore
        // if (*endptr != '\0') { /* gestione errore */ }
    
        printf("%c\n", (char)value);
    
        byte_index += 8; // avanzamento di un byte (8 bit)
        if (byte_index >= MAX_ARRAY_SIZE) {
            byte_index = 0;
            array_index++;
        }
    }
    

    /*printf("[%s] Flushing content: in bits\n", label);
    for (size_t i = 0; i < NUM_ARRAYS; i++) {
        for(size_t j = 0; j < MAX_ARRAY_SIZE; j++) {
            printf("%d ", packer->arrays[i][j]);
        }
        printf("\n");
    }
    printf("\n");*/

    packer->array_index = 0;
    packer->bit_position = 0;
    memset(packer->arrays, 0, sizeof(packer->arrays));
    packer->bit_position = 0;
    packer->consecutive_ones = 0;
}

/**
 * @brief Processes one input struct by distributing bits
 */
void process_tone_bits(struct_tone_bits input) {
    if(G_MODE > 1){
        bool a = add_bit(&master_packer, input.master, "MASTER");
        bool b = add_bit(&slave_packer, input.slave, "SLAVE");
        bool c = add_bit(&config_packer, input.configuration, "CONFIG");
        serial_write_formatted("Debug: PTB Master: %d Slave: %d Config: %d\n", a, b, c);
    }else{
        add_bit(&master_packer, input.master, "MASTER");
        add_bit(&slave_packer, input.slave, "SLAVE");
        add_bit(&config_packer, input.configuration, "CONFIG");
    }

}



#ifdef __cplusplus
}
#endif
