/*! \file main.cpp
 * \brief File principale per ESP32 con acquisizione DMA
 */

 /* C Library Headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Driver Headers */
#include <Arduino.h>

/* Our Headers */
#include "global_parameters.h"
#include "complex_g3.h"
#include "reader.h"
#include "fft.h"
#include "decoder.h"
#include "gtzl.h"
#include "decoder_gtzl.h"
#include "frequencies_comparator.h"
#include "serial_bridge.h"
#include "leds.h"
//#include <HardwareSerial.h>

#include "global_parameters.h"
#include "reader.h"
#include "fft.h"
#include "decoder.h"

extern "C" {
    #include "global_parameters.h"
    #include "frequencies_comparator.h"
}

// Variabili globali
char sequence[G_SEQUENCE_LENGTH];
char last_char = 'N';
int algorithm = 2;
int g_scrolling = 0;
int g_scroll_offset = 0;

/*! \fn void decoder_operations(void)
* \brief Esegue tutte le operazioni di decodifica
* \details Gestisce FFT, Goertzel e comparazione frequenze
*/

void decoder_operations() {
    if(data_ready) {
        struct_tone_frequencies tone_frequencies;
        amplitude_profile profile;
        complex_g3_t* out;
        double amplitudes_gtzl[DTMF_FREQ_AMT];
        int comparator_return;

        if(algorithm == 2) { // FFT 
            out = FFT_simple(array_ready, G_ARRAY_SIZE);
            tone_frequencies = decode_dtmf(out);
            /*serial_write_formatted("FFT: %d-%d-%d\n", 
                tone_frequencies.high, 
                tone_frequencies.mid, 
                tone_frequencies.low);*/
        }
        /*else if(algorithm == 3) { // Goertzel 
            goertzel(array_ready, amplitudes_gtzl);
            tone_frequencies = decoder_gtzl(amplitudes_gtzl);
            serial_write_formatted("Goertzel: %d-%d\n", 
                tone_frequencies.high, 
                tone_frequencies.low);
        }
        else if(algorithm == 4) { /7 Pass Band 
            goertzel(array_ready, amplitudes_gtzl);
            profile = set_pass_band_filter(amplitudes_gtzl);
            serial_write_formatted("LB:%.1f LT:%.1f HB:%.1f HT:%.1f\n",
                profile.estimated_threshold_low_bottom,
                profile.estimated_threshold_low_top,
                profile.estimated_threshold_high_bottom,
                profile.estimated_threshold_high_top);
        }*/

        if(comparator_return == 1) { // Trovato 
            serial_write_string("MATCH: ");
            serial_write_string(sequence);
            serial_write_string("\n");
            if(g_scrolling) {
                g_scroll_offset = strlen(sequence);
            }
        }
    }
}

/*! \fn void setup(void)
* \brief Inizializzazione hardware e software
*/
void setup() {
    Serial.begin(115200);
    memset(sequence, 0, G_SEQUENCE_LENGTH);

    /* Inizializzazione reader DMA */
    reader_init();

    status_flag = 1;
}

/*! \fn void loop(void)
* \brief Loop principale
*/
void loop() {
    if(data_ready) {
        decoder_operations();
        data_ready = 0;
    }
}