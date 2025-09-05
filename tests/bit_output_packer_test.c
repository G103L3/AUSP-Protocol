/*! \file main.cpp
 * \brief File principale per ESP32 con acquisizione DMA
 */

 /* C Library Headers */
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 
 /* Driver Headers */
#include <math.h>
 
 /* Our Headers */
 #include "../src/global_parameters.h"

#include "../src/bit_output_packer.h"
//#include <HardwareSerial.h>
 


static BitOutputPacker out_packer;
static struct_out_tones* out_pairs = NULL;
static size_t out_len = 0;

void main(void){
    bit_output_packer_init(&out_packer);
        out_pairs = bit_output_packer_pack(&out_packer, "HELLO", 0);
}