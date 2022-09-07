#include "include/CanMessages.h"
#include <vector>
#include <tuple>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <FlexCAN_T4.h>
#include "include/SerialInterface.h"

void processMessage(uint32_t msgId);
void invalidMessageHandler(char *guts, int len);

typedef void (*msgHandler)(char *, int);
typedef struct
{
    uint16_t msgId;
    msgHandler msgCb;
} messageListItem;

typedef std::vector<messageListItem> MessageList;

static MessageList messageList =
    {
        {CAN_ID(INVALID, INVALID), &invalidMessageHandler}};

void initializeCanMessages()
{
}

void defaultMessageHandler(char *guts, int len)
{
    std::string msgGuts(guts, len);
    std::stringstream ss;
    ss << "No handler registered to this CAN ID." << std::endl << msgGuts << std::endl;
}

void invalidMessageHandler(char *guts, int len)
{
    std::string msgGuts(guts, len);
    std::stringstream ss;
    ss << "Invalid Message Received. Message reads: " << std::endl << msgGuts << std::endl;

}

void processMessage(const CAN_message_t &msg)
{
    uint32_t id = msg.id;
    std::stringstream ss;
    ss << "Processing message ID: " << id << std::endl;
    const auto itr
    { 
        std::find_if
        (
            messageList.begin(), 
            messageList.end(), 
            [id](const auto& msgListItem) 
            {
                return (msgListItem.msgId == id); 
            }
        ) 
    };
    
    if (itr != messageList.end())
        (*itr).msgCb((char *)msg.buf, msg.len);
    else
        defaultMessageHandler((char *)msg.buf, msg.len);
}