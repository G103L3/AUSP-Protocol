#include "movement_sensor.h"

#ifdef ARDUINO
#include <Arduino.h>
#define PIR_PIN 31

void movement_sensor_init(void){
    pinMode(PIR_PIN, INPUT);
}

bool movement_sensor_detect(unsigned long duration_ms){
    unsigned long start = millis();
    while(millis() - start < duration_ms){
        if(digitalRead(PIR_PIN)){
            return true;
        }
        delay(10);
    }
    return false;
}

#else
#include <time.h>
#include <unistd.h>
#define PIR_PIN 31

void movement_sensor_init(void){
    (void)PIR_PIN;
}

static unsigned long now_ms(void){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned long)ts.tv_sec * 1000UL + (unsigned long)(ts.tv_nsec/1000000UL);
}

static void delay_ms(unsigned long ms){
    usleep(ms * 1000);
}

bool movement_sensor_detect(unsigned long duration_ms){
    unsigned long start = now_ms();
    while(now_ms() - start < duration_ms){
        delay_ms(10);
    }
    return false;
}
#endif
