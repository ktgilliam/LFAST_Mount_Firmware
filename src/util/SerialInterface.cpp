#include "SerialInterface.h"

#include <vector>
#include <string>
#include <Algorithm>
#include <sstream>
#include <Arduino.h>

// std::vector<SerialSettings> *SerialSettings::serialSettingsPtr;

// std::vector<SerialSettings> serialSettingsTable =
//     {
//         {SerialSettings(TEST_SERIAL_NO, 115200, "TEST")},
// };

// SerialSettings serialChannels;

void initSerialInterfaces()
{
    pinMode(8, OUTPUT);

    SERIAL_CH(TEST_SERIAL_NO).begin(TEST_SERIAL_BAUD);

    // delay(10);
    // SerialSettings::initializeSerialSettings(&serialSettingsTable);
    // serialChannels.printToSerial(TEST_SERIAL_NO, "THIS IS WORKING");
}
