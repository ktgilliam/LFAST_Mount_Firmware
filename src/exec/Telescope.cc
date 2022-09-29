#include "Telescope.h"

#include <NetComms.h>

#include <device.h>
#include <debug.h>

EthernetCommsService *commsService;

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
}

void handshake(CommsMessage &dontCare)
{
    TEST_SERIAL.print("Shaking the hand!\r\n");
    
}

void serviceMountControl()
{
    // listen for incoming Ethernet connections:
    commsService->checkForNewClientData();
    commsService->processReceived();
}