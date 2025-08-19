#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "bit_output_packer.h"
#include "bit_freq_codec.h"

void bit_output_packer_init(BitOutputPacker* packer){
    if(!packer) return;
    packer->pair_count = 0;
}

struct_out_tones* bit_output_packer_pack(BitOutputPacker* packer, const char* text, int role){
    if(!packer || !text) return NULL;

    packer->pair_count = 0;
    size_t len = strlen(text);
    for(size_t i = 0; i < len && packer->pair_count < BOP_MAX_BITS; ++i){
        unsigned char c = (unsigned char)text[i];
        for(int b = 7; b >= 0 && packer->pair_count < BOP_MAX_BITS; --b){
            int bit = (c >> b) & 1;
            packer->pairs[packer->pair_count++] = frequency_coder(bit, role);
        }
    }

    return packer->pairs;
}

#ifdef __cplusplus
}
#endif

