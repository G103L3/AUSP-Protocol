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
#include "serial_bridge.h"
#include "leds.h"
#include "audio_driver.h"
#include "bit_freq_codec.h"
#include "bit_input_packer.h"
#include "bit_output_packer.h"
#include "sync_controller.h"
//#include <HardwareSerial.h>
 
#include "emit_tones.h"
 
extern "C" {
    #include "global_parameters.h"
}

static BitOutputPacker out_packer;
static struct_out_tones* out_pairs = NULL;
static size_t out_len = 0;
static bool message_sent = false;

 
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
         complex_g3_t* out;
         struct_tone_bits tone_bits;
         out = FFT_simple(array_ready, G_ARRAY_SIZE);
         tone_frequencies = decode_ausp(out);
         serial_write_formatted("Info: Master %d %d %d Slave: %d %d %d Config: %d %d %d \n", tone_frequencies.master[0],tone_frequencies.master[1],tone_frequencies.master[2],tone_frequencies.slave[0],tone_frequencies.slave[1],tone_frequencies.slave[2],tone_frequencies.configuration[0],tone_frequencies.configuration[1],tone_frequencies.configuration[2]);
         tone_bits = bit_coder(tone_frequencies);
         serial_write_formatted("Info: A Master: %d Slave: %d Config: %d \n", tone_bits.master, tone_bits.slave, tone_bits.configuration);
         process_tone_bits(tone_bits);
         
             }
 }
 
 /*! \fn void setup(void)
 * \brief Inizializzazione hardware e software
 */
void setup() {
    Serial.begin(115200);
    memset(sequence, 0, G_SEQUENCE_LENGTH);
 
     /*Inizializzazione audio driver I2S*/
     audio_init();
     /* Inizializzazione reader DMA */
    reader_init();
    sync_controller_init();

    bit_output_packer_init(&out_packer);
    out_pairs = bit_output_packer_pack(&out_packer, "HELLO", 0);
    out_len = out_packer.pair_count;

    sync_time_init();
    wait_for_next_slot();
 

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
    if(!message_sent && out_len > 0) {
        if(is_channel_free()) {
            resync_time();
            wait_for_next_slot();
            emit_tones(out_pairs, out_len);
            message_sent = true;
        }
    }

}