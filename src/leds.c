#ifdef __cplusplus
extern "C" {
#endif
#include "leds.h"

void turn_red(uint8_t val){
    pinMode(25, OUTPUT);
    pinMode(27, OUTPUT);
    digitalWrite(25, val);
    digitalWrite(27, !val);
}
void turn_green(uint8_t val){
    pinMode(27, OUTPUT);
    digitalWrite(27, val);
}
void turn_blue(uint8_t val){
    pinMode(26, OUTPUT);
    digitalWrite(26, val);
}

void turn_off(){
    pinMode(25, OUTPUT);
    digitalWrite(25, LOW);    
    turn_green(0);
    turn_blue(0);
}

#ifdef __cplusplus
}
#endif