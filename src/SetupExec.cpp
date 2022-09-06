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

    pinMode(CAN3_RX_PIN, OUTPUT);
    pinMode(CAN3_TX_PIN, OUTPUT);

    Serial.begin(115200);
    delay(1000);
    Serial.println("Teensy 4.0 Triple CAN test");
    resetHeartbeat();

    digitalWrite(CAN3_RX_PIN, LOW);
    digitalWrite(CAN3_TX_PIN, LOW);
    execTimer.begin(sendTestFrame, 500000); // Send frame every 500ms
}