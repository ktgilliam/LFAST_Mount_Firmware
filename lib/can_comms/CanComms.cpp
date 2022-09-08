///
///  @ Author: Kevin Gilliam
///  @ Create Time: 2022-09-07 15:54:35
///  @ Modified by: Kevin Gilliam
///  @ Modified time: 2022-09-08 14:14:51
///  @ Description:
///

#include <CanComms.h>

#include <Arduino.h>
#include <FlexCAN_T4.h>

#include <debug.h>
#include <array>
#include <tuple>
#include <iostream>
#include <algorithm>
#include <sstream>

#include <device.h>

// #define TRANSMIT_BAUD_RATE   115200
#define TRANSMIT_BAUD_RATE 300000 // seems it must be rounded to the 10,000 (which is weird)
#define MAX_CAN_MESSAGES 0x40     // can be increased if needed
#define MAX_MESSAGE_LENGTH_BYTES 8

#if TEST_MODE_ACTIVE
#define NUM_TX_MAILBOXES 1
#define NUM_RX_MAILBOXES 1
#else
#define NUM_TX_MAILBOXES 1
#define NUM_RX_MAILBOXES 1
#endif

typedef std::array<MsgHandler, MAX_CAN_MESSAGES> MessageList;

static MessageList messageHandlerList;

typedef FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> CanBusConfig;
CanBusConfig canPort; // can1 port

void initCanDevice();
void registerCanMessageHandler(uint16_t canId, MsgHandler fn);
void defaultMessageHandler(char *guts, int len);
void processReceived(const CAN_message_t &msg);
void processTransmitted(const CAN_message_t &msg);
void sendDeadBeef();
void canSniff20(const CAN_message_t &msg);

void processMessage(uint32_t msgId);
bool printMessageInfo();

///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
void initCanComms()
{
    initCanDevice();
    for (int16_t ii = 0; ii < MAX_CAN_MESSAGES; ii++)
    {
        messageHandlerList[ii] = defaultMessageHandler;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// LOCAL/PRIVATE FUNCTIONS ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

void initCanDevice()
{
    // Enable bus and set baud rate
    canPort.begin();
    canPort.setBaudRate(TRANSMIT_BAUD_RATE); // 500kbps data rate

    canPort.setMaxMB(NUM_TX_MAILBOXES + NUM_RX_MAILBOXES);

    for (int ii = 0; ii < NUM_RX_MAILBOXES; ii++)
    {
        canPort.setMB((FLEXCAN_MAILBOX)ii, RX, STD);
    }
    for (int ii = NUM_RX_MAILBOXES; ii < (NUM_TX_MAILBOXES + NUM_RX_MAILBOXES); ii++)
    {
        canPort.setMB((FLEXCAN_MAILBOX)ii, TX, STD);
    }

    canPort.setMBFilter(REJECT_ALL);
    canPort.enableMBInterrupts();
    canPort.onReceive(MB0, processReceived);

#if TEST_MODE_ACTIVE
    canPort.onTransmit(MB1, processTransmitted);
#endif

    canPort.setMBFilter(ACCEPT_ALL);
}

void setRxMailboxFilterRange(uint16_t begin, uint16_t end)
{
    // Set up RX filters
    canPort.setMBFilterRange(MB0, begin, end);
}

void registerCanMessageHandler(uint16_t canId, MsgHandler fn)
{
    // canPort.setMBFilter
    messageHandlerList[canId] = fn;
    TEST_SERIAL.print("Registered handler to CAN ID: ");
    TEST_SERIAL.print(canId, HEX);
    TEST_SERIAL.print(" at address ");
    TEST_SERIAL.println((uint32_t)fn, HEX);
}

void registerCanRxCallback(_MB_ptr callbackFn)
{
    canPort.onReceive(FIFO, callbackFn);
}

void defaultMessageHandler(char *guts, int len)
{
    // TOGGLE_DEBUG_PIN();
    std::string msgGuts(guts, len);
    std::stringstream ss;
    ss << "Invalid/Unregistered Message ID. Payload: ";
    for (int ii = 0; ii < 8; ii++)
    {
        ss << std::hex << (uint8_t)guts[ii];
    }
    ss << std::endl;
    TEST_SERIAL.println(ss.str().c_str());
    // TOGGLE_DEBUG_PIN();
}

void processReceived(const CAN_message_t &msg)
{
    uint32_t id = msg.id;
    MsgHandler handlerFn = (MsgHandler)0;

    TOGGLE_DEBUG_PIN();
    if (id <= MAX_CAN_MESSAGES)
    {
        handlerFn = messageHandlerList[id];
    }
    if(handlerFn)
    {
        handlerFn((char *)msg.buf, msg.len);
    }

    TOGGLE_DEBUG_PIN();
}

void processTransmitted(const CAN_message_t &msg)
{
    // canSniff20(msg);
    TOGGLE_DEBUG_PIN();
    TOGGLE_DEBUG_PIN();

    TOGGLE_DEBUG_PIN();
    TOGGLE_DEBUG_PIN();

    TOGGLE_DEBUG_PIN();
    TOGGLE_DEBUG_PIN();
}
/**
 * @brief
 *
 */
void sendTestFrame()
{
    CAN_message_t msg;
    msg.id = 0x401;
    msg.len = MAX_MESSAGE_LENGTH_BYTES;
    static int d;

    for (uint8_t ii = 0; ii < msg.len; ii++)
    {
        msg.buf[ii] = ii + 1;
    }

    msg.id = 0x402;
    msg.buf[1] = d++;

    canPort.write(msg); // write to can1
}

bool printMessageInfo()
{
    bool retVal = false;
    CAN_message_t msg;

    if (updateCanBusEvents(msg))
    {
        Serial.print("MB: ");
        Serial.print(msg.mb);
        Serial.print("  OVERRUN: ");
        Serial.print(msg.flags.overrun);
        Serial.print("  ID: 0x");
        Serial.print(msg.id, HEX);
        Serial.print("  EXT: ");
        Serial.print(msg.flags.extended);
        Serial.print("  LEN: ");
        Serial.print(msg.len);
        Serial.print(" DATA: ");
        for (uint8_t ii = 0; ii < msg.len; ii++)
        {
            Serial.print(msg.buf[ii]);
            Serial.print(" ");
        }
        Serial.print("  TS: ");
        Serial.println(msg.timestamp);
        retVal = true;
    }
    return (retVal);
}

void sendDeadBeef()
{
    const char deadbeef[] = {0xDE, 0xAD, 0xBE, 0xEF};
    CAN_message_t msg;
    msg.id = 99;
    msg.len = strlen(deadbeef);
    memcpy(msg.buf, deadbeef, msg.len);
    canPort.write(msg); // write to can1
}

void updateCanBusEvents()
{
    canPort.events();
    // return (canPort.readMB(msg));
}

void sendCanBusMessage(const CAN_message_t &msg)
{
    canPort.write(msg); // write to can1
}
void sendCanBusMessage(uint32_t id, char *mBuff, uint8_t len)
{
    CAN_message_t msg;
    msg.id = 0x401;
    msg.len = len <= MAX_MESSAGE_LENGTH_BYTES ? len : MAX_MESSAGE_LENGTH_BYTES;

    for (uint8_t ii = 0; ii < msg.len; ii++)
    {
        msg.buf[ii] = mBuff[ii];
    }

    msg.id = id;
    canPort.write(msg); // write to can1
}

void canSniff20(const CAN_message_t &msg)
{ // global callback
    TEST_SERIAL.print("T4: ");
    TEST_SERIAL.print("MB ");
    TEST_SERIAL.print(msg.mb);
    TEST_SERIAL.print(" OVERRUN: ");
    TEST_SERIAL.print(msg.flags.overrun);
    TEST_SERIAL.print(" BUS ");
    TEST_SERIAL.print(msg.bus);
    TEST_SERIAL.print(" LEN: ");
    TEST_SERIAL.print(msg.len);
    TEST_SERIAL.print(" EXT: ");
    TEST_SERIAL.print(msg.flags.extended);
    TEST_SERIAL.print(" REMOTE: ");
    TEST_SERIAL.print(msg.flags.remote);
    TEST_SERIAL.print(" TS: ");
    TEST_SERIAL.print(msg.timestamp);
    TEST_SERIAL.print(" ID: ");
    TEST_SERIAL.print(msg.id, HEX);
    TEST_SERIAL.print(" IDHIT: ");
    TEST_SERIAL.print(msg.idhit);
    TEST_SERIAL.print(" Buffer: ");
    for (uint8_t i = 0; i < msg.len; i++)
    {
        TEST_SERIAL.print(msg.buf[i], HEX);
        TEST_SERIAL.print(" ");
    }
    TEST_SERIAL.println();
}
