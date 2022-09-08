///
///  @ Author: Kevin Gilliam
///  @ Create Time: 2022-09-06 09:36:04
///  @ Modified by: Kevin Gilliam
///  @ Modified time: 2022-09-08 12:13:45
///  @ Description:
///

#include "MainExec.h"

#include <Arduino.h>
#include <cctype>

#include <heartbeat.h>
#include <debug.h>
#include <device.h>
#include <CanComms.h>
#include <fakeController.h>
#include <fakeMount.h>

#define MODE_PIN_LOW 0U
#define MODE_PIN_HIGH 1U
#define MODE_PIN_INVALID 2U

void deviceSetup();
/**
 * @brief call init functions for the modules used
 *
 */
void setup(void)
{
  deviceSetup();
  initHeartbeat();
  resetHeartbeat();
  initCanComms();

  uint8_t modePinState = digitalRead(MODE_PIN);
  if (modePinState == HIGH)
  {
    TEST_SERIAL.println("CAN Test Mode: Control PC. ");
    setHeartBeatPeriod(100000);
    enableFakeController();
  }
  else
  {
    TEST_SERIAL.println("CAN Test Mode: Mount. ");
    setHeartBeatPeriod(400000);
    enableFakeMount();
  }
}

void loop(void)
{
  // pingHeartBeat();
  // updateCanBusEvents();
}

/**
 * @brief configure pins and test interfaces
 *
 */
void deviceSetup()
{
  pinMode(LED_PIN, OUTPUT);
  pinMode(MODE_PIN, INPUT);
  pinMode(DEBUG_PIN_1, OUTPUT);
  pinMode(TEST_SERIAL_TX_PIN, OUTPUT);

  digitalWrite(DEBUG_PIN_1, LOW);

  TEST_SERIAL.begin(TEST_SERIAL_BAUD);
  delay(500);
  TEST_SERIAL.println("Device Setup Complete.");
}