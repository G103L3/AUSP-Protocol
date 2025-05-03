/*! \file decoder_gtzl.h
* \author Gabriel Carabott
* \brief Identifies two dominant frequencies in a DTMF-compliant signal using the Goertzel technique
*/

#ifndef DECODER_GTZL_H_
#define DECODER_GTZL_H_

/* Our Headers */
#include "global_parameters.h"
#include "gtzl.h"

/*! \fn struct_tone_frequencies decoder_gtzl (double amplitudes[DTMF_FREQ_AMT])
* \param amplitudes[DTMF_FREQ_AMT] Array of amplitudes for the 8 DTMF frequencies; size is enforced through the constant DTMF_FREQ_AMT.
* \returns Two dominant DTMF frequencies; values of -1 for both frequencies ({-1,-1}) indicates an error during function execution or that the input signal is not DTMF compliant.
* \brief Identifies the two dominant frequencies of an input signal represented through the amplitudes of DTMF frequencies within it
*
* DTMF compliancy is checked at this stage.
*/
struct_tone_frequencies
decoder_gtzl
(
double amplitudes[DTMF_FREQ_AMT]
);

amplitude_profile set_pass_band_filter(double amplitudes[DTMF_FREQ_AMT]);
#endif
