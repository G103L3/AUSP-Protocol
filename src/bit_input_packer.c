#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>            // <-- per clock_gettime
#include "serial_bridge.h"
#include "bit_input_packer.h"
#include "global_parameters.h"

#define TOTAL_BITS (MAX_ARRAY_SIZE * NUM_ARRAYS * 7)

// ------------------------ Nuove utility: tempo & filtro ASCII ------------------------

static uint64_t now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)(ts.tv_nsec / 1000000ULL);
}

static bool is_allowed_ascii_char(unsigned char c) {
    if ((c >= '0' && c <= '9') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z'))
        return true;

    switch (c) {
        case '{': case '}': case '[': case ']':
        case '(': case ')': case ':': case ';':
            return true;
        default:
            return false;
    }
}

static bool is_clean_ascii(const char* s) {
    // scarta stringhe con controlli (NUL, DEL, ecc.) o caratteri non ammessi
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        if (!is_allowed_ascii_char(*p)) {
            return false;
        }
    }
    return true;
}

// -------------------------------------------------------------------------------------

BitPacker master_packer = {0};
BitPacker slave_packer = {0};
BitPacker config_packer = {0};

static bool noise_flag_master = false;
static bool noise_flag_slave = false;
static bool noise_flag_config = false;

char master_ascii_arrays[ASCII_NUM_ARRAYS][ASCII_ARRAY_SIZE] = {0};
char slave_ascii_arrays[ASCII_NUM_ARRAYS][ASCII_ARRAY_SIZE] = {0};
char config_ascii_arrays[ASCII_NUM_ARRAYS][ASCII_ARRAY_SIZE] = {0};

uint8_t master_codes_arrays[ASCII_NUM_ARRAYS][ASCII_ARRAY_SIZE] = {0};
uint8_t slave_codes_arrays[ASCII_NUM_ARRAYS][ASCII_ARRAY_SIZE] = {0};
uint8_t config_codes_arrays[ASCII_NUM_ARRAYS][ASCII_ARRAY_SIZE] = {0};

int test_count = 0;

// ------------------------ Stato timeout per canale (1s) ------------------------------
static uint64_t last_bit_ms_master  = 0;
static uint64_t last_bit_ms_slave   = 0;
static uint64_t last_bit_ms_config  = 0;

static bool timeout_armed_master    = false;
static bool timeout_armed_slave     = false;
static bool timeout_armed_config    = false;

#define TIMEOUT_MS 1000
// -------------------------------------------------------------------------------------

static char (*ascii_for_packer(BitPacker* packer))[ASCII_ARRAY_SIZE] {
    if (packer == &master_packer) return master_ascii_arrays;
    if (packer == &slave_packer) return slave_ascii_arrays;
    return config_ascii_arrays;
}

static uint8_t (*codes_for_packer(BitPacker* packer))[ASCII_ARRAY_SIZE] {
    if (packer == &master_packer) return master_codes_arrays;
    if (packer == &slave_packer) return slave_codes_arrays;
    return config_codes_arrays;
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

bool flush_and_convert_to_ascii(BitPacker* packer, const char* label) {
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

    char temp[ASCII_PACKET_SIZE] = {0};
    size_t array_index = 0;
    size_t byte_index = 0;
    size_t buf_idx = 0;
    printf("Info: Converting %zu bits to ASCII\n", total_bits);
    for (size_t i = 0; i < total_bytes && buf_idx < ASCII_PACKET_SIZE - 1; i++) {
        if (byte_index + 7 > MAX_ARRAY_SIZE) {
            byte_index = 0;
            array_index++;
        }
        char bits[8] = {0}; // <-- assicurati che sia terminato a NUL
        for (size_t j = 0; j < 7; j++) {
            bits[j] = packer->arrays[array_index][byte_index + j] ? '1' : '0';
        }

        unsigned long value = strtoul(bits, NULL, 2);
        temp[buf_idx++] = (char)value;
        printf("-> %c \n", (char)value);

        byte_index += 7;
        if (byte_index >= MAX_ARRAY_SIZE) {
            byte_index = 0;
            array_index++;
        }
    }
    temp[buf_idx] = '\0';

    if (temp[0] && !is_clean_ascii(temp)) {
        printf("%s: flush scartato (caratteri non ammessi). Considerato sporcizia.\n", label);
        packer->array_index = 0;
        packer->bit_position = 0;
        packer->codes_packet_index = 0;
        memset(packer->codes_packet, 0, sizeof(packer->codes_packet));
        memset(packer->arrays, 0, sizeof(packer->arrays));
        return false;
    }

    char (*ascii_dest)[ASCII_ARRAY_SIZE] = ascii_for_packer(packer);
    uint8_t (*codes_dest)[ASCII_ARRAY_SIZE] = codes_for_packer(packer);
    for (size_t i = 0; i < buf_idx; i++) {
        ascii_dest[packer->ascii_array_index][packer->ascii_char_index++] = temp[i];
        if (packer->ascii_char_index >= ASCII_ARRAY_SIZE) {
            packer->ascii_char_index = 0;
            packer->ascii_array_index++;
            if (packer->ascii_array_index >= ASCII_NUM_ARRAYS) {
                printf("Warning: %s ASCII arrays full.\n", label);
                break;
            }
        }
    }
    if (packer->ascii_array_index < ASCII_NUM_ARRAYS) {
        ascii_dest[packer->ascii_array_index][packer->ascii_char_index++] = (char)18;
        if (packer->ascii_char_index >= ASCII_ARRAY_SIZE) {
            packer->ascii_char_index = 0;
            packer->ascii_array_index++;
        }
        if (packer->ascii_array_index < ASCII_NUM_ARRAYS && packer->ascii_char_index < ASCII_ARRAY_SIZE) {
            ascii_dest[packer->ascii_array_index][packer->ascii_char_index] = '\0';
        }
    }

    for (size_t i = 0; i < packer->codes_packet_index; i++) {
        codes_dest[packer->codes_array_index][packer->codes_char_index++] = packer->codes_packet[i];
        if (packer->codes_char_index >= ASCII_ARRAY_SIZE) {
            packer->codes_char_index = 0;
            packer->codes_array_index++;
            if (packer->codes_array_index >= ASCII_NUM_ARRAYS) {
                printf("Warning: %s codes arrays full.\n", label);
                break;
            }
        }
    }
    if (packer->codes_array_index < ASCII_NUM_ARRAYS) {
        codes_dest[packer->codes_array_index][packer->codes_char_index++] = 18;
        if (packer->codes_char_index >= ASCII_ARRAY_SIZE) {
            packer->codes_char_index = 0;
            packer->codes_array_index++;
        }
    }

    packer->codes_packet_index = 0;
    memset(packer->codes_packet, 0, sizeof(packer->codes_packet));
    packer->array_index = 0;
    packer->bit_position = 0;
    memset(packer->arrays, 0, sizeof(packer->arrays));
    return true;
}

// ------------------------ Flush condizionato da timeout + validazione ----------------
static bool timeout_flush_if_needed(BitPacker* packer,
                                     const char* label,
                                     bool* timeout_armed,
                                     uint64_t last_bit_ms,
                                     bool no_new_bit_this_tick)
{
    if (!*timeout_armed) return false;
    if (!no_new_bit_this_tick) return false; // è arrivato un bit ora: non flussare

    uint64_t tnow = now_ms();
    if ((tnow - last_bit_ms) < TIMEOUT_MS) return false;

    bool ok = flush_and_convert_to_ascii(packer, label);
    *timeout_armed = false; // finestra chiusa
    return ok;
}
// -------------------------------------------------------------------------------------

bool add_bit(BitPacker* packer, uint8_t signal_code, const char* label) {
    size_t array_index_ = packer->array_index;
    size_t bit_index = packer->bit_position;

    serial_write_formatted("Info: Array_index: %d\n", array_index_);

    if (signal_code != 8 && packer->codes_packet_index < ASCII_PACKET_SIZE) {
        packer->codes_packet[packer->codes_packet_index++] = signal_code;
    }

    if (signal_code <= 9) {
        if(signal_code <= 6){
            for(int i = 0; i < signal_code+1; i++){
                packer->arrays[array_index_][bit_index] = 0;
                if(update_packet(packer, (char*)label)){
                    return flush_and_convert_to_ascii(packer, label);
                }
                array_index_ = packer->array_index;
                bit_index = packer->bit_position;
            }
        }
        if(signal_code == 8){
            // Code 8 = 21 zeri (flush immediato)
            printf("%s: %d consecutive 1s (code 8). Auto flush.\n", label, MAX_CONSECUTIVE_ZEROS);
            return flush_and_convert_to_ascii(packer, label);
        }
    } else {
        if(signal_code <= 19){
            signal_code = signal_code%10;
            for(int i = 0; i < signal_code+1; i++){
                packer->arrays[array_index_][bit_index] = 1;
                if(update_packet(packer, (char*)label)){
                    return flush_and_convert_to_ascii(packer, label);
                }
                array_index_ = packer->array_index;
                bit_index = packer->bit_position;
            }
        }
    }

    return false;
}

bool process_tone_bits(struct_tone_bits input) {
    bool has_tone_master = (input.master >= 0);
    bool has_tone_slave  = (input.slave >= 0);
    bool has_tone_config = (input.configuration >= 0);

    bool packet_ready = false;

    // Mantieni la logica "noise" esistente
    if (!has_tone_master) noise_flag_master = true;
    if (!has_tone_slave)  noise_flag_slave  = true;
    if (!has_tone_config) noise_flag_config = true;

    if (!noise_flag_master && !noise_flag_slave && !noise_flag_config) {
        // Nessun canale in rumore ⇒ nulla da fare
        return false;
    }

    // 1) Prima: gestisci timeout (se in questa "tick" non è arrivato un nuovo bit per quel canale)
    packet_ready |= timeout_flush_if_needed(&master_packer, "MASTER", &timeout_armed_master, last_bit_ms_master, !has_tone_master);
    packet_ready |= timeout_flush_if_needed(&slave_packer,  "SLAVE",  &timeout_armed_slave,  last_bit_ms_slave,  !has_tone_slave);
    packet_ready |= timeout_flush_if_needed(&config_packer, "CONFIG", &timeout_armed_config, last_bit_ms_config, !has_tone_config);

    uint64_t tnow = now_ms();

    // 2) Poi: processa eventuali nuovi bit
    if (has_tone_master && noise_flag_master) {
        printf(" %d- ", input.master);

        // Se è code 8, flush immediato e disarma timeout
        if (input.master == 8) {
            if(add_bit(&master_packer, input.master, "MASTER")) packet_ready = true;
            timeout_armed_master = false;
        } else {
            if(add_bit(&master_packer, input.master, "MASTER")) packet_ready = true;
            // arma la finestra di 1s in attesa del prossimo bit o di code 8
            timeout_armed_master = true;
            last_bit_ms_master = tnow;
        }
        noise_flag_master = false;
    }

    if (has_tone_slave && noise_flag_slave) {
        if (input.slave == 8) {
            if(add_bit(&slave_packer, input.slave, "SLAVE")) packet_ready = true;
            timeout_armed_slave = false;
        } else {
            if(add_bit(&slave_packer, input.slave, "SLAVE")) packet_ready = true;
            timeout_armed_slave = true;
            last_bit_ms_slave = tnow;
        }
        noise_flag_slave = false;
    }

    if (has_tone_config && noise_flag_config) {
        if (input.configuration == 8) {
            if(add_bit(&config_packer, input.configuration, "CONFIG")) packet_ready = true;
            timeout_armed_config = false;
        } else {
            if(add_bit(&config_packer, input.configuration, "CONFIG")) packet_ready = true;
            timeout_armed_config = true;
            last_bit_ms_config = tnow;
        }
        noise_flag_config = false;
    }

    return packet_ready;
}

size_t bit_input_packer_total_code_sections(BitPacker* packer){
    uint8_t (*codes)[ASCII_ARRAY_SIZE] = codes_for_packer(packer);
    size_t count = 0;
    for(size_t i = 0; i < ASCII_NUM_ARRAYS; ++i){
        for(size_t j = 0; j < ASCII_ARRAY_SIZE; ++j){
            if(codes[i][j] == 18) count++;
        }
    }
    return count;
}

bool bit_input_packer_get_code_section(BitPacker* packer, size_t section, uint8_t* out, size_t* out_len){
    if(!packer || !out || !out_len) return false;
    uint8_t (*codes)[ASCII_ARRAY_SIZE] = codes_for_packer(packer);
    size_t total = packer->codes_array_index * ASCII_ARRAY_SIZE + packer->codes_char_index;
    size_t current = 0;
    size_t out_idx = 0;
    for(size_t idx = 0; idx < total; ++idx){
        uint8_t c = codes[idx / ASCII_ARRAY_SIZE][idx % ASCII_ARRAY_SIZE];
        if(current == section){
            if(c == 18){
                *out_len = out_idx;
                return true;
            }
            out[out_idx++] = c;
        }
        if(c == 18) current++;
    }
    return false;
}

bool bit_input_packer_remove_code_section(BitPacker* packer, size_t section){
    if(!packer) return false;
    uint8_t (*codes)[ASCII_ARRAY_SIZE] = codes_for_packer(packer);
    size_t total = packer->codes_array_index * ASCII_ARRAY_SIZE + packer->codes_char_index;
    size_t current = 0;
    size_t start = 0;
    size_t end = 0;
    bool found = false;
    for(size_t idx = 0; idx < total; ++idx){
        uint8_t c = codes[idx / ASCII_ARRAY_SIZE][idx % ASCII_ARRAY_SIZE];
        if(current == section && !found) start = idx;
        if(c == 18){
            if(current == section){
                end = idx;
                found = true;
                break;
            }
            current++;
        }
    }
    if(!found) return false;
    size_t remove_len = end - start + 1;
    for(size_t idx = end + 1; idx < total; ++idx){
        uint8_t c = codes[idx / ASCII_ARRAY_SIZE][idx % ASCII_ARRAY_SIZE];
        size_t dst = idx - remove_len;
        codes[dst / ASCII_ARRAY_SIZE][dst % ASCII_ARRAY_SIZE] = c;
    }
    for(size_t idx = total - remove_len; idx < total; ++idx){
        codes[idx / ASCII_ARRAY_SIZE][idx % ASCII_ARRAY_SIZE] = 0;
    }
    total -= remove_len;
    packer->codes_array_index = total / ASCII_ARRAY_SIZE;
    packer->codes_char_index = total % ASCII_ARRAY_SIZE;
    return true;
}

#ifdef __cplusplus
}
#endif
