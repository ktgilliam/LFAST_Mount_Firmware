 /// 
 ///  @ Author: Kevin Gilliam
 ///  @ Create Time: 2022-09-06 15:53:51
 ///  @ Modified by: Kevin Gilliam
 ///  @ Modified time: 2022-09-08 14:31:00
 ///  @ Description:
 ///

#pragma once
#include <device.h>

#define TEST_MODE_ACTIVE 1

#define TOGGLE_DEBUG_PIN() digitalWrite(DEBUG_PIN_1, !digitalRead(DEBUG_PIN_1));

inline void LedDebugBlink(uint32_t prd, uint32_t blinks)
{
    for (uint32_t ii = 0; ii < blinks; ii++)
    {
        digitalWrite(LED_PIN, HIGH);
        delayMicroseconds(prd);
        digitalWrite(LED_PIN, LOW);
        delayMicroseconds(prd);
    }
}