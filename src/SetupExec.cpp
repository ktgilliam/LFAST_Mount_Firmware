#include "include/SetupExec.h"

#include <Arduino.h>
#include <FlexCAN_T4.h>
#include <cstdint>

#include "include/CanInterface.h"
#include "include/heartbeat.h"
#include "include/CanTestExec.h"
#include "include/CanMessages.h"
#include "include/CanTestExec.h"
#include "include/SerialInterface.h"

IntervalTimer execTimer;

void setupExec(void)
{
    initHeartbeat();
    initCanInterfaces();
    initializeCanMessages();
    initCanTestExec();
    initSerialInterface();
    
    delay(1000);
    Serial.println("Teensy 4.0 Triple CAN test");
    resetHeartbeat();



    execTimer.begin(sendDeadBeef, 500000); // Send frame every 500ms
}