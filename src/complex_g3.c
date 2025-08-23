/*! \file complex_g3.c
* \brief Functions for complex_g3.h
*/

#include "complex_g3.h"

complex_g3_t complex_from_polar(float r, float theta_radians)
{
    complex_g3_t result;

    result.re = r * cosf(theta_radians);
    result.im = r * sinf(theta_radians);

    return result;
}

float complex_magnitude(complex_g3_t c)
{
    return sqrtf(c.re * c.re + c.im * c.im);
}

float complex_decibels(complex_g3_t c)
{
    return 20.0f * log10f(complex_magnitude(c));
}

complex_g3_t complex_add(complex_g3_t left, complex_g3_t right)
{
    complex_g3_t result;

    result.re = left.re + right.re;
    result.im = left.im + right.im;

    return result;
}

complex_g3_t complex_sub(complex_g3_t left, complex_g3_t right)
{
    complex_g3_t result;

    result.re = left.re - right.re;
    result.im = left.im - right.im;

    return result;
}

complex_g3_t complex_mult(complex_g3_t left, complex_g3_t right)
{
    complex_g3_t result;

    result.re = left.re * right.re - left.im * right.im;
    result.im = left.re * right.im + left.im * right.re;

    return result;
}
