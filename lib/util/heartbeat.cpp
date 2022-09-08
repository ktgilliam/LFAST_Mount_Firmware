 /// 
 ///  @ Author: Kevin Gilliam
 ///  @ Create Time: 2022-09-06 12:05:44
 ///  @ Modified by: Kevin Gilliam
 ///  @ Modified time: 2022-09-08 08:13:58
 ///  @ Description:
 ///

#include "heartbeat.h"
#include <cctype>
#include <cstdint>
#include <device.h>

enum ledState
{
    LED_OFF = LOW,
    LED_ON = HIGH
};

static uint32_t cnt = 0;
static ledState heartbeatState = LED_OFF;
uint32_t heartbeatPeriodCnts = 1000;

void resetHeartbeat();
ledState toggleHeartbeatState(ledState hbState);
void setHeartbeatStateOff();
void setHeartbeatStateOn();
void updateLedPin();

/**
 * @brief
 *
 */
void initHeartbeat()
{
    pinMode(LED_PIN, OUTPUT);
    setHeartbeatStateOn();
    updateLedPin();
}

/**
 * @brief
 *
 */
void resetHeartbeat()
{
    cnt = 0;
    setHeartbeatStateOff();
    updateLedPin();
}

/**
 * @brief
 *
 * @return true
 * @return false
 */
bool pingHeartBeat()
{
    bool pinToggledFlag = false;
    if (cnt++ >= heartbeatPeriodCnts)
    {
        cnt = 0;
        heartbeatState = toggleHeartbeatState(heartbeatState);
        pinToggledFlag = true;
    }
    updateLedPin();
    return (pinToggledFlag);
}

void setHeartBeatPeriod(uint32_t cnts)
{
    heartbeatPeriodCnts = cnts;
}
/**
 * @brief
 *
 * @param hbState
 * @return ledState
 */
ledState toggleHeartbeatState(ledState hbState)
{
    ledState returnVal;
    if (hbState == LED_OFF)
    {

        returnVal = LED_ON;
    }
    else
    {
        returnVal = LED_OFF;
    }
    return (returnVal);
}

/**
 * @brief Set the Heartbeat State Off object
 *
 */
void setHeartbeatStateOff()
{
    heartbeatState = LED_OFF;
}

/**
 * @brief Set the Heartbeat State On object
 *
 */
void setHeartbeatStateOn()
{
    heartbeatState = LED_ON;
}

/**
 * @brief
 *
 */
void updateLedPin()
{
    digitalWrite(LED_PIN, heartbeatState);
}