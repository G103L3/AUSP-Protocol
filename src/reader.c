/*! \file reader.c
 * \author Gioele Giunta (modifiche minime per I2S-ADC/DC removal)
 * \version 2.1
 * \since 25 Aug 2025
 * \brief Functions for reader.h on ESP32 (I2S ADC, DC removal coerente con I2S)
 */

 #ifdef __cplusplus
 extern "C" {
 #endif
 
 #include "reader.h"
 #include <string.h>
 #include <freertos/FreeRTOS.h>
 #include <freertos/task.h>
 #include <driver/i2s.h>
 #include <driver/adc.h>
 #include <esp_adc_cal.h>
 #include <esp_system.h>
 #include <esp_log.h>
 
 /* Buffer declarations */
 static complex_g3_t main_array[ARRAY_ELEMENTS];
 static complex_g3_t secondary_array[ARRAY_ELEMENTS];
 static complex_g3_t *current_data;
 
 complex_g3_t *array_ready;
 volatile int data_ready = 0;
 volatile int status_flag = 1;
 static int counter = 0;
 
 /* Task handle */
 static TaskHandle_t reader_task_handle = NULL;
 
 static const char *TAG = "reader";
 
 /*! \brief Swap current and ready arrays */
 static void swap_array(void) {
     array_ready = current_data;
     current_data = (current_data == main_array) ? secondary_array : main_array;
 }
 
 /*! \brief Task to continuously read ADC values via I2S */
 static void reader_task(void *param) {
     size_t bytes_read;
     uint16_t dma_buffer[DMA_BUFFER_SIZE / 2]; // 2 bytes per sample
 
     // DC estimator (coerente con la catena I2S)
     static float dc_mean = 0.0f;
     const float alpha = 1.0f / 1024.0f; // costante di tempo ~1024 campioni
 
     // Autodetect allineamento a 12 bit (left/right)
     static int shift_bits = 0;      // 0 = già 0..4095, 4 = shift >> 4 se left-aligned
     static bool shift_decided = false;
 
     while (1) {
         i2s_read(I2S_PORT, (void*)dma_buffer, DMA_BUFFER_SIZE, &bytes_read, portMAX_DELAY);
         int samples = bytes_read / 2;
 
         // Decidi una volta lo shift (in base al primo buffer stabile)
         if (!shift_decided && samples > 0) {
             // euristica: se molti campioni hanno i 4 LSB a zero, probabile allineamento a sinistra
             int zerosLSB = 0;
             int checkN = samples < 64 ? samples : 64;
             for (int k = 0; k < checkN; ++k) {
                 if ((dma_buffer[k] & 0x000F) == 0) zerosLSB++;
             }
             if (zerosLSB > (checkN * 3) / 4) { // soglia 75%
                 shift_bits = 4; // >> 4 per portare 12 bit in 0..4095
             } else {
                 shift_bits = 0;
             }
             shift_decided = true;
             ESP_LOGI(TAG, "ADC 12-bit alignment: shift_bits=%d", shift_bits);
         }
 
         for (int i = 0; i < samples; i++) {
             uint16_t s16 = dma_buffer[i];
             if (shift_bits) s16 >>= shift_bits; // ora presumibilmente 0..4095
 
             // Debug grezzo (come da tue stampe)
             //printf("Debug: Raw ADC %u \n", (unsigned)s16);
 
             float x = (float)s16;
 
             // Stima DC dalla stessa catena I2S
             dc_mean += alpha * (x - dc_mean);
 
             float val = x - dc_mean;  // campione centrato ~0 (LSB)
             // Niente *2: la scala te la gestisci a valle se serve
             //printf("Debug: Calibrated ADC %.6f \n", (double)val);
 
             // Scrivi nell'array per FFT
             current_data[counter].re = (double)val;
             current_data[counter].im = 0.0;
             counter++;
 
             if (counter >= ARRAY_ELEMENTS) {
                 data_ready = 1;
                 swap_array();
                 counter = 0;
             }
         }
     }
 }
 
 /*! \brief Reader initialization: configures ADC, I2S, allocates buffers and launches task */
 void reader_init(void) {
     serial_init(115200);
 
     // ---------------------------
     // 1) Config ADC1 coerente
     // ---------------------------
     // Nota: AUDIO_PIN deve essere un canale ADC1 valido (GPIO32-39). Es. ADC1_CHANNEL_0 = GPIO36 (VP).
     adc1_config_width(ADC_WIDTH_BIT_12);
     adc1_config_channel_atten((adc1_channel_t)AUDIO_PIN, ADC_ATTEN_DB_11);
 
     // Caratterizzazione (opzionale — la teniamo per eventuali mV, ma non usata per bias)
     esp_adc_cal_characteristics_t adc_chars;
     esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
 
     // ---------------------------
     // 2) I2S configuration
     // ---------------------------
     i2s_config_t i2s_config = {
         .mode = I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN,
         .sample_rate = SAMPLE_RATE, // es. 48000
         .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
         .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
         .communication_format = I2S_COMM_FORMAT_STAND_I2S,
         .intr_alloc_flags = 0,
         .dma_buf_count = DMA_BUFFERS,
         .dma_buf_len = DMA_BUFFER_SIZE / 2,
         .use_apll = false,
         .tx_desc_auto_clear = true,
     };
 
     i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
     i2s_set_adc_mode(ADC_UNIT_1, (adc1_channel_t)AUDIO_PIN);
 
     // Forza i parametri di clock in modo esplicito (mono, 16-bit)
     i2s_set_clk(I2S_PORT, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
 
     // ---------------------------
     // 3) Abilita ADC via I2S e warm-up
     // ---------------------------
     i2s_adc_enable(I2S_PORT);
 
     // Scarta qualche buffer per stabilizzare offset e DMA
     {
         size_t br;
         uint16_t dumpbuf[DMA_BUFFER_SIZE / 2];
         for (int k = 0; k < 5; k++) {
             i2s_read(I2S_PORT, (void*)dumpbuf, DMA_BUFFER_SIZE, &br, portMAX_DELAY);
         }
     }
 
     // ---------------------------
     // 4) Prep strutture e task
     // ---------------------------
     current_data = main_array;
     array_ready = NULL;
     data_ready = 0;
     counter = 0;
 
     xTaskCreate(reader_task, "reader_task", 4096, NULL, 5, &reader_task_handle);
 
     ESP_LOGI(TAG, "reader_init complete (SR=%d Hz, ADC1 atten=11dB, width=12bit)", SAMPLE_RATE);
 }
 
 #ifdef __cplusplus
 }
 #endif
 