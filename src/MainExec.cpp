#include "include/MainExec.h"
#include <cctype>
#include "include/SetupExec.h"
#include "include/CanInterface.h"
#include "include/heartbeat.h"

IntervalTimer timer;

bool printMessage();

int main()
{
  setupExec();
  bool newMessageFlag;
  while (1)
  {
    newMessageFlag = printMessage();
    if (newMessageFlag)
    {
      pingHeartBeat();
    }

    // Serial.println("Teensy 4.0 Triple CAN test");
  }
  return 0;
}

bool printMessage()
{
  bool retVal = false;
  CAN_message_t msg;

  if (updateCanBusEvents(msg))
  {
    Serial.print("MB: ");
    Serial.print(msg.mb);
    Serial.print("  OVERRUN: ");
    Serial.print(msg.flags.overrun);
    Serial.print("  ID: 0x");
    Serial.print(msg.id, HEX);
    Serial.print("  EXT: ");
    Serial.print(msg.flags.extended);
    Serial.print("  LEN: ");
    Serial.print(msg.len);
    // Serial.print("  BRS: ");
    // Serial.print(msg.brs);
    // Serial.print("  EDL: ");
    // Serial.print(msg.edl);
    Serial.print(" DATA: ");
    for (uint8_t i = 0; i < msg.len; i++)
    {
      Serial.print(msg.buf[i]);
      Serial.print(" ");
    }
    Serial.print("  TS: ");
    Serial.println(msg.timestamp);
    retVal = true;
  }
  return (retVal);
}