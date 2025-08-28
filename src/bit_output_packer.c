#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdlib.h>


#include "bit_output_packer.h"
#include "bit_freq_codec.h"

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

    size_t len = strlen(text);
    if(len > BOP_MAX_CHARS) len = BOP_MAX_CHARS;
    size_t needed = (len * 7)*7 + (3*7)*7;
    packer->pairs = (struct_out_tones*)malloc(needed * sizeof(struct_out_tones));
    if(!packer->pairs) return NULL;

    packer->pair_count = 0;
    for(size_t i = 0; i < len; ++i){
        unsigned char c = (unsigned char)text[i];
        printf("INFO: %c: ", c);
        for(int b = 6; b >= 0; --b){

            int bit = (c >> b) & 1;
            printf(" %d ", bit);
            //Ne aggiunge due di proposito cosi che poi in audio driver l'emissione si di 0.35 + 0.35
            packer->pairs[packer->pair_count++] = frequency_coder(bit, role);
            packer->pairs[packer->pair_count++] = frequency_coder(bit, role);
            packer->pairs[packer->pair_count++] = frequency_coder(bit, role);
            packer->pairs[packer->pair_count++] = frequency_coder(bit, role);
            packer->pairs[packer->pair_count++] = frequency_coder(bit, role);
            packer->pairs[packer->pair_count++] = frequency_coder(bit, role);
            packer->pairs[packer->pair_count++] = silent;

        }
        printf("\n");
    }

    //IL CODICE SOTTO FA ANDARE IN REBOOT DA FIXARE!!!!
    for(int i = 0; i < 3; i++){
        printf("INFO: NUL: " );
        for(int b = 6; b >= 0; --b){
            printf(" 0 ");
            packer->pairs[packer->pair_count++] = frequency_coder(0, role);
            packer->pairs[packer->pair_count++] = frequency_coder(0, role);
            packer->pairs[packer->pair_count++] = frequency_coder(0, role);
            packer->pairs[packer->pair_count++] = frequency_coder(0, role);
            packer->pairs[packer->pair_count++] = frequency_coder(0, role);
            packer->pairs[packer->pair_count++] = frequency_coder(0, role);
            packer->pairs[packer->pair_count++] = silent;
        }
        printf("\n");

    }


    return packer->pairs;
}

#ifdef __cplusplus
}
#endif

