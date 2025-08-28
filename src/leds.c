#ifdef __cplusplus
extern "C" {
#endif
#include "leds.h"

void turn_red(uint8_t val){
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    digitalWrite(RED_LED, val);
    digitalWrite(GREEN_LED, !val);
}
void turn_green(uint8_t val){
    pinMode(GREEN_LED, OUTPUT);
    digitalWrite(GREEN_LED, val);
}
void turn_blue(uint8_t val){
    pinMode(BLUE_LED, OUTPUT);
    digitalWrite(BLUE_LED, val);
}

void turn_off(){
    pinMode(RED_LED, OUTPUT);
    digitalWrite(RED_LED, LOW);    
    turn_green(0);
    turn_blue(0);
}

#ifdef __cplusplus
}
#endif