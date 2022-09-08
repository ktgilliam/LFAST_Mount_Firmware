 /// 
 ///  @ Author: Kevin Gilliam
 ///  @ Create Time: 2022-09-06 09:38:17
 ///  @ Modified by: Kevin Gilliam
 ///  @ Modified time: 2022-09-07 16:42:01
 ///  @ Description:
 ///

#include "SetupExec.h"

#include <Arduino.h>
#include <FlexCAN_T4.h>
#include <cstdint>

#include <CanMessages.h>
#include <CanInterface.h>
#include <heartbeat.h>
#include "CanTestExec.h"


void setupExec(void)
{
    initHeartbeat();
    initCanInterfaces();
    initializeCanMessages();


    delay(1000);

    resetHeartbeat();
}