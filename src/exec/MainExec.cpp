 /// 
 ///  @ Author: Kevin Gilliam
 ///  @ Create Time: 2022-09-06 09:36:04
 ///  @ Modified by: Kevin Gilliam
 ///  @ Modified time: 2022-09-07 16:42:26
 ///  @ Description:
 ///

#include "MainExec.h"

#include <Arduino.h>
#include <cctype>

#include <heartbeat.h>
#include <debug.h>
#include <CanInterface.h>
#include "SetupExec.h"
#include "CanTestExec.h"

#define MODE_PIN         31
#define MODE_PIN_LOW     0U
#define MODE_PIN_HIGH    1U
#define MODE_PIN_INVALID 2U

int main(void)
{
  setupExec();
  pinMode(MODE_PIN, INPUT);
  uint8_t modePinState;
  uint8_t prevModePinState = MODE_PIN_INVALID;
  
  while (1)
  {
    modePinState = digitalRead(MODE_PIN);
    if (modePinState != prevModePinState)
    {
      if (modePinState == HIGH)
      {
        initCanTestExec(CAN_TEST_MODE_TALKER);
        setHeartBeatPeriod(600000);
      }
      else
      {
        initCanTestExec(CAN_TEST_MODE_LISTENER);
        setHeartBeatPeriod(2400000);
      }
      prevModePinState = modePinState;
    }
    pingHeartBeat();
  }
  return 0;
}