#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include "global_parameters.h"
#include "audio_driver.h"
#include "bit_freq_codec.h"
#include "serial_bridge.h"
#include "leds.h"


bool emit_tones(int *bits, int role){
    if(bits == NULL){
        turn_red(1);
        serial_write_string("Error: bits array is NULL\n");
        return false;
    }
    for(int i = 0; i < sizeof(bits)/sizeof(int); i++){
        if(bits[i] < 0 || bits[i] > 1){
            serial_write_string("Error: bits array contains invalid values, only 0 and 1 are allowed\n");
            turn_red(1);
            return false;
        }
        struct_out_tones output = frequency_coder(bits[i], role); 
        play_two_tones(output.tones[0], output.tones[1]);

    }
    /*Inizializzazione Driver Audio*/
    //Già inizializzato nel Main
}

#ifdef __cplusplus
}
#endif
