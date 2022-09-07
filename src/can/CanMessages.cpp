#include "CanMessages.h"
#include <vector>
#include <tuple>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <FlexCAN_T4.h>
#include "util/SerialInterface.h"

void processMessage(uint32_t msgId);
void invalidMessageHandler(char *guts, int len);

typedef void (*msgHandler)(char *, int);
typedef struct
{
    uint16_t msgId;
    msgHandler msgCb;
} messageListItem;

typedef std::vector<messageListItem> MessageList;

static MessageList messageList
    {
        {CAN_ID(INVALID_ID, INVALID_ID), &invalidMessageHandler}
    };

void initializeCanMessages()
{
}

void defaultMessageHandler(char *guts, int len)
{
    std::string msgGuts(guts, len);
    std::stringstream ss;
    ss << "No handler registered to this CAN ID." << std::endl;
    SERIAL_CH(TEST_SERIAL_NO).println(ss.str().c_str());
}

void invalidMessageHandler(char *guts, int len)
{
    std::string msgGuts(guts, len);
    std::stringstream ss;
    ss << "Invalid Message ID. Payload: ";
    for(int ii = 0; ii < 8; ii++)
    { 
        ss << std::hex << (uint8_t)guts[ii]; 
    }
    ss << std::endl;
    SERIAL_CH(TEST_SERIAL_NO).println(ss.str().c_str());
}

void processMessage(const CAN_message_t &msg)
{
    uint32_t id = msg.id;
    std::stringstream ss;
    ss << "Processing message ID: " << id << std::endl;
    const auto itr{
        std::find_if(
            messageList.begin(),
            messageList.end(),
            [id](const auto &msgListItem)
            {
                return (msgListItem.msgId == id);
            })};

    if (itr != messageList.end())
        (*itr).msgCb((char *)msg.buf, msg.len);
    else
        defaultMessageHandler((char *)msg.buf, msg.len);
}