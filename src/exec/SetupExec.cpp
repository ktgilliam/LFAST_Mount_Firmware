 /// 
 ///  @ Author: Kevin Gilliam
 ///  @ Create Time: 2022-09-06 09:38:17
 ///  @ Modified by: Kevin Gilliam
 ///  @ Modified time: 2022-09-07 15:30:23
 ///  @ Description:
 ///

#include "SetupExec.h"

#include <Arduino.h>
#include <FlexCAN_T4.h>
#include <cstdint>

#include "can/CanMessages.h"
#include "can/CanInterface.h"
#include "util/heartbeat.h"
#include "util/SerialInterface.h"
#include "CanTestExec.h"


void setupExec(void)
{
    initHeartbeat();
    initSerialInterfaces();
    initCanInterfaces();
    initializeCanMessages();


    delay(1000);

    SERIAL_CH(TEST_SERIAL_NO).print("\nTeensy 4.0 Triple CAN test");
    resetHeartbeat();
}