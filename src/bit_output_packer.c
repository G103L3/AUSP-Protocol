#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "bit_output_packer.h"

void bit_output_packer_init(BitOutputPacker* packer){
    if(!packer) return;
    memset(packer->arrays, 0, sizeof(packer->arrays));
    packer->array_index = 0;
    packer->bit_position = 0;
    packer->total_bits = 0;
}

static bool add_bit(BitOutputPacker* packer, int bit){
    if(bit < 0 || bit > 1) return false;
    if(!packer) return false;
    if(packer->array_index >= BOP_NUM_ARRAYS) return false;
    packer->arrays[packer->array_index][packer->bit_position] = bit;
    packer->bit_position++;
    packer->total_bits++;
    if(packer->bit_position >= BOP_MAX_ARRAY_SIZE){
        packer->bit_position = 0;
        packer->array_index++;
        if(packer->array_index >= BOP_NUM_ARRAYS) return false;
    }
    return true;
}

bool bit_output_packer_load(BitOutputPacker* packer, const char* text){
    if(!packer || !text) return false;
    size_t len = strlen(text);
    for(size_t i=0;i<len;i++){
        unsigned char c = (unsigned char)text[i];
        for(int b=7;b>=0;b--){
            if(!add_bit(packer, (c >> b) & 1)){
                return false;
            }
        }
    }
    return true;
}

size_t bit_output_packer_flatten(BitOutputPacker* packer, int* out_bits, size_t max_len){
    if(!packer || !out_bits) return 0;
    size_t count = 0;
    for(size_t i=0;i<packer->array_index;i++){
        for(size_t j=0;j<BOP_MAX_ARRAY_SIZE && count < max_len;j++){
            out_bits[count++] = packer->arrays[i][j];
        }
    }
    for(size_t j=0;j<packer->bit_position && count < max_len;j++){
        out_bits[count++] = packer->arrays[packer->array_index][j];
    }
    return count;
}

#ifdef __cplusplus
}
#endif
