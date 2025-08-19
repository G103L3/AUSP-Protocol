/*! \file bit_coder.h
 * \author Gioele Giunta
 * \version 1.0
 * \since 21<sup>st</sup> March 2024
 * \brief Function that identifies a key, handles noise, multitone and overflow; and manages a sequence of characters to be projected in print.
 */

#ifndef _bit_coder_H_
#define _bit_coder_H_

#ifdef __cplusplus
extern "C" {
#endif

/* C Library Headers */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Our Headers */
#include "global_parameters.h"

struct_tone_bits bit_coder(struct_tone_frequencies tones);
struct_out_tones frequency_coder(int bit, int role);

#ifdef __cplusplus
}
#endif

#endif

// ******************************* Gioele Giunta University Of Malta *************************************
