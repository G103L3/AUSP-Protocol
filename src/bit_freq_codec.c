#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#include "complex_g3.h"
#include "serial_bridge.h"
#include "global_parameters.h"
#include "leds.h"
#include "bit_freq_codec.h"

/*! \fn int interpret_bits(int freqs[3])
 * \param freqs An array of three integers representing the detected frequencies
 * \returns An integer representing the interpreted bits: 0, 1, -2 (error), or -3 (noise)
 * \brief Interprets the detected frequencies and returns the corresponding bit value.
 * 
 * This function checks the presence of specific frequencies in the input array and determines
 * the bit value based on the combination of active frequencies. It also handles error cases
 * such as multitone detection or noise.
 */
int interpret_bits(int freqs[3]) 
{
    int count = 0;
    int active[3] = {0};

    for (int i = 0; i < 3; ++i) {
        if (freqs[i] > 0) {
            active[i] = 1;
            count++;
        }
    }

    if (count == 2) {
        
        if (active[0] && active[1] && !active[2]){ turn_green(1); return 0;} 
        if (!active[0] && active[1] && active[2]){ turn_green(1); return 1;}
        return -2; // errore logico: f1 + f3
    } else if (count == 3) {
        return -2; // multitone
    } else {
        return -3; // noise
    }
}


/*! \fn struct_tone_bits bit_coder(struct_tone_frequencies tones)
 * \param tones A struct_tone_frequencies object containing the detected frequencies
 * \returns A struct_tone_bits object containing the interpreted bits for master, slave, and configuration
 * \brief Converts the detected frequencies into bits for master, slave, and configuration.
 * 
 * This function interprets the frequencies detected in the DTMF signal and converts them into bits.
 * It uses the interpret_bits function to determine the bit values based on the presence of specific frequencies.
 */
struct_tone_bits bit_coder(struct_tone_frequencies tones) 
{
    int master_bit = interpret_bits(tones.master);
    int slave_bit  = interpret_bits(tones.slave);
    int config_bit = interpret_bits(tones.configuration);

    struct_tone_bits result;
    result.master = master_bit;
    result.slave = slave_bit;
    result.configuration = config_bit;
    return result;
}

/*! \fn struct_out_tones frequency_coder(int bit, int role)
 * \param bit An integer representing the bit to be encoded (0 or 1)
 * \param role An integer representing the role (0 for master, 1 for slave, 2 for configuration)
 * \returns A struct_out_tones object containing the encoded frequencies
 * \brief Encodes a bit into specific frequencies based on the role.
 * 
 * This function maps the input bit and role to specific frequency pairs. It returns a struct_out_tones
 * object containing the two frequencies corresponding to the input parameters.
 */
struct_out_tones frequency_coder(int bit, int role){
    struct_out_tones out_tones;
    out_tones.tones[0] = -1;
    out_tones.tones[1] = -1;

    if (role == 0) { // Master
        if (bit == 0) {
            out_tones.tones[0] = 1000;   // f1
            out_tones.tones[1] = 2000;  // f2
        } else if (bit == 1) {
            out_tones.tones[0] = 2000;   // f3
            out_tones.tones[1] = 3000;  // f4
        }
    } else if (role == 1) { // Slave
        if (bit == 0) {
            out_tones.tones[0] = 2000;   // f5
            out_tones.tones[1] = 5500;  // f6
        } else if (bit == 1) {
            out_tones.tones[0] = 5500;   // f7
            out_tones.tones[1] = 9000;  // f8
        }
    } else if (role== 2) { // Configuration
        if (bit == 0) {
            out_tones.tones[0] = 3000;   // f9
            out_tones.tones[1] = 7000;  // f10
        } else if (bit == 1) {
            out_tones.tones[0] = 7000;   // f11
            out_tones.tones[1] = 10000; // f12
        }
    }

    return out_tones;
}


#ifdef __cplusplus
}
#endif
