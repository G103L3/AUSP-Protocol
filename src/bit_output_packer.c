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

struct_out_tones* bit_output_packer_pack(BitOutputPacker* packer, const char* text, int role){
    if(!packer || !text) return NULL;

    bit_output_packer_free(packer);

    size_t len = strlen(text);
    if(len > BOP_MAX_CHARS) len = BOP_MAX_CHARS;
    size_t needed = len * 8;
    packer->pairs = (struct_out_tones*)malloc(needed * sizeof(struct_out_tones));
    if(!packer->pairs) return NULL;

    packer->pair_count = 0;
    for(size_t i = 0; i < len; ++i){
        unsigned char c = (unsigned char)text[i];
        for(int b = 7; b >= 0; --b){
            int bit = (c >> b) & 1;
            packer->pairs[packer->pair_count++] = frequency_coder(bit, role);
        }
    }

    return packer->pairs;
}

#ifdef __cplusplus
}
#endif

