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
#include <cstring>
#include <vector>
#include <iterator>
// #include <regex>

#include <device.h>

#define TEST_MODE_MSG_PERIOD_US 100000
// #define TRANSMIT_BAUD_RATE   115200
// #define TRANSMIT_BAUD_RATE 300000 // seems it must be rounded to the 10,000 (which is weird)

#define MAX_MESSAGE_LENGTH_BYTES 64
#define MAX_CLIENTS 8

// assign a MAC address for the Ethernet controller.
// fill in your address here:
// byte mac[] = {0x04, 0xe9, 0xBE, 0xEF, 0xFE, 0xED};
// assign an IP address for the controller:
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 121, 177);
uint32_t port = 4400;
// IPAddress ip(10, 0, 0, 177);
// IPAddress myDns(192, 168, 1, 1);
// IPAddress gateway(192, 168, 190, 1);
// IPAddress subnet(255, 255, 255, 0);

// Initialize the Ethernet server library
// with the IP address and port you want to use
EthernetServer server(port);
EthernetClient clients[MAX_CLIENTS];

void stopDisconnectedClients();
void getTeensyMacAddr(uint8_t *mac);

///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
EthernetCommsService::EthernetCommsService()
{
    bool initResult = true;

    TEST_SERIAL.println("\nInitializing Ethernet... ");
    getTeensyMacAddr(mac);
    // Ethernet.MACAddress(mac);
    TEST_SERIAL.printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
                       mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    TEST_SERIAL.println("");
    // initialize the Ethernet device
    Ethernet.begin(mac, ip);
    // Ethernet.begin(mac, ip, myDns, gateway, subnet)

    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
        TEST_SERIAL.println("Ethernet PHY was not found.  Sorry, can't run without hardware. :(");
        initResult = false;
    }

    TEST_SERIAL.println("Checking Link...");
    if (Ethernet.linkStatus() == LinkOFF)
    {
        TEST_SERIAL.println("Ethernet cable is not connected.");
        initResult = false;
    }

    // start listening for clients
    server.begin(port);
    TEST_SERIAL.print("Start listening... Local IP: ");
    TEST_SERIAL.println(Ethernet.localIP());
    this->commsServiceStatus = initResult;
}

bool EthernetCommsService::checkForNewClients()
{
    bool newClientFlag = false;
    // check for any new client connecting, and say hello (before any incoming data)
    // TEST_SERIAL.print("Checking for new clients");
    EthernetClient newClient = server.accept();
    if (newClient)
    {
        newClientFlag = true;
        for (byte ii = 0; ii < MAX_CLIENTS; ii++)
        {
            if (!clients[ii])
            {
                TEST_SERIAL.print("New client #");
                TEST_SERIAL.println(ii);
                // Once we "accept", the client is no longer tracked by EthernetServer
                // so we must store it into our list of clients
                clients[ii] = newClient;
                break;
            }
        }
    }
    return (newClientFlag);
}

void EthernetCommsService::checkForNewClientData()
{
    checkForNewClients();

    // check for incoming data from all clients
    for (byte ii = 0; ii < MAX_CLIENTS; ii++)
    {
        if (clients[ii] && clients[ii].available() > 0)
        {
            checkForNewMessages(clients[ii]);
        }
    }
    stopDisconnectedClients();
}

void EthernetCommsService::stopDisconnectedClients()
{
    // stop any clients which disconnect
    for (byte ii = 0; ii < MAX_CLIENTS; ii++)
    {
        if (clients[ii] && !clients[ii].connected())
        {
            TEST_SERIAL.print("disconnect client #");
            TEST_SERIAL.println(ii);
            clients[ii].stop();
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// LOCAL/PRIVATE FUNCTIONS ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

void EthernetCommsService::getTeensyMacAddr(uint8_t *mac)
{
    for (uint8_t by = 0; by < 2; by++)
        mac[by] = (HW_OCOTP_MAC1 >> ((1 - by) * 8)) & 0xFF;
    for (uint8_t by = 0; by < 4; by++)
        mac[by + 2] = (HW_OCOTP_MAC0 >> ((3 - by) * 8)) & 0xFF;
}

bool EthernetCommsService::checkForNewMessages(EthernetClient &client)
{
    // listen for incoming clients
    if (client)
    {
        uint8_t readBuff[RX_BUFF_SIZE];
        memset(readBuff, 0, RX_BUFF_SIZE);
        unsigned int bytesRead = 0;
        while (client.connected())
        {
            if (client.available())
            {
                char c = client.read();

                if ((c == '\n') || (bytesRead >= RX_BUFF_SIZE))
                {
                    this->parseReceivedData((char *)readBuff);
                    // TEST_SERIAL.println("Parsing done.");
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

void EthernetCommsService::parseReceivedData(char *rxBuff)
{
    std::vector<std::string> argStrs;
    char *token = std::strtok(rxBuff, "#;");

    while (token != NULL)
    {
        argStrs.push_back(std::string(token));
        token = strtok(NULL, "#;");
    }

    uint16_t idVal = std::atoi(argStrs.back().c_str());
    argStrs.pop_back();

    // CommsMessage *newMsg = new CommsMessage(idVal);
    CommsMessage newMsg(idVal);
    for (uint16_t ii = 0; ii < argStrs.size(); ii++)
    {
        double argDbl = std::atof(argStrs.at(ii).c_str());
        newMsg.args.push_back(argDbl);
    }

    this->messageQueue.push_back(newMsg);
}