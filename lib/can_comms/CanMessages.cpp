///
///  @ Author: Kevin Gilliam
///  @ Create Time: 2022-09-07 15:54:35
///  @ Modified by: Kevin Gilliam
///  @ Modified time: 2022-09-08 08:25:34
///  @ Description:
///

#include "CanMessages.h"
#include <array>
#include <tuple>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <FlexCAN_T4.h>
#include <device.h>

#define MAX_CAN_MESSAGES 0x4F // can be increased if needed

void processMessage(uint32_t msgId);
void invalidMessageHandler(char *guts, int len);


typedef std::array<MsgHandler, MAX_CAN_MESSAGES> MessageList;

static MessageList messageHandlerList;

void registerCanMessageHandler(uint16_t canId, MsgHandler fn)
{
    messageHandlerList[canId] = fn;
}

void initializeCanMessages()
{
    for (int16_t ii = 0; ii < MAX_CAN_MESSAGES; ii++)
    {
        registerCanMessageHandler(ii, defaultMessageHandler);
    }
    registerCanMessageHandler(INVALID_ID, invalidMessageHandler);
}



void defaultMessageHandler(char *guts, int len)
{
    std::string msgGuts(guts, len);
    std::stringstream ss;
    ss << "No handler registered to this CAN ID." << std::endl;
    TEST_SERIAL.println(ss.str().c_str());
}

void invalidMessageHandler(char *guts, int len)
{
    std::string msgGuts(guts, len);
    std::stringstream ss;
    ss << "Invalid Message ID. Payload: ";
    for (int ii = 0; ii < 8; ii++)
    {
        ss << std::hex << (uint8_t)guts[ii];
    }
    ss << std::endl;
    TEST_SERIAL.println(ss.str().c_str());
}

void processMessage(const CAN_message_t &msg)
{
    uint32_t id = msg.id;
    MsgHandler handlerFn = defaultMessageHandler;
    if(id <= MAX_CAN_MESSAGES)
    {
        handlerFn = messageHandlerList[id];
    }
    handlerFn((char *)msg.buf, msg.len);
}