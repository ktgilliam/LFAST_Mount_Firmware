 /// 
 ///  @ Author: Kevin Gilliam
 ///  @ Create Time: 2022-09-06 15:53:51
 ///  @ Modified by: Kevin Gilliam
 ///  @ Modified time: 2022-09-07 15:30:30
 ///  @ Description:
 ///

#pragma once
#include <Arduino.h>

#define DEBUG_PIN_1 32
// #define DEBUG_PRD 10

#define CONFIG_DEBUG_PIN_1() pinMode(DEBUG_PIN_1, OUTPUT);
#define SET_DEBUG_PIN_1() digitalWrite(DEBUG_PIN_1, HIGH);
#define CLR_DEBUG_PIN_1() digitalWrite(DEBUG_PIN_1, LOW);

inline void LedDebugBlink()
{
    for (int ii = 0; ii < 5; ii++)
    {
        digitalWrite(13, HIGH);
        delayMicroseconds(1000000);
        digitalWrite(13, LOW);
        delayMicroseconds(100000);
    }
}