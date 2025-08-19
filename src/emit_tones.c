#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <stddef.h>
#include "global_parameters.h"
#include "audio_driver.h"
#include "bit_freq_codec.h"
#include "serial_bridge.h"
#include "leds.h"


bool emit_tones(const int *bits, size_t length, int role){
    if(bits == NULL || length == 0){
        turn_red(1);
        serial_write_string("Error: bits array is NULL or length is zero\n");
        return false;
    }
    for(size_t i = 0; i < length; i++){
        if(bits[i] < 0 || bits[i] > 1){
            serial_write_string("Error: bits array contains invalid values, only 0 and 1 are allowed\n");
            turn_red(1);
            return false;
        }
        struct_out_tones output = frequency_coder(bits[i], role);
        play_two_tones(output.tones[0], output.tones[1]);
    }
    /*Inizializzazione Driver Audio già eseguita nel main*/
    return true;
}

#ifdef __cplusplus
}
#endif
