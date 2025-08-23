/*! \file decoder.c
* \brief Functions for decoder.h
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "decoder.h"
#include "string.h"
#include "leds.h"
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//Test
double sum_bins = 0, sum_amp = 0, sum_bins2 = 0, sum_bins_amp = 0, slope = 0, intercept = 0;
int regress_count = 0;


double const T = 1.0 / FS;  /* Sampling interval */
<<<<<<< Updated upstream
=======
/* Amplitude thresholds */
double amplitude_threshold[] = {50000, 50000, 50000};
>>>>>>> Stashed changes

/* Frequenze utilizzate per la decodifica */
static const int ausp_freq[] = {1000, 4000, 8000, 2000, 5500, 9000, 3000, 7000, 10000};
static const int NUM_AUSP_FREQ = sizeof(ausp_freq) / sizeof(int);

/* Stima del rumore di fondo per ogni frequenza (adattata dinamicamente) */
static double noise_floor[9] = {0};

double const freq_tolerance = (double)G_SAMPLE_RATE/(double)G_ARRAY_SIZE; /* Frequency tolerance due to FFT resolution */

void serial_init(unsigned long baudrate);
void serial_write_char(char c);
void serial_write_string(const char* str);
void serial_write_formatted(const char* format, ...);

struct_interpolated_frequency check_active_frequencies(complex_g3_t *data, int  bin_1, int bin_2, int id);
struct_interpolated_frequency interpolate_peak_frequency(complex_g3_t *data, int peak_bin, double sample_rate, int fft_size);

/* Goertzel semplificato per calcolare l'ampiezza di una singola frequenza */
static double goertzel_magnitude(const complex_g3_t *data, int len, double target_freq) {
    double s_prev = 0.0;
    double s_prev2 = 0.0;
    double normalized_frequency = (2.0 * M_PI * target_freq) / FS;
    double coeff = 2.0 * cos(normalized_frequency);

    for (int i = 0; i < len; i++) {
        double s = data[i].re + coeff * s_prev - s_prev2;
        s_prev2 = s_prev;
        s_prev = s;
    }

<<<<<<< Updated upstream
    return s_prev2 * s_prev2 + s_prev * s_prev - coeff * s_prev * s_prev2;
}



=======
>>>>>>> Stashed changes

/*! \fn struct_tone_frequencies decode_ausp(complex_g3_t *data)
 * \param data Array of complex numbers representing the FFT output
 * \returns A struct_tone_frequencies containing the decoded frequencies
 * \brief Decodes the AUSP frequencies from the FFT output
 * 
 * This function checks for specific frequencies in the FFT output and returns a struct_tone_frequencies
 * containing the detected frequencies for master, slave, and configuration.
 */
struct_tone_frequencies decode_ausp(complex_g3_t *data) 
{
	struct_tone_frequencies decoded_tones;
	serial_init(115200);
	int results_[3][3] = 
	{
		{0, 0, 0},
		{0, 0, 0},
		{0, 0, 0}
	};

<<<<<<< Updated upstream
        turn_off();

        for (int i = 0; i < NUM_AUSP_FREQ; i++) {
                double amp = goertzel_magnitude(data, NN, ausp_freq[i]);
=======
	turn_off();

	for (int i = 0; i < sizeof(ausp_freq)/sizeof(int); i++) {
		int range_start = floor(ausp_freq[i]/(freq_tolerance));
		int range_end = range_start+1;
		struct_interpolated_frequency frequencies = check_active_frequencies(data, range_start, range_end, i);
		if(frequencies.work){
			serial_write_formatted("Debug: Freq: %f Amp: %f Threshold: %f\n", frequencies.frequency, frequencies.estimated_amplitude, frequencies.dynamic_amplitude_threshold);
			if ((fabs(frequencies.frequency - ausp_freq[i]) <= freq_tolerance) && (frequencies.estimated_amplitude > frequencies.dynamic_amplitude_threshold)) {
				results_[i / 3][i % 3] = ausp_freq[i];
				turn_blue(1);
				serial_write_formatted("Debug: freq %f amp: %f \n", frequencies.frequency, frequencies.estimated_amplitude);
			} else {
				turn_red(1);
				results_[i / 3][i % 3] = -1;
			}
		}
	}
>>>>>>> Stashed changes

                /* Aggiorna la stima del rumore di fondo (EWMA) */
                if (noise_floor[i] == 0) {
                        noise_floor[i] = amp;
                } else {
                        noise_floor[i] = (noise_floor[i] * 0.9) + (amp * 0.1);
                }

                double threshold = noise_floor[i] * 3.0 + 1000.0; /* margine per rumore */

                serial_write_formatted("Debug: freq %d amp: %f thr: %f\n", ausp_freq[i], amp, threshold);

                if (amp > threshold) {
                        results_[i / 3][i % 3] = ausp_freq[i];
                        turn_blue(1);
                } else {
                        results_[i / 3][i % 3] = -1;
                        turn_red(1);
                }
        }

        memcpy(decoded_tones.master, results_[0], 3 * sizeof(int));
        memcpy(decoded_tones.slave, results_[1], 3 * sizeof(int));
        memcpy(decoded_tones.configuration, results_[2], 3 * sizeof(int));


        return decoded_tones;
}

/*! \struct struct_interpolated_frequency
 * \brief Structure to hold the interpolated frequency and its properties
 * 
 * This structure contains the frequency, estimated amplitude, dynamic amplitude threshold,
 * and a work flag indicating if the frequency was successfully detected.
 */
struct_interpolated_frequency check_active_frequencies(complex_g3_t *data, int  bin_1, int bin_2, int id){
<<<<<<< Updated upstream
	int i, j;
	struct_interpolated_frequency detected_freq;
	detected_freq.work = 0;
	detected_freq.frequency = -1.0;  // Default value indicating no frequency detected
	detected_freq.estimated_amplitude = -1.0;  // Default value indicating no amplitude detected
	detected_freq.dynamic_amplitude_threshold = -1.0;  // Default value indicating no threshold detected														
	for (j = bin_1; j <=bin_2; j++) 
	{
		double freq = (double)(FS * j) / NN;
		double amp = complex_magnitude(data[j]);
		if(G_LINEAR_REGRESSION_MODE == 0){

			
			//serial_write_formatted(">Spectrum:%f:%f|xy\n", freq, amp);
			//serial_write_formatted(">Bins Spectrum:%d:%f|xy\n", j, amp, freq_tolerance);
			double dynamic_amplitude_threshold = (-301.751324*j)+48531.689491;
			//double dynamic_amplitude_threshold = (-219.828502*j)+21204.707830;
=======
        int i, j;
        struct_interpolated_frequency detected_freq;
        detected_freq.work = 0;
        detected_freq.frequency = -1.0;  // Default value indicating no frequency detected
        detected_freq.estimated_amplitude = -1.0;  // Default value indicating no amplitude detected
        detected_freq.dynamic_amplitude_threshold = -1.0;  // Default value indicating no threshold detected

        /* Calcola il livello di rumore medio sull'intero spettro per adattare la soglia
         * Questo approccio rende la rilevazione più robusta ai cambiamenti di ampiezza
         * e al rumore ambientale. */
        double sum_noise = 0.0;
        for (i = 0; i < NN; i++) {
                sum_noise += complex_magnitude(data[i]);
        }
        double noise_floor = sum_noise / NN;
        double dynamic_amplitude_threshold = noise_floor * 8.0; // Fattore empirico
>>>>>>> Stashed changes

        for (j = bin_1; j <= bin_2; j++) {
                double freq = (double)(FS * j) / NN;
                double amp = complex_magnitude(data[j]);
                if (G_LINEAR_REGRESSION_MODE == 0) {
                        if (amp > dynamic_amplitude_threshold) {
                                serial_write_formatted("Debug: Freq: %f Amplitude: %f  Threshold: %f \n", freq, amp, dynamic_amplitude_threshold);

                                //Capisce se il bin che sto analizzando è il maggiore di tutto il suo intorno
                                for (i = j - 6; i < j + 6 && complex_magnitude(data[i]) <= amp; i++) {
                                        // scansione dell'intorno per confermare il picco
                                }

                                if (i == j + 6) {
                                        // Usa l'interpolazione per ottenere una stima più precisa della frequenza
                                        detected_freq = interpolate_peak_frequency(data, j, FS, NN);
                                        detected_freq.dynamic_amplitude_threshold = dynamic_amplitude_threshold;
                                        detected_freq.work = 1;

                                        serial_write_formatted("Debug: Detected amp: %f diff_freq: %f tolerance: %f threshold: %f\n",
                                                                detected_freq.estimated_amplitude,
                                                                fabs(detected_freq.frequency - ausp_freq[id]),
                                                                freq_tolerance,
                                                                dynamic_amplitude_threshold);

                                        return detected_freq;
                                }
                        }
                } else if (G_LINEAR_REGRESSION_MODE == 2) {
                        regress_linear_update(j, amp);
                }
        }
        return detected_freq;
}

/*! \struct struct_interpolated_frequency
 * \brief Structure to hold the interpolated frequency and its properties
 * 
 * This structure contains the frequency, estimated amplitude, dynamic amplitude threshold,
 * and a work flag indicating if the frequency was successfully detected.
 */
struct_interpolated_frequency interpolate_peak_frequency(complex_g3_t *data, int peak_bin, double sample_rate, int fft_size) {
    // Evita di fare interpolazione se il picco è al bordo dello spettro
    if (peak_bin <= 0 || peak_bin >= fft_size - 1) {
			//Formattazione del Return
			struct_interpolated_frequency frequency;
			frequency.frequency = peak_bin * sample_rate / fft_size;
			frequency.estimated_amplitude = complex_magnitude(data[peak_bin]);
        return frequency;
    }
    
    // Ottieni le ampiezze dei tre bin
    double alpha = complex_magnitude(data[peak_bin-1]);
    double beta = complex_magnitude(data[peak_bin]);
    double gamma = complex_magnitude(data[peak_bin+1]);

	serial_write_formatted("Debug: Alpha %f Beta %f Gamma %f\n", alpha, beta, gamma);
    
    // Formula dell'interpolazione parabolica
    double p = 0.5 * (alpha - gamma) / (alpha - 2 * beta + gamma);
    
    // Limita p nell'intervallo [-0.5, 0.5] per evitare risultati anomali
    if (p < -0.5) p = -0.5;
    if (p > 0.5) p = 0.5;
    
    // Calcola la frequenza interpolata
    double interpolated_bin = peak_bin + p;
    double interpolated_freq = interpolated_bin * sample_rate / fft_size;
    
	// Calcola l'ampiezza interpolata
	double interpolated_amplitude = beta - 0.25 * (alpha - gamma) * p;

	//Formattazione del Return
	struct_interpolated_frequency frequency;
	frequency.frequency = interpolated_freq;
	frequency.estimated_amplitude = interpolated_amplitude;
    return frequency;
}


void regress_linear_update(const int bin, const double amplitude) {
    sum_bins += bin;
    sum_amp += amplitude;
    sum_bins2 += bin * bin;
    sum_bins_amp += bin * amplitude;
	regress_count++;
    
    double denominator = regress_count * sum_bins2 - sum_bins * sum_bins;
    if (denominator == 0) {
        serial_write_string("Error: denominator zero, probably all bin values are equal\n");
    }
    
    slope = (regress_count * sum_bins_amp - sum_bins * sum_amp) / denominator;
    intercept = (sum_amp - (slope) * sum_bins) / regress_count;
	serial_write_formatted("Slope: %f  Intercept: %f \n", slope, intercept);
}


#ifdef __cplusplus
}
#endif