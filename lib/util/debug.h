 /// 
 ///  @ Author: Kevin Gilliam
 ///  @ Create Time: 2022-09-06 15:53:51
 ///  @ Modified by: Kevin Gilliam
 ///  @ Modified time: 2022-09-08 08:58:21
 ///  @ Description:
 ///

#pragma once
#include "teensy41_device.h"
// #include <cliMacros.h>

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