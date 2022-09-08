 /// 
 ///  @ Author: Kevin Gilliam
 ///  @ Create Time: 2022-09-07 08:34:54
 ///  @ Modified by: Kevin Gilliam
 ///  @ Modified time: 2022-09-08 08:14:10
 ///  @ Description:
 ///

#pragma once

#include "macro.h"
#include <Arduino.h>

#if defined(ARDUINO_TEENSY41)
#define NUM_SERIAL_DEVICES 8
#else
#define NUM_SERIAL_DEVICES 7
#endif

#define TEST_SERIAL_NO 2
#define TEST_SERIAL SERIAL_CH(2)
#define TEST_SERIAL_BAUD 115200
#define TEST_SERIAL_TX_PIN 8
#define TEST_SERIAL_RX_PIN 7


#define MODE_PIN         31
#define LED_PIN          13
#define DEBUG_PIN_1 32


#define SERIAL_CH(N) CONCAT(Serial, N)