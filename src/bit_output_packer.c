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

void bit_output_packer_init(BitOutputPacker* packer){
    if(!packer) return;
    packer->pairs = NULL;
    packer->pair_count = 0;
    memset(packer->codes, 0, sizeof(packer->codes));
    packer->array_index = 0;
    packer->position = 0;
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
static struct_out_tones silent = {0,0}; 

bool bit_output_packer_compress(BitOutputPacker* packer, const char* text){
    if(!packer || !text) return false;

    bit_output_packer_free(packer);

    //Azzeramento array
    memset(packer->codes, 0, sizeof(packer->codes));
    last = 2;
    packer->array_index = 0;
    packer->position = 0;

    size_t len = strlen(text);

    if(len > BOP_MAX_CHARS) len = BOP_MAX_CHARS;

    packer->pair_count = 0;
    for(size_t i = 0; i < len; ++i){
        unsigned char c = (unsigned char)text[i];
        printf("INFO: %c: \n", c);
        for(int b = 6; b >= 0; --b){

            int bit = (c >> b) & 1;

            //Inizio compressione

                printf(" %d i: %d len-1: %d \n", bit, i, len-1);
                if(bit == 0){
                    if(last != bit || (i == len-1 && b == 0 && consecutive_packing_zeroes > 0)){
                        if(last != 2){
                            printf(" -> ZIPPED (stampa gli 1 precedenti): %d \n", consecutive_packing_ones);
                            packer->codes[packer->array_index][packer->position] = 10 + consecutive_packing_ones-1;
                            printf(" -> POS (stampa gli 1 precedenti): %d %d %d \n", packer->position, consecutive_packing_ones, packer->codes[packer->array_index][packer->position]);
                            //10 = un 1; 11 = due 1; 12 = tre 1; 13 = quattro 1; 14 = cinque 1; 15 = sei 1; 16 = sette 1; 17 = 14 1; 18 = 21 1
                            packer->position++;
                            if(packer->position >= ZIPPED_ARRAY_SIZE){
                                packer->position = 0;
                                packer->array_index++;
                            }
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
                            packer->codes[packer->array_index][packer->position] = consecutive_packing_zeroes-1;
                            printf("-> POS (stampa gli 0 precedenti): %d %d %d \n", packer->position, consecutive_packing_zeroes, packer->codes[packer->array_index][packer->position]);
                            // 0 = un 0; 1 = due 0; 2 = tre 0; 3 = quattro 0; 4 = cinque 0; 5 = sei 0; 6 = sette 0; 7 = 14 0; 8 = 21 0
                            packer->position++;
                            if(packer->position >= ZIPPED_ARRAY_SIZE){
                                packer->position = 0;
                                packer->array_index++;
                            }
                        }

                        consecutive_packing_zeroes = 0;
                    }
                    last = 1;
                    consecutive_packing_ones++;

                    if(check_one){
                        packer->codes[packer->array_index][packer->position] = 10 + consecutive_packing_ones-1;
                        packer->position++;
                        if(packer->position >= ZIPPED_ARRAY_SIZE){
                            packer->position = 0;
                            packer->array_index++;
                        }
                    }
                }




        }
        printf("\n");
    }
    packer->codes[packer->array_index][packer->position++] = 18;
    if(packer->position >= ZIPPED_ARRAY_SIZE){
        packer->position = 0;
        packer->array_index++;
    }
    return true;
}

static size_t section_bounds(const BitOutputPacker* packer, size_t section, size_t* start){
    size_t total = packer->array_index * ZIPPED_ARRAY_SIZE + packer->position;
    size_t current = 0;
    size_t idx;
    *start = 0;
    for(idx = 0; idx < total; ++idx){
        int code = packer->codes[idx / ZIPPED_ARRAY_SIZE][idx % ZIPPED_ARRAY_SIZE];
        if(code == 18){
            if(current == section){
                return idx;
            }
            current++;
            *start = idx + 1;
        }
    }
    if(current == section) return total;
    return (size_t)-1;
}

bool bit_output_packer_add_codes(BitOutputPacker* packer, const int* codes, size_t count){
    if(!packer || !codes) return false;
    for(size_t i = 0; i < count; ++i){
        packer->codes[packer->array_index][packer->position++] = codes[i];
        if(packer->position >= ZIPPED_ARRAY_SIZE){
            packer->position = 0;
            packer->array_index++;
            if(packer->array_index >= ZIPPED_NUM_ARRAYS) return false;
        }
    }
    packer->codes[packer->array_index][packer->position++] = 18;
    if(packer->position >= ZIPPED_ARRAY_SIZE){
        packer->position = 0;
        packer->array_index++;
        if(packer->array_index >= ZIPPED_NUM_ARRAYS) return false;
    }
    return true;
}

size_t bit_output_packer_total_sections(const BitOutputPacker* packer){
    size_t total = packer->array_index * ZIPPED_ARRAY_SIZE + packer->position;
    size_t count = 0;
    for(size_t idx = 0; idx < total; ++idx){
        if(packer->codes[idx / ZIPPED_ARRAY_SIZE][idx % ZIPPED_ARRAY_SIZE] == 18) count++;
    }
    return count;
}

bool bit_output_packer_convert(BitOutputPacker* packer, size_t section, int role){
    if(!packer) return false;

    size_t start;
    size_t end = section_bounds(packer, section, &start);
    if(end == (size_t)-1) return false;

    size_t codes = end - start;
    size_t needed = (codes * 7)*rep + (3*7)*rep;
    packer->pairs = (struct_out_tones*)malloc(needed * sizeof(struct_out_tones));
    if(!packer->pairs) return false;

    packer->pair_count = 0;
    for(size_t idx = start; idx < end; idx++){
        int code = packer->codes[idx / ZIPPED_ARRAY_SIZE][idx % ZIPPED_ARRAY_SIZE];
        printf(" %d \n", code);
        for(int j = 0; j < rep; j++){
            packer->pairs[packer->pair_count++] = frequency_coder(code, role);
            printf(" -> FREQ: %d %d \n", frequency_coder(code, role).tones[0], frequency_coder(code, role).tones[1]);
            //Ne aggiunge sette (rep) cosi che poi in audio driver l'emissione si di 0.35 + 0.35

        }
        packer->pairs[packer->pair_count++] = silent;
    }
    //Ne aggiunge sette cosi che poi in audio driver l'emissione si di 0.35 + 0.35


    //Inserimento terminatori (21 volte 0 -> codice 8)
    printf("INFO: NUL: " );

    for(int b = 6; b >= 0; --b){
        printf(" 8 ");
        packer->pairs[packer->pair_count++] = frequency_coder(8, role);
        //Ne aggiunge sette (rep) cosi che poi in audio driver l'emissione si di 0.35 + 0.35
    }
    packer->pairs[packer->pair_count++] = silent;

    printf("\n");

    return true;
}

bool bit_output_packer_remove_section(BitOutputPacker* packer, size_t section){
    if(!packer) return false;
    size_t start;
    size_t end = section_bounds(packer, section, &start);
    if(end == (size_t)-1) return false;
    size_t total = packer->array_index * ZIPPED_ARRAY_SIZE + packer->position;
    size_t remove_len = end - start + 1; // include delimiter
    for(size_t idx = end + 1; idx < total; ++idx){
        int code = packer->codes[idx / ZIPPED_ARRAY_SIZE][idx % ZIPPED_ARRAY_SIZE];
        size_t dst_idx = idx - remove_len;
        packer->codes[dst_idx / ZIPPED_ARRAY_SIZE][dst_idx % ZIPPED_ARRAY_SIZE] = code;
    }
    total -= remove_len;
    packer->array_index = total / ZIPPED_ARRAY_SIZE;
    packer->position = total % ZIPPED_ARRAY_SIZE;
    for(size_t idx = packer->position; idx < ZIPPED_ARRAY_SIZE; ++idx){
        packer->codes[packer->array_index][idx] = 0;
    }
    for(size_t arr = packer->array_index + 1; arr < ZIPPED_NUM_ARRAYS; ++arr){
        memset(packer->codes[arr], 0, sizeof(packer->codes[arr]));
    }
    return true;
}

#ifdef __cplusplus
}
#endif

