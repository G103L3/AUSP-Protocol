/*! \file leds.h
 * \author Gioele Giunta
 * \version 1.0
 * \since 21<sup>st</sup> May 2025
 */

 #ifndef _LEDS_H_
 #define _LEDS_H_

#ifdef __cplusplus
extern "C" {
#endif
 /* C Library Headers */
 #include <Arduino.h>
 #include "global_parameters.h"
 
 void turn_red(uint8_t val);
 void turn_green(uint8_t val);
 void turn_blue(uint8_t val);
 void turn_off();

#ifdef __cplusplus
}
#endif

 #endif
 
 // ******************************* Gioele Giunta University Of Malta *************************************
 