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
int ausp_freq[] = {1000, 4000, 8000, 2000, 5500, 9000, 3000, 7000, 10000};
double const freq_tolerance = (double)G_SAMPLE_RATE/(double)G_ARRAY_SIZE; /* Frequency tolerance due to FFT resolution */

void serial_init(unsigned long baudrate);
void serial_write_char(char c);
void serial_write_string(const char* str);
void serial_write_formatted(const char* format, ...);

struct_interpolated_frequency check_active_frequencies(complex_g3_t *data, int  bin_1, int bin_2, int id);
struct_interpolated_frequency interpolate_peak_frequency(complex_g3_t *data, int peak_bin, double sample_rate, int fft_size);




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
		//regress_linear_update(j, amp);
		//serial_write_formatted(">Spectrum:%f:%f|xy\n", freq, amp);
		//serial_write_formatted(">Bins Spectrum:%d:%f|xy\n", j, amp, freq_tolerance);
		double dynamic_amplitude_threshold = (-301.751324*j)+48531.689491;
		if (amp > dynamic_amplitude_threshold) 
		{
			serial_write_formatted("Debug: Freq: %f Amplitude: %f  Threshold: %f \n", freq, amp, dynamic_amplitude_threshold);

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
				detected_freq = interpolate_peak_frequency(data, j, FS, NN);
				detected_freq.dynamic_amplitude_threshold = dynamic_amplitude_threshold;
				detected_freq.work = 1;
				
				/*serial_write_formatted("Detected Freq: %.2f Hz  Amp: %.2f \n", 
									detected_freq.frequency, detected_freq.estimated_amplitude);*/
				
				// Verifica se la frequenza rilevata è vicina alla frequenza target e che la amplitude stimata sia maggiore del threshold
				serial_write_formatted("Debug: Detected amp: %f diff_freq: %f tolerance: %f threshold: %f\n", detected_freq.estimated_amplitude, fabs(detected_freq.frequency - ausp_freq[id]), freq_tolerance, dynamic_amplitude_threshold);

				return detected_freq;

			}


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