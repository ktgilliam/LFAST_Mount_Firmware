 /// 
 ///  @ Author: Kevin Gilliam
 ///  @ Create Time: 2022-09-07 08:34:54
 ///  @ Modified by: Kevin Gilliam
 ///  @ Modified time: 2022-09-07 15:31:07
 ///  @ Description:
 ///

#pragma once

#include "macro.h"

#if defined(ARDUINO_TEENSY41)
#define NUM_SERIAL_DEVICES 8
#else
#define NUM_SERIAL_DEVICES 7
#endif

#define TEST_SERIAL_NO 2
#define TEST_SERIAL_BAUD 115200


#define SERIAL_CH(N) CONCAT(Serial, N)

void initSerialInterfaces();
