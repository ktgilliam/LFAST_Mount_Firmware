///
///  @ Author: Kevin Gilliam
///  @ Create Time: 2022-09-06 11:44:37
///  @ Modified by: Kevin Gilliam
///  @ Modified time: 2022-09-06 14:56:08
///  @ Description:
///

#include "CanInterface.h"
#include "util/debug.h"

#include <FlexCAN_T4.h>
#include <stdint.h>

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1; // can1 port

void initCanInterfaces()
{
    pinMode(CAN1_PIN, OUTPUT);
    digitalWrite(CAN1_PIN, LOW);

    can1.begin();
    can1.setBaudRate(500000); // 500kbps data rate
    can1.enableFIFO();
    can1.enableFIFOInterrupt();

    can1.mailboxStatus();
    CONFIG_DEBUG_PIN_1();
    CLR_DEBUG_PIN_1();
}

bool updateCanBusEvents(CAN_message_t &msg)
{
    can1.events();
    return (can1.readMB(msg));
}


/**
 * @brief
 *
 */
void canSendMessage(uint32_t id, char *mBuff, uint8_t len)
{
    SET_DEBUG_PIN_1();
    CAN_message_t msg;
    msg.id = 0x401;
    msg.len = len <= MAX_MESSAGE_LENGTH ? len : MAX_MESSAGE_LENGTH;

    for (uint8_t ii = 0; ii < msg.len; ii++)
    {
        msg.buf[ii] = mBuff[ii];
    }

    msg.id = id;
    can1.write(msg); // write to can1
    CLR_DEBUG_PIN_1();
}

void canSniff(const CANFD_message_t &msg)
{
    Serial.print("ISR - MB ");
    Serial.print(msg.mb);
    Serial.print("  OVERRUN: ");
    Serial.print(msg.flags.overrun);
    Serial.print("  LEN: ");
    Serial.print(msg.len);
    Serial.print(" EXT: ");
    Serial.print(msg.flags.extended);
    Serial.print(" TS: ");
    Serial.print(msg.timestamp);
    Serial.print(" ID: ");
    Serial.print(msg.id, HEX);
    Serial.print(" Buffer: ");
    for (uint8_t i = 0; i < msg.len; i++)
    {
        Serial.print(msg.buf[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

void canSniff20(const CAN_message_t &msg)
{ // global callback
    Serial.print("T4: ");
    Serial.print("MB ");
    Serial.print(msg.mb);
    Serial.print(" OVERRUN: ");
    Serial.print(msg.flags.overrun);
    Serial.print(" BUS ");
    Serial.print(msg.bus);
    Serial.print(" LEN: ");
    Serial.print(msg.len);
    Serial.print(" EXT: ");
    Serial.print(msg.flags.extended);
    Serial.print(" REMOTE: ");
    Serial.print(msg.flags.remote);
    Serial.print(" TS: ");
    Serial.print(msg.timestamp);
    Serial.print(" ID: ");
    Serial.print(msg.id, HEX);
    Serial.print(" IDHIT: ");
    Serial.print(msg.idhit);
    Serial.print(" Buffer: ");
    for (uint8_t i = 0; i < msg.len; i++)
    {
        Serial.print(msg.buf[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

void registerCanRxCallback(_MB_ptr callbackFn)
{
    can1.onReceive(FIFO, callbackFn);
}

void sendMessage(const CAN_message_t &msg)
{
        can1.write(msg); // write to can1
}