#include "include/SetupExec.h"

#include <Arduino.h>
#include <FlexCAN_T4.h>
#include <cstdint>

#include "include/CanInterface.h"
#include "include/heartbeat.h"

IntervalTimer execTimer;

void setupExec(void)
{
    initHeartbeat();
    initCanInterfaces();



    Serial.begin(115200);
    delay(1000);
    Serial.println("Teensy 4.0 Triple CAN test");
    resetHeartbeat();



    execTimer.begin(sendTestFrame, 50000000); // Send frame every 500ms
}