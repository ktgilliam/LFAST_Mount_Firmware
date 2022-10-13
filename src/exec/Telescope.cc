#include "Telescope.h"

#include <NetComms.h>

#include <device.h>
#include <debug.h>

LFAST::EthernetCommsService *commsService;

Telescope scopeData;

// Message Handlers:
void handshake(unsigned int val);
void updateTime(double indiTimeStamp);
void sendAzElPositions(double indiTimeStamp);
void sendParkedStatus(double indiTimeStamp);
void sendTrackRate(double indiTimeStamp);
void parkScope(double indiTimeStamp);
void unparkScope(double indiTimeStamp);
void initMountControl()
{
    TEST_SERIAL.printf("\r\n\r\n\r\n\r\n##########################################################\r\n");
    commsService = new LFAST::EthernetCommsService();

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
    commsService->registerMessageHandler<double>("RequestAltAz", sendAzElPositions);
    commsService->registerMessageHandler<double>("IsParked", sendParkedStatus);
    commsService->registerMessageHandler<double>("getTrackRate", sendParkedStatus);
    commsService->registerMessageHandler<double>("Park", parkScope);
    commsService->registerMessageHandler<double>("Unpark", unparkScope);
}

void serviceMountControl()
{
    // listen for incoming Ethernet connections:
    commsService->checkForNewClients();
    commsService->checkForNewClientData();
    commsService->processClientData();
    commsService->stopDisconnectedClients();
}

void handshake(unsigned int val)
{
    LFAST::CommsMessage newMsg;
    TEST_SERIAL.print("Shaking the hand!\r\n");
    if (val == 0xDEAD)
    {
        // char json[] =
        //     "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";
        // std::memcpy(newMsg.jsonInputBuffer, json, sizeof(json));
        // newMsg.deserialize();

        newMsg.addKeyValuePair<unsigned int>("Handshake", 0xBEEF);
        commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
    }
    else
    {
        // TODO: Generate error
    }
    // commsService->txMessageQueue.push_back(newMsg);
    return;
    // commsService->sendMessage(handshakeMsg);
}

void updateTime(double indiTimeStamp)
{
    TEST_SERIAL.print("Sync'd time.\r\n");
    scopeData.indiTime = indiTimeStamp;
}

void sendAzElPositions(double indiTimeStamp)
{
    LFAST::CommsMessage newMsg;

#if SIM_SCOPE_ENABLED
    scopeData.azPosn += 0.1;
    scopeData.elPosn += 0.001;
#endif
    scopeData.indiTime = indiTimeStamp;

    newMsg.addKeyValuePair<double>("AzPosition", scopeData.azPosn);
    newMsg.addKeyValuePair<double>("ElPosition", scopeData.elPosn);
    commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
}

void sendParkedStatus(double indiTimeStamp)
{
#if SIM_SCOPE_ENABLED
    if (scopeData.scopeStatus == Telescope::SCOPE_PARKING)
    {
        if (scopeData.parkingCounter++ >= SCOPE_PARK_TIME_COUNT)
        {
            scopeData.isParked = true;
            scopeData.scopeStatus = Telescope::SCOPE_IDLE;
        }
    }

#endif
    scopeData.indiTime = indiTimeStamp;
    LFAST::CommsMessage newMsg;
    newMsg.addKeyValuePair<bool>("IsParked", scopeData.isParked);
    commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
}

void sendTrackRate(double indiTimeStamp)
{
    scopeData.indiTime = indiTimeStamp;
    LFAST::CommsMessage newMsg;
    newMsg.addKeyValuePair<double>("TrackRate", scopeData.trackRate);
    commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
}

void parkScope(double indiTimeStamp)
{
#if SIM_SCOPE_ENABLED
    scopeData.scopeStatus = Telescope::SCOPE_PARKING;
    scopeData.parkingCounter = 0;
#endif
    scopeData.indiTime = indiTimeStamp;

    LFAST::CommsMessage newMsg;
    newMsg.addKeyValuePair<std::string>("Park", "$OK^");
    commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
}

void unparkScope(double indiTimeStamp)
{
#if SIM_SCOPE_ENABLED
    scopeData.isParked = false;
#endif
    scopeData.indiTime = indiTimeStamp;

    LFAST::CommsMessage newMsg;
    newMsg.addKeyValuePair<std::string>("Park", "$OK^");
    commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
}