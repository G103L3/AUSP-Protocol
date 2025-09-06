#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdlib.h>

#include "bit_output_packer.h"
#include "bit_freq_codec.h"

int consecutive_packing_zeroes = 0;
int consecutive_packing_ones = 0;
int last = 2;
int rep = 3;
static int zipped_pack[ZIPPED_NUM_ARRAYS][ZIPPED_ARRAY_SIZE];
static size_t zipped_array_index = 0;
static size_t zipped_position = 0;
static size_t zipped_sections = 0;

static struct_out_tones silent = {0,0};

static bool append_code_internal(int code){
    if(zipped_array_index >= ZIPPED_NUM_ARRAYS) return false;
    zipped_pack[zipped_array_index][zipped_position++] = code;
    if(zipped_position >= ZIPPED_ARRAY_SIZE){
        zipped_position = 0;
        zipped_array_index++;
        if(zipped_array_index >= ZIPPED_NUM_ARRAYS) return false;
    }
    return true;
}

static size_t total_codes(void){
    return zipped_array_index * ZIPPED_ARRAY_SIZE + zipped_position;
}

static int get_code(size_t idx){
    size_t arr = idx / ZIPPED_ARRAY_SIZE;
    size_t pos = idx % ZIPPED_ARRAY_SIZE;
    return zipped_pack[arr][pos];
}

static void set_code(size_t idx, int code){
    size_t arr = idx / ZIPPED_ARRAY_SIZE;
    size_t pos = idx % ZIPPED_ARRAY_SIZE;
    zipped_pack[arr][pos] = code;
}

void bit_output_packer_init(BitOutputPacker* packer){
    if(!packer) return;
    packer->pairs = NULL;
    packer->pair_count = 0;
    memset(zipped_pack, 0, sizeof(zipped_pack));
    zipped_array_index = 0;
    zipped_position = 0;
    zipped_sections = 0;
    consecutive_packing_zeroes = 0;
    consecutive_packing_ones = 0;
    last = 2;
}

void bit_output_packer_free(BitOutputPacker* packer){
    if(!packer) return;
    free(packer->pairs);
    packer->pairs = NULL;
    packer->pair_count = 0;
}

static bool compress_impl(BitOutputPacker* packer, const char* text, bool reset){
    if(!packer || !text) return false;
    if(reset){
        bit_output_packer_free(packer);
        memset(zipped_pack, 0, sizeof(zipped_pack));
        zipped_array_index = 0;
        zipped_position = 0;
        zipped_sections = 0;
    }
    consecutive_packing_zeroes = 0;
    consecutive_packing_ones = 0;
    last = 2;

    size_t len = strlen(text);
    if(len > BOP_MAX_CHARS) len = BOP_MAX_CHARS;

    for(size_t i = 0; i < len; ++i){
        unsigned char c = (unsigned char)text[i];
        printf("INFO: %c: \n", c);
        for(int b = 6; b >= 0; --b){
            int bit = (c >> b) & 1;
            printf(" %d i: %zu len-1: %zu \n", bit, i, len-1);
            if(bit == 0){
                if(last != bit || (i == len-1 && b == 0 && consecutive_packing_zeroes > 0)){
                    if(last != 2){
                        printf(" -> ZIPPED (stampa gli 1 precedenti): %d \n", consecutive_packing_ones);
                        if(!append_code_internal(10 + consecutive_packing_ones-1)) return false;
                        printf(" -> POS (stampa gli 1 precedenti): %zu %d %d \n", zipped_position-1, consecutive_packing_ones, get_code(total_codes()-1));
                        //10 = un 1; 11 = due 1; 12 = tre 1; 13 = quattro 1; 14 = cinque 1; 15 = sei 1; 16 = sette 1; 17 = 14 1; 18 = 21 1
                    }
                    consecutive_packing_ones = 0;
                }
                last = 0;
                consecutive_packing_zeroes++;
            }
            if(bit == 1){
                printf("consecutive_packing_ones: %d \n", consecutive_packing_ones);
                bool check_one = i == len-1 && b == 0 && consecutive_packing_ones > 0;
                if(last != bit){
                    if(last != 2 && !check_one){
                        printf("-> ZIPPED (stampa gli 0 precedenti): %d \n", consecutive_packing_zeroes);
                        if(!append_code_internal(consecutive_packing_zeroes-1)) return false;
                        printf("-> POS (stampa gli 0 precedenti): %zu %d %d \n", zipped_position-1, consecutive_packing_zeroes, get_code(total_codes()-1));
                        // 0 = un 0; 1 = due 0; 2 = tre 0; 3 = quattro 0; 4 = cinque 0; 5 = sei 0; 6 = sette 0; 7 = 14 0; 8 = 21 0
                    }
                    consecutive_packing_zeroes = 0;
                }
                last = 1;
                consecutive_packing_ones++;
                if(check_one){
                    if(!append_code_internal(10 + consecutive_packing_ones-1)) return false;
                }
            }
        }
        printf("\n");
    }
    if(!append_code_internal(18)) return false;
    zipped_sections++;
    return true;
}

bool bit_output_packer_compress(BitOutputPacker* packer, const char* text){
    return compress_impl(packer, text, true);
}

bool bit_output_packer_append_ascii(BitOutputPacker* packer, const char* text){
    return compress_impl(packer, text, false);
}

bool bit_output_packer_append_codes(BitOutputPacker* packer, const int* codes, size_t count){
    (void)packer;
    if(!codes) return false;
    for(size_t i = 0; i < count; i++){
        if(!append_code_internal(codes[i])) return false;
    }
    if(!append_code_internal(18)) return false;
    zipped_sections++;
    return true;
}

size_t bit_output_packer_section_count(void){
    return zipped_sections;
}

static bool find_section_bounds(size_t section_index, size_t* start, size_t* end){
    size_t total = total_codes();
    size_t curr = 0;
    size_t s = 0;
    for(size_t i = 0; i < total; i++){
        if(get_code(i) == 18){
            if(curr == section_index){
                *start = s;
                *end = i;
                return true;
            }
            curr++;
            s = i + 1;
        }
    }
    return false;
}

bool bit_output_packer_convert_section(BitOutputPacker* packer, size_t section_index, int role){
    if(!packer) return false;
    size_t start, end;
    if(!find_section_bounds(section_index, &start, &end)) return false;
    size_t codes = end - start;
    size_t needed = (codes * 7)*rep + (3*7)*rep;
    packer->pairs = (struct_out_tones*)malloc(needed * sizeof(struct_out_tones));
    if(!packer->pairs) return false;
    packer->pair_count = 0;
    for(size_t idx = start; idx < end; idx++){
        int code = get_code(idx);
        for(int j = 0; j < rep; j++){
            packer->pairs[packer->pair_count++] = frequency_coder(code, role);
        }
        packer->pairs[packer->pair_count++] = silent;
    }
    for(int b = 6; b >= 0; --b){
        (void)b;
        packer->pairs[packer->pair_count++] = frequency_coder(8, role);
    }
    packer->pairs[packer->pair_count++] = silent;
    return true;
}

bool bit_output_packer_remove_section(size_t section_index){
    size_t start, end;
    if(!find_section_bounds(section_index, &start, &end)) return false;
    size_t total = total_codes();
    size_t remove_len = (end - start) + 1;
    for(size_t i = end + 1; i < total; i++){
        set_code(i - remove_len, get_code(i));
    }
    for(size_t i = total - remove_len; i < total; i++){
        set_code(i, 0);
    }
    total -= remove_len;
    zipped_array_index = total / ZIPPED_ARRAY_SIZE;
    zipped_position = total % ZIPPED_ARRAY_SIZE;
    if(zipped_sections > 0) zipped_sections--;
    return true;
}

bool bit_output_packer_convert(BitOutputPacker* packer, int role){
    if(!packer) return false;
    size_t codes = total_codes();
    size_t needed = (codes * 7)*rep + (3*7)*rep;
    packer->pairs = (struct_out_tones*)malloc(needed * sizeof(struct_out_tones));
    if(!packer->pairs) return false;
    packer->pair_count = 0;
    for(size_t idx = 0; idx < codes; idx++){
        int code = get_code(idx);
        if(code == 18) continue;
        for(int j = 0; j < rep; j++){
            packer->pairs[packer->pair_count++] = frequency_coder(code, role);
        }
        packer->pairs[packer->pair_count++] = silent;
    }
    for(int b = 6; b >= 0; --b){
        (void)b;
        packer->pairs[packer->pair_count++] = frequency_coder(8, role);
    }
    packer->pairs[packer->pair_count++] = silent;
    return true;
}

#ifdef __cplusplus
}
#endif
