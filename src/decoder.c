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

//Test
double sum_bins = 0, sum_amp = 0, sum_bins2 = 0, sum_bins_amp = 0, slope = 0, intercept = 0;
int regress_count = 0;


double const T = 1.0 / FS;  /* Sampling interval */
/* Amplitude thresholds */
double amplitude_threshold[] = {50000, 50000, 50000};

/* AUSP FREQUENCIES */
int ausp_freq[] = {1000, 2000, 3000, 4000, 5500, 7000, 8000, 9000, 10000};
double const freq_tolerance = (double)G_SAMPLE_RATE/(double)G_ARRAY_SIZE; /* Frequency tolerance due to FFT resolution */

void serial_init(unsigned long baudrate);
void serial_write_char(char c);
void serial_write_string(const char* str);
void serial_write_formatted(const char* format, ...);

int* check_active_frequencies(int* results_, complex_g3_t *data, int  bin_1, int bin_2, int id);
struct_interpolated_frequency interpolate_peak_frequency(complex_g3_t *data, int peak_bin, double sample_rate, int fft_size);

/*! \fn struct_tone_frequencies decode_dtmf(complex_g3_t *data)
* \param data Pointer to an array of complex numbers representing the frequency spectrum of a DTMF signal
* \returns A struct_tone_frequencies object containing the dominant low and high frequencies detected in the DTMF signal
* \brief Identifies the potential low and high frequencies from a given DTMF signal using the FFT output provided in the `data` array.
* 
* This function processes the FFT results stored in `data` to detect the presence of specific DTMF frequencies within the permissible 
* frequency tolerance. It calculates the magnitude of each frequency that is within a range of a valid dtmf tone in the FFT output and 
* compares it against an amplitude threshold and frequency tolerance. The function returns the dominant frequencies that are within 
* the specified thresholds. If multiple valid frequencies are found within the tolerance range or if no valid frequency is detected, the 
* function may adjust or set the values to indicate an error or ambiguity in detection.
*/
struct_tone_frequencies decode_dtmf(complex_g3_t *data) 
{
	struct_tone_frequencies result;
	serial_init(115200);
	int results[] = {0, 0, 0};

	turn_off();

	for (int i = 0; i < sizeof(ausp_freq)/sizeof(int); i++) {
		int range_start = floor(ausp_freq[i]/(freq_tolerance));
		int range_end = range_start+1;
		check_active_frequencies(results, data, range_start, range_end, i);
	}

	result.low = results[0];
	result.mid = results[1];
	result.high = results[2];
	

	return result;
}

int* check_active_frequencies(int* results_, complex_g3_t *data, int  bin_1, int bin_2, int id){
	int i, j;

	for (j = bin_1; j <=bin_2; j++) 
	{
		double freq = (double)(FS * j) / NN;
		double amp = complex_magnitude(data[j]);
		//regress_linear_update(j, amp);
		//serial_write_formatted(">Spectrum:%f:%f|xy\n", freq, amp);
		//serial_write_formatted(">Bins Spectrum:%d:%f|xy\n", j, amp, freq_tolerance);
		double dynamic_amplitude_threshold = (-301.751324*j)+48531.689491;
		if (amp > dynamic_amplitude_threshold) 
		{
			serial_write_formatted("Info: Freq: %f Amplitude: %f  Threshold: %f \n", freq, amp, dynamic_amplitude_threshold);

			//serial_write_formatted("Freq: %f Amp: %f Bin: %d Around: ", freq, amp, j);
			//Capisce se il bin che sto analizzando è il maggiore di tutto il suo intorno
			for(i = j-6; i < j + 6 && complex_magnitude(data[i]) <= amp; i++) {				
				//serial_write_formatted("%f, ", complex_decibels(data[i]));
				//Sophisticated system to check if there are a max value, it doesn't need any variable.
				//amp is the bigger if i == j+6
				//serial_write_formatted(">Spectrum (+-)6:%f:%f|xy\n", ((double)(FS * i) / NN), complex_magnitude(data[i]));
			}

			//serial_write_formatted(" I: %d J+6: %d\n", i, (j+6));
			if (i == j+6) 
			{
				//Dato che è quello con ampiezza massima allora FACCIO L'INTERPOLAZIONE per capire se conduce
				//alla vera frequenza che cerco
				// Usa l'interpolazione per ottenere una stima più precisa della frequenza
				struct_interpolated_frequency detected_freq = interpolate_peak_frequency(data, j, FS, NN);
				
				/*serial_write_formatted("Detected Freq: %.2f Hz  Amp: %.2f \n", 
									detected_freq.frequency, detected_freq.estimated_amplitude);*/
				
				// Verifica se la frequenza rilevata è vicina alla frequenza target e che la amplitude stimata sia maggiore del threshold
				serial_write_formatted("Info: Detected amp: %f diff_freq: %f tolerance: %f \n", detected_freq.estimated_amplitude, fabs(detected_freq.frequency - ausp_freq[id]), freq_tolerance);
				if ((fabs(detected_freq.frequency - ((FS*bin_1) / NN)) <= freq_tolerance) && (detected_freq.estimated_amplitude > dynamic_amplitude_threshold)) {
					results_[id] = ausp_freq[id];
					turn_blue(1);
					serial_write_formatted("Info: freq %f amp: %f \n", detected_freq.frequency, detected_freq.estimated_amplitude);
				} else {
					turn_red(1);
					results_[id] = -1;
				}
				

			}
			else if (results_[id] == 0 )
			{
				results_[id] = 0;
			}


		}
	}
	return results_;
}


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

	serial_write_formatted("Info: Alpha %f Beta %f Gamma %f\n", alpha, beta, gamma);
    
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
        return -2;
    }
    
    slope = (regress_count * sum_bins_amp - sum_bins * sum_amp) / denominator;
    intercept = (sum_amp - (slope) * sum_bins) / regress_count;
	serial_write_formatted("Slope: %f  Intercept: %f \n", slope, intercept);
    return 0;  // OK
}


#ifdef __cplusplus
}
#endif