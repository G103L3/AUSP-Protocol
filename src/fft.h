/*! \file fft.h
* \brief Performs the FFT of an array of voltage levels
*/

#ifndef FFT_H
#define FFT_H

#ifdef __cplusplus
extern "C" {
#endif

/* C Library Headers */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Driver Headers */

/* Our Headers */
#include "complex_g3.h"
#include "global_parameters.h"

#define NN G_ARRAY_SIZE  // Define the maximum FFT size, must be a power of 2

/**
 * \brief Simplified interface to perform FFT on a complex array of voltage levels
 * \param x Pointer to a complex array of voltage levels
 * \param N The number of samples in the array, should be a power of two
 * \returns A pointer to a global array representing the frequency spectrum of the input signal, note that this is not thread-safe
 * 
 * This function provides a simplified interface to perform the FFT, suitable for straightforward use cases. It wraps the operations of computing twiddle factors and executing the FFT into a single call, managing all intermediate storage internally. This is ideal for single-threaded applications where ease of use is more critical than modularity.
*/
complex_g3_t *FFT_simple(complex_g3_t *x, int N);

#ifdef __cplusplus
}
#endif

#endif
