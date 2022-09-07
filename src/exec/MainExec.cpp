#include "MainExec.h"

#include <Arduino.h>
#include <cctype>


#include "util/heartbeat.h"
#include "util/debug.h"
#include "can/CanInterface.h"
#include "SetupExec.h"
#include "CanTestExec.h"


#define MODE_PIN 31

#define MODE_PIN_LOW 0U
#define MODE_PIN_HIGH 1U
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