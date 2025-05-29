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
 #include "audio_driver.h"
 //#include <HardwareSerial.h>
 
 #include "global_parameters.h"
 #include "reader.h"
 #include "reader_test.h"
 #include "fft.h"
 #include "decoder.h"
 #include "reading_queue.h"
 #include "sync_controller.h"
 
 extern "C" {
     #include "global_parameters.h"
     
 }
 
 int res;

 /*! \fn void decoder_operations(void)
 * \brief Esegue tutte le operazioni di decodifica
 * \details Gestisce FFT, Goertzel e comparazione frequenze
 */
 
 void decoder_operations() {
    res = detect_tones();
 }
 
 /*! \fn void setup(void)
 * \brief Inizializzazione hardware e software
 */
 void setup() {
     Serial.begin(115200);


     reading_queue_init();
 
     sync_controller_init();
     /*Inizializzazione audio driver I2S*/
     audio_init();
     /* Inizializzazione reader DMA */
     reader_init();  
     //reader_test_init();
  }
 
 int temp_counter = 0; //da rimuovere
 /*! \fn void loop(void)
 * \brief Loop principale
 */
 void loop() {
     if(data_ready) {
         decoder_operations();
         temp_counter++;
         data_ready = 0;
     }
     //play_tone(300);
 
 }