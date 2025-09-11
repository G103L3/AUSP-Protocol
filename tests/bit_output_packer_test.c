/*! \file bit_output_packer_test.c
 * \author Gioele Giunta
 * \version 1.2
 * \since 2025
 * \brief Implementazione del modulo bit output packer test
 */

/* Librerie */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Headers specifici */
#include "../src/global_parameters.h"
#include "../src/bit_output_packer.h"
#include "bit_output_packer_test.h"

 /* C Library Headers */
 
 /* Driver Headers */
 
 /* Our Headers */

/*#include <HardwareSerial.h> */
 


static BitOutputPacker out_packer;
static struct_out_tones* out_pairs = NULL;
static size_t out_len = 0;
/**
 * @brief Funzione main.
 */

void main(void){
    bit_output_packer_init(&out_packer);
    if(bit_output_packer_compress(&out_packer, "HELLO")){
        bit_output_packer_convert(&out_packer, 0);
    }
}
