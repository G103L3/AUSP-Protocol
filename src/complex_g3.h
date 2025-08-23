/*! \file complex_g3.h
* \brief Custom complex number implementation
*
* This implements a narrow subset of the \a complex.h file in the C standard library, thus conforming to stock C89.
*/

#ifndef COMPLEX_G3_H
#define COMPLEX_G3_H

#include <math.h>

/* Our Headers */
#include "complex_g3.h"
#include "global_parameters.h"

/*! \typedef complex_g3_t
* \brief Custom complex number implementation
*/
typedef struct complex_g3_t
{
	float re;	/*!< Real component of a complex number */
	float im;	/*!< Imaginary component of a complex number */
} complex_g3_t;

/* Function Definitions */

/*! \fn complex_g3_t complex_from_polar (double r, double theta_radians)
* \param r Magnitude of the input polar coordinate
* \param theta_radians Angle/Phase of the input polar coordinate
* \returns A complex number using the input information
* \brief Converts to a \p complex_g3_t from a polar coordinate
*/
complex_g3_t complex_from_polar(double r, double theta_radians);

/*! \fn double complex_magnitude (complex_g3_t c)
* \param c Complex number
* \returns The magnitude of the input complex number
* \brief Computes the magnitude of a complex number
*/
double complex_magnitude(complex_g3_t c);

/*! \fn double complex_decibels (complex_g3_t c)
* \param c Complex number
* \returns The decibels of the input complex number
* \brief Computes the decibels of a complex number
*/
double complex_decibels(complex_g3_t c);


/*! \fn complex_g3_t complex_add (complex_g3_t left, complex_g3_t right)
* \param left LHS complex number
* \param right RHS complex number
* \returns The result of the addition
* \brief Adds two complex numbers together
*/
complex_g3_t complex_add(complex_g3_t left, complex_g3_t right);

/*! \fn complex_g3_t complex_sub (complex_g3_t left, complex_g3_t right)
* \param left LHS complex number
* \param right RHS complex number
* \returns The result of the subtraction
* \brief Subtracts one complex number (RHS) from another (LHS)
*/
complex_g3_t complex_sub(complex_g3_t left, complex_g3_t right);

/*! \fn complex_g3_t complex_mult(complex_g3_t left, complex_g3_t right)
* \param left LHS complex number
* \param right RHS complex number
* \returns The result of the multiplication
* \brief Multiplies two complex numbers together
*/
complex_g3_t complex_mult(complex_g3_t left, complex_g3_t right);

#endif
