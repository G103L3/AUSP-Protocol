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

#define TOTAL_BITS (MAX_ARRAY_SIZE * NUM_ARRAYS * 7)

BitPacker master_packer = {0};
BitPacker slave_packer = {0};
BitPacker config_packer = {0};

static bool noise_flag_master = false;
static bool noise_flag_slave = false;
static bool noise_flag_config = false;

char master_ascii_packet[ASCII_PACKET_SIZE] = {0};
char slave_ascii_packet[ASCII_PACKET_SIZE] = {0};
char config_ascii_packet[ASCII_PACKET_SIZE] = {0};



int test_count = 0;


static char* buffer_for_packer(BitPacker* packer) {
    if (packer == &master_packer) return master_ascii_packet;
    if (packer == &slave_packer) return slave_ascii_packet;
    return config_ascii_packet;
}

bool update_packet(BitPacker* packer_, char* label_){
    packer_->bit_position++;
    if (packer_->bit_position >= MAX_ARRAY_SIZE) {
        packer_->bit_position = 0;
        packer_->array_index++;
    }
    if (packer_->array_index >= NUM_ARRAYS) {
        printf("Warning: %s arrays full. Auto flush.\n", label_);
        return true;
    }else{
        return false;
    }
}

char* add_bit(BitPacker* packer, uint8_t signal_code, const char* label) {
    char* out = buffer_for_packer(packer);
    out[0] = '\0';
    size_t array_index_ = packer->array_index;
    size_t bit_index = packer->bit_position;

    serial_write_formatted("Info: Array_index: %d\n", array_index_);

    if (signal_code <= 9) {
        if(signal_code <= 6){
            //Si è nell'arco dai 1 ai 7 zeri quindi nessun codice speciale come 21 volte 0 (flush)
            //Trascrive tutti gli 0
            for(int i = 0; i < signal_code+1; i++){
                packer->arrays[array_index_][bit_index] = 0;
                if(update_packet(packer, label)){
                    return flush_and_convert_to_ascii(packer, label);
                }
                array_index_ = packer->array_index;
                bit_index = packer->bit_position;
            }
        }
        if(signal_code == 8){
            //Code 8 = 21 volte 0 (flush)
            printf("%s: %d consecutive 1s (code 8). Auto flush.\n", label, MAX_CONSECUTIVE_ZEROS);
            return flush_and_convert_to_ascii(packer, label);
        }
    } else {
        if(signal_code <= 19){
            //Si è nell'arco dai 1 ai 7 uno 
            //Trascrive tutti gli 1
            signal_code = signal_code%10;
            for(int i = 0; i < signal_code+1; i++){
                packer->arrays[array_index_][bit_index] = 1;
                if(update_packet(packer, label)){
                    return flush_and_convert_to_ascii(packer, label);
                }                array_index_ = packer->array_index;
                bit_index = packer->bit_position;
            }
        }
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
    size_t total_bytes = total_bits / 7;

    serial_write_formatted("Info: Flushing %s packer with %zu bits (%zu bytes)\n",
                           label, packer->bit_position, total_bytes);

    char* buffer = buffer_for_packer(packer);
    memset(buffer, 0, ASCII_PACKET_SIZE);
    size_t array_index = 0;
    size_t byte_index = 0;
    size_t buf_idx = 0;
    printf("Info: Converting %zu bits to ASCII\n", total_bits);
    for (size_t i = 0; i < total_bytes && buf_idx < ASCII_PACKET_SIZE - 1; i++) {
        if (byte_index + 7 > MAX_ARRAY_SIZE) {
            byte_index = 0;
            array_index++;
        }
        char bits[7];
        int done_count = 0;
        for (size_t j = 0; j < 7; j++) {
            if(packer->arrays[array_index][byte_index + j] == -1){
            } else{
                bits[j] = packer->arrays[array_index][byte_index + j] ? '1' : '0';
                done_count++;
            }
        }


        unsigned long value = strtoul(bits, NULL, 2);
        buffer[buf_idx++] = (char)value;
        printf("-> %c \n", (char)value);

        byte_index += 7;
        if (byte_index >= MAX_ARRAY_SIZE) {
            byte_index = 0;
            array_index++;
        }
    }
    buffer[buf_idx] = '\0';

    packer->array_index = 0;
    packer->bit_position = 0;
    memset(packer->arrays, 0, sizeof(packer->arrays));
    return buffer;
}

void process_tone_bits(struct_tone_bits input) {
    bool has_tone_master = false;
    bool has_tone_slave = false;
    bool has_tone_config = false;
    if (input.master >= 0) has_tone_master = true;
    if (input.slave >= 0) has_tone_slave = true;
    if (input.configuration >= 0) has_tone_config = true;
    //printf("Info: Received bits - Master: %d, Slave: %d, Config: %d\n", input.master, input.slave, input.configuration);
    //printf("has_tone_master: %d, has_tone_slave: %d, has_tone_config: %d\n", has_tone_master, has_tone_slave, has_tone_config);

    if (!has_tone_master) {
        noise_flag_master = true;
    }
    
    if (!has_tone_slave) {
        noise_flag_slave = true;
    }

    if (!has_tone_config) {
        noise_flag_config = true;
    }

    if (!noise_flag_master && !noise_flag_slave && !noise_flag_config) {
        return;
    }

    if ((input.master >= 0) && noise_flag_master) {
        printf(" %d- ", input.master);
        add_bit(&master_packer, input.master, "MASTER");
        noise_flag_master = false;
    }
    if ((input.slave >= 0) && noise_flag_slave) {
        add_bit(&slave_packer, input.slave, "SLAVE");
        noise_flag_slave = false;
    }
    if ((input.configuration >= 0) && noise_flag_config) {
        add_bit(&config_packer, input.configuration, "CONFIG");
        noise_flag_config = false;
    }

}

#ifdef __cplusplus
}
#endif

