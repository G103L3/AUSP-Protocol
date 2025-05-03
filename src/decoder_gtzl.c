/*! \file decoder_gtzl.c
* \author Gabriel Carabott
* \brief Functions for decoder_gtzl.h
*/

/* Our Headers */
#include "global_parameters.h"
#include "gtzl.h"
#include "decoder_gtzl.h"

const double AMPL_THRS = 1200000000;	/* The amplitude threshold for the amplitude of a DTMF frequency */
const unsigned short DTMF_FREQ_AMT_D2 = DTMF_FREQ_AMT / 2;
const unsigned short DTMF_FRQS[DTMF_FREQ_AMT] = {697, 770, 852, 941, 1209, 1336, 1477, 1633, 2000, 4000};

void serial_init(unsigned long baudrate);
void serial_write_char(char c);
void serial_write_string(const char* str);
void serial_write_formatted(const char* format, ...);


// Function to verify if configuration frequencies are above a defined threshold
// It checks the amplitude values from Goertzel and ensures they exceed 600 million
// The threshold of 600 million is an experimentally determined value based on measurements taken at 20 meters
amplitude_profile set_pass_band_filter(double amplitudes[DTMF_FREQ_AMT])
{
    amplitude_profile profile = {0, 0, 0, 0, 0};
    const double MIN_THRESHOLD = 500000000.0; // Experimental value from 20m measurement
    
    double frequency_one_min = amplitudes[8];
    double frequency_one_max = amplitudes[8];
    double frequency_two_min = amplitudes[9];
    double frequency_two_max = amplitudes[9];
	
	serial_write_formatted(" Amp: %f  Freq: %d\n", amplitudes[8], DTMF_FRQS[8]);
	serial_write_formatted(" Amp: %f  Freq: %d\n", amplitudes[9], DTMF_FRQS[9]);

    
    if (amplitudes[8] < MIN_THRESHOLD || amplitudes[9] < MIN_THRESHOLD) {
		pinMode(A6, OUTPUT);
		analogWrite(A6, 255);
		pinMode(A7, OUTPUT);
		analogWrite(A7, 0);
        return profile; // One or both frequencies do not exceed the required threshold
    }
    
    // Ensure bottom <= top for thresholds
    profile.estimated_threshold_low_bottom = (frequency_one_min < frequency_one_max) ? frequency_one_min : frequency_one_max;
    profile.estimated_threshold_low_top = (frequency_one_max > frequency_one_min) ? frequency_one_max : frequency_one_min;
    profile.estimated_threshold_high_bottom = (frequency_two_min < frequency_two_max) ? frequency_two_min : frequency_two_max;
    profile.estimated_threshold_high_top = (frequency_two_max > frequency_two_min) ? frequency_two_max : frequency_two_min;
    
    profile.new_profile = 1;
	pinMode(A7, OUTPUT);
	analogWrite(A7, 255);
	pinMode(A6, OUTPUT);
	analogWrite(A6, 0);
    return profile;
}


struct_tone_frequencies
decoder_gtzl
(
double amplitudes[DTMF_FREQ_AMT]
)
{
	/* This contains the dominant frequencies from the received array of frequency amplitudes.
	* Values of -1 indicate an error or that a DTMF tone has not been identified.
	*/
	struct_tone_frequencies frqs_tone_return = {-1,-1};

	/* The below two booleans indicate if a low and high frequency has been found, which is above the desired threshold.
	* These will also be used to check if there are multiple DTMF tone pairs, which results in a signal that is not DTMF.
	* Finally, unsigned char is used to signify a boolean, since stdbool.h is not available in C89
	*/
	unsigned int found_lo = 0;
	unsigned int found_hi = 0;

	unsigned short frq_tmp_lo = 0;
	unsigned short frq_tmp_hi = 0;

	unsigned short i = 0;	/* Generic Iterator */

	/* Ideal threshold for amps probably in the range [500, 800] */
	for (i = 0; i < DTMF_FREQ_AMT_D2; i++)
	{
		if (amplitudes[i] > AMPL_THRS)
		/* This signifies the low frequencies.
		* The amplitudes of the following frequencies are held between i = 0 and i = 3: {697, 770, 852, 941}
		*/
		{
			if (found_lo)
			/* A low frequency with an amplitude higher than the threshold has been found */
			{
				/* There are more than two dominant low DTMF frequencies, which no longer remains a DTMF signal. */
				return frqs_tone_return;
			}
			else
			{
				frq_tmp_lo = DTMF_FRQS[i];
				found_lo = 1;
				serial_write_formatted(" Amp: %f  Freq: %d\n", amplitudes[i], frq_tmp_lo);

			}
		}
		
		if (amplitudes[4 + i] > AMPL_THRS)
		/* This signifies the high frequencies.
		* The amplitudes of the following frequencies are held between i = 4 and i = 7: {1209, 1336, 1477, 1633}
		*/
		{
			if (found_hi)
			/* A high frequency with an amplitude higher than the threshold has been found */
			{
				/* There are more than two dominant high DTMF frequencies, which no longer remains a DTMF signal. */
				return frqs_tone_return;
			}
			else
			{
				frq_tmp_hi = DTMF_FRQS[4 + i];
				found_hi = 1;
				serial_write_formatted(" Amp: %f  Freq: %d\n", amplitudes[4+i], frq_tmp_hi);
			}
		}
	}

	frqs_tone_return.low = (int) frq_tmp_lo;
	frqs_tone_return.high = (int) frq_tmp_hi;

	return frqs_tone_return;
}
