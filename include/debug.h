#include <Arduino.h>

#define DEBUG_PIN_1 32

#define CONFIG_DEBUG_PIN_1() pinMode(DEBUG_PIN_1, OUTPUT);
#define SET_DEBUG_PIN_1()    digitalWrite(DEBUG_PIN_1, HIGH);
#define CLR_DEBUG_PIN_1()    digitalWrite(DEBUG_PIN_1, LOW);