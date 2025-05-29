#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#include "complex_g3.h"
#include "decoder.h"
#include "fft.h"
#include "reading_queue.h"
#include "sync_controller.h"
#include "global_parameters.h"

/// Starting index for the window cut
int start_point;

/// Number of elements to extract from the queue
int range;

int temp_counter;

/// Temporary array to store the extracted window
complex_g3_t window_cut[WINDOW_SIZE];


void sync_controller_init() {
    start_point = 0;
    range = G_ARRAY_SIZE;
    temp_counter = 0;
}

bool detect_tones() {
    struct_tone_frequencies tone_frequencies;

    // Extract a window of samples from the reading queue
    if (!reading_queue_range(start_point, range, window_cut)) {
        serial_write_formatted("Debug: Error in reading_queue_range Sync Controller\n");
        return false;
    }
    serial_write_formatted("Debug: Success in reading_queue_range Sync Controller\n");


// Stampa dei valori in window_cut
/*for (int i = 0; i < range; i++) {
serial_write_formatted("window_cut[%d].re = %f, window_cut[%d].im = %f\n", i, window_cut[i].re, i, window_cut[i].im);
delay(5);
}*/
    // Perform FFT on the extracted window
    complex_g3_t *out = FFT_simple(window_cut, WINDOW_SIZE);

    // Decode the frequencies from the FFT output
    tone_frequencies = decode_dtmf(out);    

    serial_write_formatted("Info: %d - %d - %d \n", tone_frequencies.low, tone_frequencies.mid, tone_frequencies.high);

    return true;
}

#ifdef __cplusplus
}
#endif
