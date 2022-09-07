 /// 
 ///  @ Author: Kevin Gilliam
 ///  @ Create Time: 2022-09-07 08:34:33
 ///  @ Modified by: Kevin Gilliam
 ///  @ Modified time: 2022-09-07 15:30:44
 ///  @ Description:
 ///

#include "SerialInterface.h"

#include <vector>
#include <string>
#include <Algorithm>
#include <sstream>
#include <Arduino.h>

void initSerialInterfaces()
{
    pinMode(8, OUTPUT);
    SERIAL_CH(TEST_SERIAL_NO).begin(TEST_SERIAL_BAUD);
}
