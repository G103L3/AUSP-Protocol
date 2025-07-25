#ifndef EMIT_TONES_H
#define EMIT_TONES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

// Includi solo ciò che è strettamente necessario
#include "global_parameters.h"  // Presumibilmente definisce 'role' e altri parametri condivisi

// Forward declaration della funzione principale
bool emit_tones(int *bits, int role);

#ifdef __cplusplus
}
#endif

#endif // EMIT_TONES_H
