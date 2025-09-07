#include "movement_sensor.h"

#include <Arduino.h>

void movement_sensor_init(void){
    pinMode(PIR_PIN, INPUT);
}

bool movement_sensor_detect(unsigned long duration_ms){
    unsigned long start = millis();
    while(millis() - start < duration_ms){
        if(analogRead(PIR_PIN) > PIR_THRESHOLD){
            return true;
        }
        delay(10);
    }
    return false;
}


