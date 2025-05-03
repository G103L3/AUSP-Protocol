#ifndef READER_H_
#define READER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#include <driver/i2s.h>
#include <driver/adc.h>

#include "complex_g3.h"
#include "global_parameters.h"

#define ARRAY_ELEMENTS G_ARRAY_SIZE
#define SAMPLE_RATE G_SAMPLE_RATE
#define I2S_PORT I2S_NUM_0
#define DMA_BUFFER_SIZE 1024
#define DMA_BUFFERS 4
#define VREF (3.3)
#define ADC_MASK G_MAX_AMPLITUDE
#define AUDIO_PIN ADC1_CHANNEL_0  // GPIO36 (VP)

extern volatile int data_ready;
extern volatile int status_flag;
extern complex_g3_t *array_ready;

void reader_init(void);

#ifdef __cplusplus
}
#endif

#endif // READER_H_
