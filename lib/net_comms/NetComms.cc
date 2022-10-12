///
///  @ Author: Kevin Gilliam
///  @ Create Time: 2022-09-07 15:54:35
///  @ Modified by: Kevin Gilliam
///  @ Modified time: 2022-09-08 12:53:34
///  @ Description:
///

#include <NetComms.h>

#include <NativeEthernet.h>
#include <Arduino.h>

#include <debug.h>
#include <array>
#include <tuple>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>

#include <vector>
#include <iterator>
// #include <regex>

#include <device.h>

#define TEST_MODE_MSG_PERIOD_US 100000
// #define TRANSMIT_BAUD_RATE   115200
// #define TRANSMIT_BAUD_RATE 300000 // seems it must be rounded to the 10,000 (which is weird)

#define MAX_MESSAGE_LENGTH_BYTES 64

// assign a MAC address for the Ethernet controller.
// fill in your address here:
// byte mac[] = {0x04, 0xe9, 0xBE, 0xEF, 0xFE, 0xED};
// assign an IP address for the controller:

// IPAddress ip(10, 0, 0, 177);
// IPAddress myDns(192, 168, 1, 1);
// IPAddress gateway(192, 168, 190, 1);
// IPAddress subnet(255, 255, 255, 0);

// Initialize the Ethernet server library
// with the IP address and port you want to use

void stopDisconnectedClients();
void getTeensyMacAddr(uint8_t *mac);

IPAddress LFAST::EthernetCommsService::ip = IPAddress(192, 168, 121, 177);
EthernetServer LFAST::EthernetCommsService::server = EthernetServer(PORT);
byte LFAST::EthernetCommsService::mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
LFAST::EthernetCommsService::EthernetCommsService()
{
    bool initResult = true;

    TEST_SERIAL.println("\nInitializing Ethernet... ");
    getTeensyMacAddr(mac);
    // Ethernet.MACAddress(mac);
    TEST_SERIAL.printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                       mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    // TEST_SERIAL.println("");
    // initialize the Ethernet device
    Ethernet.begin(mac, ip);
    // Ethernet.begin(mac, ip, myDns, gateway, subnet)

    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
        TEST_SERIAL.println("Ethernet PHY was not found.  Sorry, can't run without hardware. :(");
        initResult = false;
    }

    // TEST_SERIAL.println("Checking Link...");
    if (Ethernet.linkStatus() == LinkOFF)
    {
        TEST_SERIAL.println("Ethernet cable is not connected.");
        initResult = false;
    }

    if (initResult)
    {
        // start listening for clients
        this->server.begin(PORT);
        TEST_SERIAL.print("Start listening... Local IP: ");
        TEST_SERIAL.println(Ethernet.localIP());
    }
    this->commsServiceStatus = initResult;
}

bool LFAST::EthernetCommsService::checkForNewClients()
{
    bool newClientFlag = false;
    // check for any new client connecting, and say hello (before any incoming data)
    // TEST_SERIAL.print("Checking for new clients");
    EthernetClient newClient = server.accept();
    if (newClient)
    {
        newClientFlag = true;
        TEST_SERIAL.printf("New client # %d\r\n", clients.size() + 1);
        // Once we "accept", the client is no longer tracked by EthernetServer
        // so we must store it into our list of clients
        clients.push_back(newClient);

        // for (byte ii = 0; ii < clients.size(); ii++)
        // {
        //     TEST_SERIAL.println(ii);
        //     // Once we "accept", the client is no longer tracked by EthernetServer
        //     // so we must store it into our list of clients
        //     clients.push_back(newClient);
        //     break;
        // }
    }
    return (newClientFlag);
}

void LFAST::EthernetCommsService::checkForNewClientData()
{
    checkForNewClients();

    // check for incoming data from all clients
    // for (byte ii = 0; ii < MAX_CLIENTS; ii++)

    for (auto &client : this->clients)
    {
        if (client.available())
        {
            checkForNewMessages(client);
        }
    }
    // for (auto itr = clients.begin(); itr != clients.end(); itr++)
    // {
    //     if ((*itr).available())
    //     {
    //         checkForNewMessages(*itr);
    //     }
    // }
    stopDisconnectedClients();
}

void LFAST::EthernetCommsService::stopDisconnectedClients()
{
    // stop any clients which disconnect
    // for (auto &client : this->clients)
    // {
    //     if (!client.connected())
    //     {
    //         TEST_SERIAL.print("disconnect client #");
    //         // TEST_SERIAL.println(ii);
    //         client.stop();
    //     }
    // }

    // this->clients.erase(std::remove_if(this->clients.begin(), this->clients.end(), 
    //                    [](EthernetClient client) { return !client.connected(); }), this->clients.end());

    auto itr = clients.begin();
    while (itr != clients.end())
    {
        if (!(*itr).connected())
        {
            // TEST_SERIAL.println("Disconnected client.");
            // TEST_SERIAL.println(ii);
            (*itr).stop();
            itr = clients.erase(itr);
        }
        else
        {
            itr++;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// LOCAL/PRIVATE FUNCTIONS ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

void LFAST::EthernetCommsService::getTeensyMacAddr(uint8_t *mac)
{
    for (uint8_t by = 0; by < 2; by++)
        mac[by] = (HW_OCOTP_MAC1 >> ((1 - by) * 8)) & 0xFF;
    for (uint8_t by = 0; by < 4; by++)
        mac[by + 2] = (HW_OCOTP_MAC0 >> ((3 - by) * 8)) & 0xFF;
}

bool LFAST::EthernetCommsService::checkForNewMessages(EthernetClient &client)
{
    // listen for incoming clients
    if (client)
    {
        uint8_t readBuff[RX_BUFF_SIZE];
        memset(readBuff, 0, RX_BUFF_SIZE);
        unsigned int bytesRead = 0;
        while (client.connected())
        {
            // TEST_SERIAL.println("Checking connected client messages");
            if (client.available())
            {
                char c = client.read();
                // TEST_SERIAL.print(c);
                // TEST_SERIAL.printf("%u ", (unsigned int) c);
                // if (c == '\0') TEST_SERIAL.printf("%u ", (unsigned int) c);
                
                if ((c == '\0') || (bytesRead >= RX_BUFF_SIZE))
                {
                    // NetCommsMessage newMsg;
                    // TEST_SERIAL.printf("\nReceived Data: [%s]\r\n", (char *)readBuff);
                    // auto newMsg = new NetCommsMessage();
                    NetCommsMessage newMsg;
                    newMsg.loadReceivedData((char *)readBuff, bytesRead);

                    // TEST_SERIAL.println("Parsing done.");
                    this->messageQueue.push_back(newMsg);
                    break;
                }
                else
                {
                    readBuff[bytesRead++] = c;
                }
            }
        }
        delay(1);
        // close the connection:
        // client.stop();
    }
    return true;
}

void LFAST::EthernetCommsService::sendMessage(CommsMessage &msg)
{
    // std::string msgStr = msg.getMessageStr();
}
