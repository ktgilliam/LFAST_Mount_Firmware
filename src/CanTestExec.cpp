
#include "include/CanTestExec.h"
#include "include/CanMessages.h"
#include "include/CanInterface.h"
#include <stdio.h>
#include <string.h>


void parseMessage(const CAN_message_t &msg);


void initCanTestExec()
{
    registerCanRxCallback(parseMessage);
}

void sendDeadBeef()
{
    const char deadbeef[] = {0xDE, 0xAD, 0xBE, 0xEF};
    CAN_message_t msg;
    msg.id = CAN_ID(INVALID, INVALID);
    msg.len = strlen(deadbeef);
    memcpy(msg.buf, deadbeef, msg.len);
    sendMessage(msg);
}

void parseMessage(const CAN_message_t &msg)
{
    processMessage(msg);
}

/**
 * @brief
 *
 */
void sendTestFrame()
{
    CAN_message_t msg;
    msg.id = 0x401;
    msg.len = MAX_MESSAGE_LENGTH;
    static int d;

    for (uint8_t i = 0; i < msg.len; i++)
    {
        msg.buf[i] = i + 1;
    }

    msg.id = 0x402;
    msg.buf[1] = d++;


    sendMessage(msg);
}