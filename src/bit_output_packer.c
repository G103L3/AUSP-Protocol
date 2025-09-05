#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdlib.h>


#include "bit_output_packer.h"
#include "bit_freq_codec.h"

int consecutive_packing_zeroes = 0;
int consecutive_packing_ones = 0;
int pack_flag = false;
int last = 2;
int rep = 3;
static int zipped_pack[2048];
int bit_position = 0;

void bit_output_packer_init(BitOutputPacker* packer){
    if(!packer) return;
    packer->pairs = NULL;
    packer->pair_count = 0;
}

void bit_output_packer_free(BitOutputPacker* packer){
    if(!packer) return;
    free(packer->pairs);
    packer->pairs = NULL;
    packer->pair_count = 0;
}
static struct_out_tones silent = {0,0}; 

struct_out_tones* bit_output_packer_pack(BitOutputPacker* packer, const char* text, int role){
    if(!packer || !text) return NULL;

    bit_output_packer_free(packer);

    //Azzeramento array
    memset(zipped_pack, 0, sizeof(zipped_pack));
    last = 2;
    bit_position = 0;

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
                            zipped_pack[bit_position] = 10 + consecutive_packing_ones-1; 
                            printf(" -> POS (stampa gli 1 precedenti): %d %d %d \n", bit_position, consecutive_packing_ones, zipped_pack[bit_position]);
                            //10 = un 1; 11 = due 1; 12 = tre 1; 13 = quattro 1; 14 = cinque 1; 15 = sei 1; 16 = sette 1; 17 = 14 1; 18 = 21 1
                            bit_position++;
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
                            zipped_pack[bit_position] = consecutive_packing_zeroes-1;
                            printf("-> POS (stampa gli 0 precedenti): %d %d %d \n", bit_position, consecutive_packing_zeroes, zipped_pack[bit_position]);
                            // 0 = un 0; 1 = due 0; 2 = tre 0; 3 = quattro 0; 4 = cinque 0; 5 = sei 0; 6 = sette 0; 7 = 14 0; 8 = 21 0
                            bit_position++;
                        }

                        consecutive_packing_zeroes = 0;
                    }
                    last = 1;
                    consecutive_packing_ones++;
                    
                    if(check_one){
                        zipped_pack[bit_position] = 10 + consecutive_packing_ones-1;
                        bit_position++;
                    }
                }

            


        }
        printf("\n");
    }
    
    //Conversione in frequenze del pacchetto compresso già in bit
    size_t needed = (bit_position * 7)*rep + (3*7)*rep;
    packer->pairs = (struct_out_tones*)malloc(needed * sizeof(struct_out_tones));
    if(!packer->pairs) return NULL;

    for(int k = 0; k < bit_position; k++){
        printf(" %d \n", zipped_pack[k]);
        for(int j = 0; j < rep; j++){
            packer->pairs[packer->pair_count++] = frequency_coder(zipped_pack[k], role);
            printf(" -> FREQ: %d %d \n", frequency_coder(zipped_pack[k], role).tones[0], frequency_coder(zipped_pack[k], role).tones[1]);
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

    


    return packer->pairs;
}

#ifdef __cplusplus
}
#endif

