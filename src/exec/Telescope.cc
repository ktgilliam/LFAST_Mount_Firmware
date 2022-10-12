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
    commsService->checkForNewClientData();
    commsService->processReceived();
}

void handshake(unsigned int val)
{

    TEST_SERIAL.print("Shaking the hand!\r\n");
    if(val == 0xDEAD)
    {
        // TODO: Generate message
    }
    else
    {
        // TODO: Generate error
    }
    // commsService->sendMessage(handshakeMsg);
}

