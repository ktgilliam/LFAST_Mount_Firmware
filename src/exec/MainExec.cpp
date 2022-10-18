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

void updateAltAzGotoCommand(uint8_t axis, double val);
void updateRaDecGotoCommand(uint8_t axis, double val);
void updateSyncCommand(uint8_t axis, double val);

// Message Handlers:
void handshake(unsigned int val);
void updateTime(double lst);
void updateLatitude(double lat);
void updateLongitude(double lon);
void sendRaDec(double lst);
void sendParkedStatus(double lst);
void sendTrackStatus(double lst);
void parkScope(double lst);
void unparkScope(double lst);
void noDisconnect(bool noDiscoFlag);
void abortSlew(double lst);
void sendSlewCompleteStatus(double lst);

void slewToRa(double ra);
void slewToDec(double dec);

void slewToAz(double tgtAzPosn);
void slewToAlt(double tgtAltPosn);

void syncRaPosition(double currentRaPosn);
void syncDecPosition(double currentDecPosn);
void findHome(double lst);

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
    commsService->registerMessageHandler<double>("latitude", updateLatitude);
    commsService->registerMessageHandler<double>("longitude", updateLongitude);
    commsService->registerMessageHandler<double>("RequestRaDec", sendRaDec);
    commsService->registerMessageHandler<double>("IsParked", sendParkedStatus);
    commsService->registerMessageHandler<double>("IsTracking", sendTrackStatus);
    commsService->registerMessageHandler<double>("Park", parkScope);
    commsService->registerMessageHandler<double>("Unpark", unparkScope);
    commsService->registerMessageHandler<double>("AbortSlew", abortSlew);
    commsService->registerMessageHandler<double>("IsSlewComplete", sendSlewCompleteStatus);
    commsService->registerMessageHandler<bool>("NoDisconnect", noDisconnect);

    commsService->registerMessageHandler<double>("slewToAzPosn", slewToAz);
    commsService->registerMessageHandler<double>("slewToAltPosn", slewToAlt);

    commsService->registerMessageHandler<double>("slewToRa", slewToRa);
    commsService->registerMessageHandler<double>("slewToDec", slewToDec);

    commsService->registerMessageHandler<double>("syncRaPosn", syncRaPosition);
    commsService->registerMessageHandler<double>("syncDecPosn", syncDecPosition);


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

void setNoReply(bool flag)
{
    commsService->setNoReplyFlag(flag);
}

void updateTime(double lst)
{
    mountControl->updateClock(lst);
#if SIM_SCOPE_ENABLED
    mountControl->updateSimMount();
#endif
}

void updateLatitude(double lat)
{
#if SIM_SCOPE_ENABLED
#endif
    mountControl->setLatitude(lat);
    TEST_SERIAL.printf("\033[%u;%uH", 7, 0);
    TEST_SERIAL.printf("Latitude:\t%8.4f", lat);
}

void updateLongitude(double lon)
{
#if SIM_SCOPE_ENABLED
#endif
    mountControl->setLongitude(lon);
    TEST_SERIAL.printf("\033[%u;%uH", 8, 0);
    TEST_SERIAL.printf("Longitude:\t%8.4f", lon);
}

void sendRaDec(double lst)
{
    LFAST::CommsMessage newMsg;
#if SIM_SCOPE_ENABLED
    mountControl->updateClock(lst);
    mountControl->updateSimMount();
#endif

    double ra = 0.0, dec = 0.0;
    mountControl->getCurrentRaDec(&ra, &dec);
    newMsg.addKeyValuePair<double>("RA", ra);
    newMsg.addKeyValuePair<double>("DEC", dec);
    commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
    mountControl->printMountStatus();
}

void sendParkedStatus(double lst)
{
#if SIM_SCOPE_ENABLED
    mountControl->updateClock(lst);
    mountControl->updateSimMount();
#endif
    LFAST::CommsMessage newMsg;
    newMsg.addKeyValuePair<bool>("IsParked", mountControl->mountIsParked());
    commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
}

void sendTrackStatus(double lst)
{
#if SIM_SCOPE_ENABLED
    mountControl->updateClock(lst);
    mountControl->updateSimMount();
#endif
    LFAST::CommsMessage newMsg;
    newMsg.addKeyValuePair<bool>("IsTracking", mountControl->mountIsTracking());
    commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
}

void parkScope(double lst)
{
    mountControl->park();

#if SIM_SCOPE_ENABLED
    mountControl->updateClock(lst);
    mountControl->updateSimMount();
#endif

    LFAST::CommsMessage newMsg;
    newMsg.addKeyValuePair<std::string>("Park", "$OK^");
    commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
}

void unparkScope(double lst)
{
    mountControl->unpark();

#if SIM_SCOPE_ENABLED
    mountControl->updateClock(lst);
    mountControl->updateSimMount();
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

void abortSlew(double lst)
{
    mountControl->abortSlew();
    LFAST::CommsMessage newMsg;
    newMsg.addKeyValuePair<std::string>("AbortSlew", "$OK^");
    commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
}

void sendSlewCompleteStatus(double lst)
{
#if SIM_SCOPE_ENABLED
    mountControl->updateClock(lst);
    mountControl->updateSimMount();
#endif
    LFAST::CommsMessage newMsg;
    newMsg.addKeyValuePair<bool>("IsSlewComplete", mountControl->mountIsIdle());
    commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
}

void slewToAlt(double targetAlt)
{
    updateAltAzGotoCommand(LFAST::MountControl::ALT_AXIS, targetAlt);
}

void slewToAz(double targetAz)
{
    updateAltAzGotoCommand(LFAST::MountControl::AZ_AXIS, targetAz);
}

void slewToRa(double targetRa)
{
    updateRaDecGotoCommand(LFAST::MountControl::RA_AXIS, targetRa);
}
void slewToDec(double targetDec)
{
    updateRaDecGotoCommand(LFAST::MountControl::DEC_AXIS, targetDec);
}

void syncRaPosition(double currentRaPosn)
{
    updateSyncCommand(LFAST::MountControl::RA_AXIS, currentRaPosn);
}

void syncDecPosition(double currentDecPosn)
{
    updateSyncCommand(LFAST::MountControl::DEC_AXIS, currentDecPosn);
}

void findHome(double lst)
{
    mountControl->findHome();
#if SIM_SCOPE_ENABLED
    mountControl->updateSimMount();
#endif

    LFAST::CommsMessage newMsg;
    newMsg.addKeyValuePair<std::string>("FindHome", "$OK^");
    commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
}

void updateAltAzGotoCommand(uint8_t axis, double val)
{
    static bool azUpdated = false;
    static bool altUpdated = false;
    static double azVal = 0.0;
    static double altVal = 0.0;

    if (axis == LFAST::MountControl::AZ_AXIS)
    {
        azUpdated = true;
        azVal = val;
    }
    else if (axis == LFAST::MountControl::ALT_AXIS)
    {
        altUpdated = true;
        altVal = val;
    }

    if (azUpdated && altUpdated)
    {
        LFAST::CommsMessage newMsg;
        newMsg.addKeyValuePair<std::string>("slewToAltPosn", "$OK^");
        newMsg.addKeyValuePair<std::string>("slewToAzPosn", "$OK^");
        commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
        azUpdated = false;
        altUpdated = false;
        mountControl->gotoAltAz(altVal, azVal);
    }
}

void updateRaDecGotoCommand(uint8_t axis, double val)
{
    static bool raUpdated = false;
    static bool decUpdated = false;
    static double raVal = 0.0;
    static double decVal = 0.0;

    if (axis == LFAST::MountControl::RA_AXIS)
    {
        raVal = val;
        raUpdated = true;
    }
    else if (axis == LFAST::MountControl::DEC_AXIS)
    {
        decVal = val;
        decUpdated = true;
    }

    if (raUpdated && decUpdated)
    {
        // CURSOR_TO_DEBUG_ROW(-1);
        // // TEST_SERIAL.printf("New Slew Coords: RA[%8.4f]/DEC[%8.4f]", raVal, decVal);
        LFAST::CommsMessage newMsg;
        newMsg.addKeyValuePair<std::string>("slewToRa", "$OK^");
        newMsg.addKeyValuePair<std::string>("slewToDec", "$OK^");
        commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
        raUpdated = false;
        decUpdated = false;
        mountControl->gotoRaDec(raVal, decVal);
    }
}

void updateSyncCommand(uint8_t axis, double val)
{
    static bool raUpdated = false;
    static bool decUpdated = false;
    static double raVal = 0.0;
    static double decVal = 0.0;

    if (axis == LFAST::MountControl::RA_AXIS)
    {
        raVal = val;
        raUpdated = true;
    }
    else if (axis == LFAST::MountControl::DEC_AXIS)
    {
        decVal = val;
        decUpdated = true;
    }

    if (raUpdated && decUpdated)
    {
        LFAST::CommsMessage newMsg;
        newMsg.addKeyValuePair<std::string>("syncRaPosn", "$OK^");
        newMsg.addKeyValuePair<std::string>("syncDecPosn", "$OK^");
        commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
        raUpdated = false;
        decUpdated = false;
        mountControl->syncRaDec(raVal, decVal);
    }
}
