#ifndef AUDIO_DRIVER_H
#define AUDIO_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <driver/i2s.h>

#include "global_parameters.h"

#define I2S_NUM         I2S_NUM_1         /**< I2S peripheral number */
#define PI 3.14159265                     /**< Pi constant */

/** I2S pin configuration - customized for my specific wiring setup */
#define I2S_BCK_PIN     33    /**< Bit Clock pin (BCLK) connected to GPIO33 */
#define I2S_WS_PIN      32    /**< Word Select (LRC) pin connected to GPIO32 */
#define I2S_DATA_PIN    14    /**< Serial Data (DIN) pin connected to GPIO14 */

/**
 * @brief Initializes the I2S driver with predefined configuration for audio output.
 *
 * This function sets up the ESP32's I2S peripheral for transmitting audio data to the MAX98357A.
 * The configuration includes sample rate, bit depth, and pin assignments for the BCLK, LRC, and DATA signals.
 */
void audio_init();

/**
 * @brief Generates and plays a sine wave tone of the specified frequency through the MAX98357A.
 *
 * This function creates a stereo sine wave buffer at the requested frequency and sends it via I2S.
 * It blocks while transmitting one full cycle of the waveform.
 *
 * @param frequency Frequency in Hz of the tone to generate and play.
 */
void play_tone(int frequency);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_DRIVER_H
