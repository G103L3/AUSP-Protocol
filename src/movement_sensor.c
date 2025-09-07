#include "movement_sensor.h"

#include <Arduino.h>

void movement_sensor_init(void){
    pinMode(PIR_PIN, INPUT);
}

static volatile bool abort_flag = false;
static volatile bool was_aborted = false;

bool movement_sensor_detect(unsigned long duration_ms){
    unsigned long start = millis();
    was_aborted = false;
    while(millis() - start < duration_ms){
        if(abort_flag){
            was_aborted = true;
            abort_flag = false;
            return false;
        }
        if(analogRead(PIR_PIN) > PIR_THRESHOLD){
            abort_flag = false;
            return true;
        }
        delay(10);
    }
    abort_flag = false;
    return false;
}

void movement_sensor_abort(void){
    abort_flag = true;
}

bool movement_sensor_aborted(void){
    bool ret = was_aborted;
    was_aborted = false;
    return ret;
}
