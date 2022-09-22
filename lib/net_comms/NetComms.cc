///
///  @ Author: Kevin Gilliam
///  @ Create Time: 2022-09-07 15:54:35
///  @ Modified by: Kevin Gilliam
///  @ Modified time: 2022-09-08 12:53:34
///  @ Description:
///

#include <NetComms.h>

#include <NativeEthernet.h>

#include <debug.h>
#include <array>
#include <tuple>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>

#include <device.h>

#include <iterator>
#include <string>
#include <regex>

#define TEST_MODE_MSG_PERIOD_US 100000
// #define TRANSMIT_BAUD_RATE   115200
#define TRANSMIT_BAUD_RATE 300000 // seems it must be rounded to the 10,000 (which is weird)
#define MAX_CTRL_MESSAGES 0x40    // can be increased if needed
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

typedef std::array<MsgHandler, MAX_CTRL_MESSAGES> MessageList;

static MessageList messageHandlerList;

bool initNetDevice();
void defaultMessageHandler(char *guts, int len);

void checkForNewClientConnection();
void stopDisconnectedClients();
void getTeensyMacAddr(uint8_t *mac);

///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
bool initNetComms()
{
    if (initNetDevice())
    {
        for (int16_t ii = 0; ii < MAX_CTRL_MESSAGES; ii++)
        {
            registerNetMessageHandler(ii, defaultMessageHandler);
        }
    }
    else
    {
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// LOCAL/PRIVATE FUNCTIONS ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
bool initNetDevice()
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

    // TEST_SERIAL.println("Checking PHY...");
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
    return initResult;
}

void registerNetMessageHandler(uint16_t msgId, MsgHandler fn)
{
    messageHandlerList[msgId] = fn;
}

void defaultMessageHandler(char *guts, int len)
{
    std::string msgGuts(guts, len);
    std::stringstream ss;
    ss << "Invalid/Unregistered Message ID. Payload: ";
    for (int ii = 0; ii < 8; ii++)
    {
        ss << std::hex << (uint8_t)guts[ii];
    }
    ss << std::endl;
    TEST_SERIAL.println(ss.str().c_str());
}

void processReceived(const NetMessage_t &msg)
{
    uint32_t id = msg.id;
    MsgHandler handlerFn = defaultMessageHandler;

    TOGGLE_DEBUG_PIN();
    if (id <= MAX_CTRL_MESSAGES)
    {
        handlerFn = messageHandlerList[id];
    }
    handlerFn((char *)msg.buf, msg.len);
}

void processTransmitted(const NetMessage_t &msg)
{
    TOGGLE_DEBUG_PIN();
}
/**
 * @brief
 *
 */
void sendTestFrame()
{
    NetMessage_t msg;
    msg.id = 0x401;
    msg.len = MAX_MESSAGE_LENGTH_BYTES;
    static int d;

    for (uint8_t ii = 0; ii < msg.len; ii++)
    {
        msg.buf[ii] = ii + 1;
    }

    msg.id = 0x402;
    msg.buf[1] = d++;

    // canPort.write(msg); // write to can1
}

bool printMessageInfo()
{
    bool retVal = false;
    NetMessage_t msg;
    // TODO
    return (retVal);
}

void sendDeadBeef()
{
    const char deadbeef[] = {0xDE, 0xAD, 0xBE, 0xEF};
    NetMessage_t msg;
    msg.id = 99;
    msg.len = strlen(deadbeef);
    memcpy(msg.buf, deadbeef, msg.len);
    // canPort.write(msg); // write to can1
}

void checkForNewMessages()
{
    // listen for incoming clients
    EthernetClient client = server.available();
#define RX_BUFF_SIZE 64
    if (client)
    {
        uint8_t readBuff[RX_BUFF_SIZE];
        memset(readBuff, 0, RX_BUFF_SIZE);
        unsigned int bytesRead = 0;
        bool eom = false;
        while (client.connected())
        {
            if (client.available())
            {
                char c = client.read();

                if (c == '\n')
                {
                    eom = true;
                }
                else
                {
                    readBuff[bytesRead++] = c;
                }

                if (eom)
                {
                    std::string recvStr((char *)readBuff);
                    TEST_SERIAL.print("\nMSG: ");
                    TEST_SERIAL.println((char *)readBuff);
                    // TEST_SERIAL.println("Replied 1#");
                    client.println("1#");

                    std::regex id_regex("#\\d+");
                    auto msg_begin =
                        std::sregex_iterator(recvStr.begin(), recvStr.end(), id_regex);
                    auto msg_end = std::sregex_iterator();
                    for (std::sregex_iterator i = msg_begin; i != msg_end; ++i)
                    {
                        std::smatch match = *i;
                        std::string id_str = match.str().substr(1,std::string::npos);
                        unsigned int id = atoi(id_str.c_str());
                        TEST_SERIAL.print("Found ID: ");
                        TEST_SERIAL.println(id);

                    }
                    break;
                }
            }
        }
        delay(1);
        // close the connection:
        // client.stop();
    }
}

void checkForNewClientData()
{
    checkForNewClientConnection();

    // check for incoming data from all clients
    for (byte ii = 0; ii < MAX_CLIENTS; ii++)
    {
        if (clients[ii] && clients[ii].available() > 0)
        {
            // read bytes from a client
            byte buffer[80];
            int count = clients[ii].read(buffer, MAX_MESSAGE_LENGTH_BYTES);
            // write the bytes to all other connected clients
            for (byte jj = 0; jj < MAX_CLIENTS; jj++)
            {
                if (jj != ii && clients[jj].connected())
                {
                    clients[jj].write(buffer, count);
                }
            }
        }
    }
    stopDisconnectedClients();
}

void checkForNewClientConnection()
{
    // check for any new client connecting, and say hello (before any incoming data)
    EthernetClient newClient = server.accept();
    if (newClient)
    {
        for (byte ii = 0; ii < MAX_CLIENTS; ii++)
        {
            if (!clients[ii])
            {
                TEST_SERIAL.print("We have a new client #");
                TEST_SERIAL.println(ii);
                newClient.print("Hello, client number: ");
                newClient.println(ii);
                // Once we "accept", the client is no longer tracked by EthernetServer
                // so we must store it into our list of clients
                clients[ii] = newClient;
                break;
            }
        }
    }
}

void stopDisconnectedClients()
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

void getTeensyMacAddr(uint8_t *mac)
{
    for (uint8_t by = 0; by < 2; by++)
        mac[by] = (HW_OCOTP_MAC1 >> ((1 - by) * 8)) & 0xFF;
    for (uint8_t by = 0; by < 4; by++)
        mac[by + 2] = (HW_OCOTP_MAC0 >> ((3 - by) * 8)) & 0xFF;
}