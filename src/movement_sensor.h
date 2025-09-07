#ifndef MOVEMENT_SENSOR_H
#define MOVEMENT_SENSOR_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void movement_sensor_init(void);
bool movement_sensor_detect(unsigned long duration_ms);

#ifdef __cplusplus
}
#endif

#endif // MOVEMENT_SENSOR_H
