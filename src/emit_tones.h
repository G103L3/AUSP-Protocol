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
bool emit_tones(const int *bits, size_t length, int role);

#ifdef __cplusplus
}
#endif

#endif // EMIT_TONES_H
