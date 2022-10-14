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
#include <NetComms.h>
#include <cmath>

#include "Telescope.h"

void ackGotoCommand(uint8_t axis);
void ackSyncCommand(uint8_t axis);

// Message Handlers:
void handshake(unsigned int val);
void updateTime(double indiTimeStamp);
void sendAltAzPositions(double indiTimeStamp);
void sendParkedStatus(double indiTimeStamp);
void sendTrackRate(double indiTimeStamp);
void parkScope(double indiTimeStamp);
void unparkScope(double indiTimeStamp);
void noDisconnect(bool noDiscoFlag);
void abortSlew(double indiTimeStamp);
void sendSlewCompleteStatus(double indiTimeStamp);
void updateTargetAzPosition(double tgtAzPosn);
void updateTargetAltPosition(double tgtAltPosn);
void syncAzPosition(double currentAzPosn);
void syncAltPosition(double currentElPosn);
void findHome(double indiTimeStamp);

LFAST::EthernetCommsService *commsService;
LFAST::MountControl *mountControl;

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
}

/**
 * @brief call init functions for the modules used
 *
 */
void setup(void)
{
  deviceSetup();
  CLEAR_CONSOLE();
  CURSOR_TO_ZEROZERO();
  TEST_SERIAL.printf("################################################################################################\r\n");
  TEST_SERIAL.printf("###################################### LFAST MOUNT CONTROL #####################################\r\n");
  TEST_SERIAL.printf("################################################################################################\r\n");
  commsService = new LFAST::EthernetCommsService();
  mountControl = new LFAST::MountControl();

  if (!commsService->Status())
  {
    TEST_SERIAL.println("Device Setup Failed.");
    while (true)
    {
      ;
      ;
    }
  }

  commsService->registerMessageHandler<unsigned int>("Handshake", handshake);
  commsService->registerMessageHandler<double>("time", updateTime);
  commsService->registerMessageHandler<double>("RequestAltAz", sendAltAzPositions);
  commsService->registerMessageHandler<double>("IsParked", sendParkedStatus);
  commsService->registerMessageHandler<double>("getTrackRate", sendTrackRate);
  commsService->registerMessageHandler<double>("Park", parkScope);
  commsService->registerMessageHandler<double>("Unpark", unparkScope);
  commsService->registerMessageHandler<double>("AbortSlew", abortSlew);
  commsService->registerMessageHandler<bool>("NoDisconnect", noDisconnect);
  commsService->registerMessageHandler<double>("IsSlewComplete", sendSlewCompleteStatus);
  commsService->registerMessageHandler<double>("slewToAzPosn", updateTargetAzPosition);
  commsService->registerMessageHandler<double>("slewToAltPosn", updateTargetAltPosition);
  commsService->registerMessageHandler<double>("syncAzPosn", syncAzPosition);
  commsService->registerMessageHandler<double>("syncAltPosn", syncAltPosition);
  commsService->registerMessageHandler<double>("FindHome", findHome);

  delay(500);

  initHeartbeat();
  resetHeartbeat();
  uint8_t modePinState = digitalRead(MODE_PIN);
  if (modePinState == HIGH)
  {
    setHeartBeatPeriod(100000);
    // TEST_SERIAL.println("CAN Test Mode: Talker. ");
  }
  else
  {
    setHeartBeatPeriod(400000);
    // TEST_SERIAL.println("CAN Test Mode: Listener. ");
  }
}

void loop(void)
{
  commsService->checkForNewClients();
  commsService->checkForNewClientData();
  commsService->processClientData();
  commsService->stopDisconnectedClients();
}

void handshake(unsigned int val)
{
  LFAST::CommsMessage newMsg;
  if (val == 0xDEAD)
  {
    newMsg.addKeyValuePair<unsigned int>("Handshake", 0xBEEF);
    commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
  }
  else
  {
    // TODO: Generate error
  }
  return;
}

void updateTime(double indiTimeStamp)
{
#if SIM_SCOPE_ENABLED
  mountControl->updateSimMount(indiTimeStamp);
#endif
}

void sendAltAzPositions(double indiTimeStamp)
{
  LFAST::CommsMessage newMsg;
#if SIM_SCOPE_ENABLED
  mountControl->updateSimMount(indiTimeStamp);
#endif

  newMsg.addKeyValuePair<double>("AzPosition", mountControl->getCurrentAz());
  newMsg.addKeyValuePair<double>("AltPosition", mountControl->getCurrentAlt());
  commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
  mountControl->printMountStatus();
}

void sendParkedStatus(double indiTimeStamp)
{
#if SIM_SCOPE_ENABLED
  mountControl->updateSimMount(indiTimeStamp);
#endif
  LFAST::CommsMessage newMsg;
  newMsg.addKeyValuePair<bool>("IsParked", mountControl->mountIsParked());
  commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
}

void sendTrackRate(double indiTimeStamp)
{
#if SIM_SCOPE_ENABLED
  mountControl->updateSimMount(indiTimeStamp);
#endif
  LFAST::CommsMessage newMsg;
  newMsg.addKeyValuePair<double>("TrackRate", mountControl->getTrackRate());
  commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
}

void parkScope(double indiTimeStamp)
{
  mountControl->park();

#if SIM_SCOPE_ENABLED
  mountControl->updateSimMount(indiTimeStamp);
#endif

  LFAST::CommsMessage newMsg;
  newMsg.addKeyValuePair<std::string>("Park", "$OK^");
  commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
}

void unparkScope(double indiTimeStamp)
{
  mountControl->unpark();

#if SIM_SCOPE_ENABLED
  mountControl->updateSimMount(indiTimeStamp);
#endif

  LFAST::CommsMessage newMsg;
  newMsg.addKeyValuePair<std::string>("Unpark", "$OK^");
  commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
}

void noDisconnect(bool noDiscoFlag)
{
  LFAST::CommsMessage newMsg;
  newMsg.addKeyValuePair<std::string>("NoDisconnect", "$OK^");
  commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
}

void abortSlew(double indiTimeStamp)
{
  mountControl->abortSlew();
  LFAST::CommsMessage newMsg;
  newMsg.addKeyValuePair<std::string>("AbortSlew", "$OK^");
  commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
}

void sendSlewCompleteStatus(double indiTimeStamp)
{
#if SIM_SCOPE_ENABLED
  mountControl->updateSimMount(indiTimeStamp);
#endif
  LFAST::CommsMessage newMsg;
  newMsg.addKeyValuePair<bool>("SlewIsComplete", mountControl->mountIsIdle());
  commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
}

void updateTargetAltPosition(double tgtAltPosn)
{
  mountControl->gotoAlt(tgtAltPosn);
  ackGotoCommand(LFAST::MountControl::EL_AXIS);
}

void updateTargetAzPosition(double tgtAzPosn)
{
  mountControl->gotoAz(tgtAzPosn);
  ackGotoCommand(LFAST::MountControl::AZ_AXIS);
}

void syncAzPosition(double currentAzPosn)
{
  mountControl->syncAz(currentAzPosn);
  ackSyncCommand(LFAST::MountControl::AZ_AXIS);
}

void syncAltPosition(double currentAltPosn)
{
  mountControl->syncAlt(currentAltPosn);
  ackSyncCommand(LFAST::MountControl::EL_AXIS);
}

void findHome(double indiTimeStamp)
{
  mountControl->findHome();
#if SIM_SCOPE_ENABLED
  mountControl->updateSimMount(indiTimeStamp);
#endif

  LFAST::CommsMessage newMsg;
  newMsg.addKeyValuePair<std::string>("FindHome", "$OK^");
  commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
}

void ackGotoCommand(uint8_t axis)
{
  static bool azUpdated = false;
  static bool elUpdated = false;

  if (axis == LFAST::MountControl::AZ_AXIS)
    azUpdated = true;
  else if (axis == LFAST::MountControl::EL_AXIS)
    elUpdated = true;

  if (azUpdated && elUpdated)
  {
    LFAST::CommsMessage newMsg;
    newMsg.addKeyValuePair<std::string>("slewToAltPosn", "$OK^");
    newMsg.addKeyValuePair<std::string>("slewToAzPosn", "$OK^");
    commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
    azUpdated = false;
    elUpdated = false;
  }
}

void ackSyncCommand(uint8_t axis)
{
  static bool azUpdated = false;
  static bool elUpdated = false;

  if (axis == LFAST::MountControl::AZ_AXIS)
    azUpdated = true;
  else if (axis == LFAST::MountControl::EL_AXIS)
    elUpdated = true;

  if (azUpdated && elUpdated)
  {
    LFAST::CommsMessage newMsg;
    newMsg.addKeyValuePair<std::string>("syncAltPosn", "$OK^");
    newMsg.addKeyValuePair<std::string>("syncAzPosn", "$OK^");
    commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
    azUpdated = false;
    elUpdated = false;
  }
}
