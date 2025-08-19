#ifndef EMIT_TONES_H
#define EMIT_TONES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

// Includi solo ciò che è strettamente necessario
#include "global_parameters.h"  // Presumibilmente definisce 'role' e altri parametri condivisi

// Forward declaration della funzione principale
bool emit_tones(const struct_out_tones *pairs, size_t length);

#ifdef __cplusplus
}
#endif

#endif // EMIT_TONES_H
