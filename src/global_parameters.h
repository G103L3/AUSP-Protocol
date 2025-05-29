/*! \file global_parameters.h
 * \author Gioele Giunta
 * \version 1.4
 * \since 21<sup>st</sup> April 2024
 * \brief Project wide variables and definitions
 * \note Modificato per ESP32 con DMA e 48kHz sampling
 */
#ifndef GLOBAL_PARAMETERS_H_
#define GLOBAL_PARAMETERS_H_
#include <Arduino.h>



/* 48000 è il nuovo sample rate, 48000 campioni al secondo */
#define G_SAMPLE_RATE 48000

/* Dimensione array calcolata per 0.064 secondi: 0.064 * 48000 = 3072 */
#define G_ARRAY_SIZE 512

/* Dimensione della finestra scorrevole (coda FIFO) di cui poi verranno analizzati solo 1024 elementi a volta*/
#define G_WINDOW_SIZE 2048

/* Massima ampiezza ADC a 12-bit */
#define G_MAX_AMPLITUDE 4096.0

#define G_SEQUENCE_LENGTH 100
#define G_PI 3.14159265358979323846


typedef struct struct_tone_frequencies {
    int low;
    int mid;
    int high;
} struct_tone_frequencies;

typedef struct amplitude_profile {
    int new_profile;
    double estimated_threshold_low_bottom;
    double estimated_threshold_low_top;
    double estimated_threshold_mid_bottom;
    double estimated_threshold_mid_top;
    double estimated_threshold_high_bottom;
    double estimated_threshold_high_top;
} amplitude_profile;



#endif