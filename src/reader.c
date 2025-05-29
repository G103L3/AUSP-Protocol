/*! \file reader.c
 * \author Gioele Giunta
 * \version 2.0
 * \since 3<sup>rd</sup> May 2025
 * \brief Functions for reader.h on ESP32
 */

#ifdef __cplusplus
extern "C" {
#endif

 #include "reader.h"
 #include <string.h>
 #include <freertos/FreeRTOS.h>
 #include <freertos/task.h>
 #include <esp_adc_cal.h>
 #include "reading_queue.h"
 #include "global_parameters.h"


 volatile int data_ready = 0;
 volatile int status_flag = 1;
 static int counter = 0;
 static double bias = 0.0;
 
 /* Task handle */
 static TaskHandle_t reader_task_handle = NULL;
 
 /*! \brief Task to continuously read ADC values via I2S */
 static void reader_task(void *param) {
     size_t bytes_read;
     uint16_t dma_buffer[DMA_BUFFER_SIZE / 2]; // 2 bytes per sample
 
     while (1) { //Purtroppo non si può mettere a 0 e quindi non si può avere controllo con status flag
         i2s_read(I2S_PORT, (void*)dma_buffer, DMA_BUFFER_SIZE, &bytes_read, portMAX_DELAY);
 
         int samples = bytes_read / 2;
         for (int i = 0; i < samples; i++) {
             double val = ((double)dma_buffer[i] - bias) * 2.0;
             //serial_write_formatted("ADC: %f \n", val);

             complex_g3_t sample;  // Stack-allocated variable
             sample.re = val;
             sample.im = 0.0;
             reading_queue_enqueue(&sample);  // Pass address of the stack variable
             
             counter++;
             if(counter == 256){
                //serial_write_formatted("add = %d \n", counter);
                data_ready = 1;
                counter = 0;
             }
         }
     }
 }
 
 /*! \brief Reader initialization: configures ADC, I2S, allocates buffers and launches task */
 void reader_init(void) {
    serial_init(115200);
     // ADC calibration
     uint32_t sum = 0;
     for (int i = 0; i < 1024; i++) {
         sum += adc1_get_raw(AUDIO_PIN);
         ets_delay_us(10);
     }
     bias = sum / 1024.0;

     esp_adc_cal_characteristics_t adc_chars;
     esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
 
     // I2S configuration
     i2s_config_t i2s_config = {
         .mode = I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN,
         .sample_rate = SAMPLE_RATE, //The sample rate defined as 48kHz
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
     i2s_set_adc_mode(ADC_UNIT_1, AUDIO_PIN);
 
     // Bias calibration

 
     // Enable ADC in I2S mode
     i2s_adc_enable(I2S_PORT);
 
     // Prepare data triggers
     data_ready = 0;
     counter = 0;
 
     // Launch reader task
     xTaskCreate(reader_task, "reader_task", 4096, NULL, 5, &reader_task_handle);
 }
 #ifdef __cplusplus
}
#endif 