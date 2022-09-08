 /// 
 ///  @ Author: Kevin Gilliam
 ///  @ Create Time: 2022-09-07 08:34:54
 ///  @ Modified by: Kevin Gilliam
 ///  @ Modified time: 2022-09-08 12:39:33
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


#define SERIAL_CH(N) CONCAT(Serial, N)




#define TEST_SERIAL_NO 2
#define TEST_SERIAL SERIAL_CH(TEST_SERIAL_NO)
#define TEST_SERIAL_BAUD 115200

#if TEST_SERIAL_NO==1
    #define TEST_SERIAL_RX_PIN 0
    #define TEST_SERIAL_TX_PIN 1
    // #define TEST_SERIAL_RX_PIN 52
    // #define TEST_SERIAL_TX_PIN 53
#elif  TEST_SERIAL_NO==2
    #define TEST_SERIAL_RX_PIN 7
    #define TEST_SERIAL_TX_PIN 8
#elif  TEST_SERIAL_NO==3
    #define TEST_SERIAL_RX_PIN 15
    #define TEST_SERIAL_TX_PIN 14
#elif  TEST_SERIAL_NO==4
    #define TEST_SERIAL_RX_PIN 16
    #define TEST_SERIAL_TX_PIN 17
#elif  TEST_SERIAL_NO==5
    #define TEST_SERIAL_RX_PIN 21
    #define TEST_SERIAL_TX_PIN 20
    // #define TEST_SERIAL_RX_PIN 46
    // #define TEST_SERIAL_TX_PIN 47
#elif  TEST_SERIAL_NO==6
    #define TEST_SERIAL_RX_PIN 25
    #define TEST_SERIAL_TX_PIN 24
#elif  TEST_SERIAL_NO==7
    #define TEST_SERIAL_RX_PIN 28
    #define TEST_SERIAL_TX_PIN 29
#elif  TEST_SERIAL_NO==8
    #define TEST_SERIAL_RX_PIN 34
    // #define TEST_SERIAL_RX_PIN 48
    #define TEST_SERIAL_TX_PIN 35
#endif


#define MODE_PIN         31
#define LED_PIN          13

#define CAN3_PIN 35
#define CAN1_PIN 34

#define DEBUG_PIN_1 32
#define TOGGLE_DEBUG_PIN() digitalWrite(DEBUG_PIN_1, !digitalRead(DEBUG_PIN_1));


#define TOGGLE_LED_PIN() digitalWrite(LED_PIN, !digitalRead(LED_PIN));
