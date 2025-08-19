#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <stddef.h>
#include "global_parameters.h"
#include "audio_driver.h"
#include "serial_bridge.h"
#include "leds.h"


bool emit_tones(const struct_out_tones *pairs, size_t length){
    if(pairs == NULL || length == 0){
        turn_red(1);
        serial_write_string("Error: pairs array is NULL or length is zero\n");
        return false;
    }
    for(size_t i = 0; i < length; i++){
        play_two_tones(pairs[i].tones[0], pairs[i].tones[1]);
    }
    /*Inizializzazione Driver Audio già eseguita nel main*/
    return true;
}

#ifdef __cplusplus
}
#endif
