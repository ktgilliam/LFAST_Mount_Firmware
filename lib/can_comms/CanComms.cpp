///
///  @ Author: Kevin Gilliam
///  @ Create Time: 2022-09-07 15:54:35
 ///  @ Modified by: Kevin Gilliam
 ///  @ Modified time: 2022-09-08 12:52:51
///  @ Description:
///

#include <CanComms.h>

#include <FlexCAN_T4.h>

#include <debug.h>
#include <array>
#include <tuple>
#include <iostream>
#include <algorithm>
#include <sstream>

#include <device.h>

#define TEST_MODE_MSG_PERIOD_US 100000
// #define TRANSMIT_BAUD_RATE   115200
#define TRANSMIT_BAUD_RATE      300000 // seems it must be rounded to the 10,000 (which is weird)
#define MAX_CAN_MESSAGES 0x40 // can be increased if needed
#define MAX_MESSAGE_LENGTH_BYTES 8

#define NUM_TX_MAILBOXES 1
#define NUM_RX_MAILBOXES 1

typedef std::array<MsgHandler, MAX_CAN_MESSAGES> MessageList;

static MessageList messageHandlerList;
IntervalTimer testExecTimer;
CanTestMode testMode;

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
        registerCanMessageHandler(ii, defaultMessageHandler);
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
    for (int ii = NUM_RX_MAILBOXES; ii<(NUM_TX_MAILBOXES + NUM_RX_MAILBOXES); ii++)
    {
        canPort.setMB((FLEXCAN_MAILBOX)ii,TX,STD);
    }

    // canPort.setMBFilter(REJECT_ALL);
    canPort.enableMBInterrupts();
    canPort.onReceive(MB0, processReceived);
    canPort.onTransmit(processTransmitted);
    // canPort.setMBFilterRange(MB0, CTRL_PC_FILT_BEGIN, CTRL_PC_FILT_END);
    canPort.mailboxStatus();
}

void registerCanMessageHandler(uint16_t canId, MsgHandler fn)
{
    messageHandlerList[canId] = fn;
}

void setCanTestMode(CanTestMode mode)
{
    if (mode == CAN_TEST_MODE_TALKER)
    {
        testExecTimer.begin(sendTestFrame, TEST_MODE_MSG_PERIOD_US);
        // testExecTimer.begin(sendDeadBeef, 50000); // Send frame every 500ms
    }
    else
    {
        testExecTimer.end();
    }
}

void registerCanRxCallback(_MB_ptr callbackFn)
{
    canPort.onReceive(FIFO, callbackFn);
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

void processReceived(const CAN_message_t &msg)
{
    uint32_t id = msg.id;
    MsgHandler handlerFn = defaultMessageHandler;
    
    TOGGLE_DEBUG_PIN();
    if (id <= MAX_CAN_MESSAGES)
    {
        handlerFn = messageHandlerList[id];
    }
    handlerFn((char *)msg.buf, msg.len);
}

void processTransmitted(const CAN_message_t &msg)
{
    TOGGLE_DEBUG_PIN();
    canSniff20(msg);
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

void canSendMessage(uint32_t id, char *mBuff, uint8_t len)
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

