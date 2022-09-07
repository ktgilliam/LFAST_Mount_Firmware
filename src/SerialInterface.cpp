#include "include/SerialInterface.h"

#include <vector>
#include <string>
#include <Algorithm>
#include <sstream>
#include <Arduino.h>
// Serial2
#define DEBUG_PULSE_DELAY 2
#define DEBUG_PIN 29

std::vector<SerialSettings> *SerialSettings::serialSettingsPtr;


void debugFuncTmp()
{
    for (int ii = 0; ii < 10; ii++)
    {
        digitalWrite(DEBUG_PIN, LOW);
        delay(DEBUG_PULSE_DELAY);
        digitalWrite(DEBUG_PIN, HIGH);
        delay(DEBUG_PULSE_DELAY);
    }
}
std::vector<SerialSettings> serialSettingsTable =
    {
        {SerialSettings(TEST_SERIAL_NO, 115200, "TEST")},
};

SerialSettings serialChannels;

void initSerialInterface()
{
    pinMode(DEBUG_PIN, OUTPUT);
    pinMode(8, OUTPUT);

    // debugFuncTmp();
    delay(10);
    SerialSettings::initializeSerialSettings(&serialSettingsTable);

    // debugFuncTmp();

    serialChannels.printToSerial(TEST_SERIAL_NO, "THIS IS WORKING");
}
