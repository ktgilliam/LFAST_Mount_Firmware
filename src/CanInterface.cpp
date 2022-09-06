///
///  @ Author: Kevin Gilliam
///  @ Create Time: 2022-09-06 11:44:37
///  @ Modified by: Kevin Gilliam
///  @ Modified time: 2022-09-06 14:56:08
///  @ Description:
///

#include "include/CanInterface.h"

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
    can1.onReceive(FIFO, canSniff20);
    can1.mailboxStatus();
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
void sendTestFrame()
{
    CAN_message_t msg2;
    msg2.id = 0x401;
    static int d = 0;

    for (uint8_t i = 0; i < 8; i++)
    {
        msg2.buf[i] = i + 1;
    }

    msg2.id = 0x402;
    msg2.buf[1] = d++;
    can1.write(msg2); // write to can1

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