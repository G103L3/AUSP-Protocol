#ifndef MOVEMENT_SENSOR_H
#define MOVEMENT_SENSOR_H

#include <stdbool.h>
#include "global_parameters.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PIR_THRESHOLD 2048

void movement_sensor_init(void);
bool movement_sensor_detect(unsigned long duration_ms);

#ifdef __cplusplus
}
#endif

#endif // MOVEMENT_SENSOR_H
