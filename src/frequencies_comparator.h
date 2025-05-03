/*! \file frequencies_comparator.h
 * \author Gioele Giunta
 * \version 1.0
 * \since 21<sup>st</sup> March 2024
 * \brief Function that identifies a key, handles noise, multitone and overflow; and manages a sequence of characters to be projected in print.
 */

#ifndef _FREQUENCIES_COMPARATOR_H_
#define _FREQUENCIES_COMPARATOR_H_

/* C Library Headers */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Our Headers */
#include "global_parameters.h"

/*! \fn int frequencies_comparator(struct_tone_frequencies tone_frequencies, char *last_char, char *sequence)
* \param tone_frequencies The struct containing the high and low frequencies
* \param *last_char The pointer to the last_char, used to determine if there is a repetition or not
* \param *sequence The char Array where character will be saved
* \returns The status of operation 1 -> Tone detected, 0 -> Noise Detected/No operation by main required, <0 -> Error detected Operations from main Required
* \brief Compares the input frequencies to the DTMF frequency matrix and determines the corresponding key.
*
* This function takes in a struct containing the high and low frequencies of a DTMF signal, pointers to the last character detected and the character sequence. 
* It then determines the key that corresponds to the input frequencies and adds it to the character sequence, if it is a new character, also handles the case where OVERFLOW is detected. 
* The function also handles the case where no DTMF signal is detected or multiple DTMF signals are detected.
*/
int frequencies_comparator(struct_tone_frequencies tone_frequencies, char *last_char, char *sequence);

#endif

// ******************************* Gioele Giunta University Of Malta *************************************
