#include "Telescope.h"

#include <NetComms.h>

#include <device.h>
#include <debug.h>

EthernetCommsService *commsService;

// Message Handlers:
void handshake(CommsMessage &handshakeMsg);

void initMountControl()
{
    commsService = new EthernetCommsService();

    if (!commsService->Status())
    {
        TEST_SERIAL.println("Device Setup Failed.");
        while (true)
        {
            ;
            ;
        }
    }
    commsService->registerMessageHandler(99, handshake);
}

void serviceMountControl()
{
    // listen for incoming Ethernet connections:
    commsService->checkForNewClientData();
    commsService->processReceived();
}

void handshake(CommsMessage &handshakeMsg)
{
    TEST_SERIAL.print("Shaking the hand!\r\n");
    commsService->sendMessage(handshakeMsg);
}

