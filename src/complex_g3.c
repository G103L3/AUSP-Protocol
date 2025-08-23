/*! \file complex_g3.c
* \brief Functions for complex_g3.h
*/

#include "complex_g3.h"
#include <math.h>

complex_g3_t complex_from_polar(double r, double theta_radians)
{
    complex_g3_t result;

    result.re = (float)(r * cos(theta_radians));
    result.im = (float)(r * sin(theta_radians));

    return result;
}

double complex_magnitude(complex_g3_t c)
{
    return sqrt((double)c.re * c.re + (double)c.im * c.im);
}

double complex_decibels(complex_g3_t c){
    return 20 * log10(complex_magnitude(c));
}

complex_g3_t complex_add(complex_g3_t left, complex_g3_t right)
{
    complex_g3_t result;

    result.re = (float)(left.re + right.re);
    result.im = (float)(left.im + right.im);

    return result;
}

complex_g3_t complex_sub(complex_g3_t left, complex_g3_t right)
{
    complex_g3_t result;

    result.re = (float)(left.re - right.re);
    result.im = (float)(left.im - right.im);

    return result;
}

complex_g3_t complex_mult(complex_g3_t left, complex_g3_t right)
{
    complex_g3_t result;

    result.re = (float)(left.re*right.re - left.im*right.im);
    result.im = (float)(left.re*right.im + left.im*right.re);

    return result;
}
