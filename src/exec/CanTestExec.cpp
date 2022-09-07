 /// 
 ///  @ Author: Kevin Gilliam
 ///  @ Create Time: 2022-09-06 09:38:38
 ///  @ Modified by: Kevin Gilliam
 ///  @ Modified time: 2022-09-07 15:30:10
 ///  @ Description:
 ///


#include "CanTestExec.h"

#include <stdio.h>
#include <string.h>

#include "can/CanMessages.h"
#include "can/CanInterface.h"

IntervalTimer testExecTimer;
CanTestMode testMode;

void parseMessage(const CAN_message_t &msg);
bool printMessageInfo();

void initCanTestExec(CanTestMode mode)
{
    if (mode == CAN_TEST_MODE_TALKER)
    {
        testExecTimer.begin(sendDeadBeef, 50000); // Send frame every 500ms
    }
    else
    {
        testExecTimer.end();
    }

    registerCanRxCallback(parseMessage);
}

void sendDeadBeef()
{
    const char deadbeef[] = {0xDE, 0xAD, 0xBE, 0xEF};
    CAN_message_t msg;
    msg.id = CAN_ID(INVALID_ID, INVALID_ID);
    msg.len = strlen(deadbeef);
    memcpy(msg.buf, deadbeef, msg.len);
    sendMessage(msg);
}

void parseMessage(const CAN_message_t &msg)
{
    processMessage(msg);
}

/**
 * @brief
 *
 */
void sendTestFrame()
{
    CAN_message_t msg;
    msg.id = 0x401;
    msg.len = MAX_MESSAGE_LENGTH;
    static int d;

    for (uint8_t ii = 0; ii < msg.len; ii++)
    {
        msg.buf[ii] = ii + 1;
    }

    msg.id = 0x402;
    msg.buf[1] = d++;

    sendMessage(msg);
}

bool printMessageInfo()
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
    for (uint8_t ii = 0; ii < msg.len; ii++)
    {
      Serial.print(msg.buf[ii]);
      Serial.print(" ");
    }
    Serial.print("  TS: ");
    Serial.println(msg.timestamp);
    retVal = true;
  }
  return (retVal);
}