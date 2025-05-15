#ifdef __cplusplus
extern "C" {
#endif

#include "audio_driver.h"
#include <math.h>
#include <string.h>



/**
 * @brief Initializes the I2S peripheral for audio playback.
 *
 * Here I configure the ESP32's I2S driver to operate in master transmit mode.
 * The sample rate is set to 44100 Hz, data width to 16-bit, and communication format to standard I2S MSB.
 * Pin numbers for BCLK, WS, and DATA are set according to my hardware setup.
 * DMA buffer configuration is also defined to manage audio stream transfer.
 */
void audio_init() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = G_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,  // stereo mode
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_DATA_PIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM, &pin_config);
}

/**
 * @brief Generates a stereo sine wave buffer and sends it through I2S.
 *
 * I create a buffer that represents one complete sine wave cycle of the given frequency.
 * This buffer is generated using the sine function, scaled to a 16-bit signed integer range.
 * Then I use i2s_write() to send this buffer to the MAX98357A via I2S.
 *
 * @param frequency The frequency of the tone to be generated, in Hz.
 */
void play_tone(int frequency) {
    // Calculate how many samples are needed to produce one cycle of the desired frequency.
    const int samples_per_cycle = G_SAMPLE_RATE / frequency;

    // Stereo buffer: each sample has two values (left and right).
    const int buffer_size = samples_per_cycle * 2;
    int16_t buffer[buffer_size];

    // Generate sine wave values for each sample.
    // Amplitude set to 3000 to avoid clipping when converted to analog.
    for (int i = 0; i < samples_per_cycle; i++) {
        int16_t sample = (int16_t)(3000 * sin(2 * PI * i / samples_per_cycle));

        // Write same sample value to both left and right channels for a mono-like effect.
        buffer[2 * i] = sample;       // Left channel
        buffer[2 * i + 1] = sample;   // Right channel
    }

    // Transmit generated buffer via I2S to the amplifier.
    size_t bytes_written = 0;
    i2s_write(I2S_NUM, buffer, sizeof(buffer), &bytes_written, portMAX_DELAY);
}

#ifdef __cplusplus
}
#endif