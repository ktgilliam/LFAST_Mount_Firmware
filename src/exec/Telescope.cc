#include "Telescope.h"

#include <NetComms.h>

#include <device.h>
#include <debug.h>

LFAST::EthernetCommsService *commsService;

// Message Handlers:
void handshake(unsigned int val);

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
    // TEST_SERIAL.println("Device Setup Complete.");
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
