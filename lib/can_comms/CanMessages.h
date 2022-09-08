#pragma once

#include <cstdint>
#include <FlexCAN_T4.h>

///////////////// SENDER ID'S /////////////////
#define INVALID_ID          0x00
#define CTRL_PC             0x10
#define PFC_CTRL            0x20
#define PEDESTAL_CTRL       0x30
#define TEC_CTRL            0x40

#define CAN_SNDR_ID_MASK    0xF0

///////////////// CTRL PC MESSAGE ID's /////////////////
#define REQEST_STATS        0x01
#define TIMESET_REQ_ACK     0x02
#define CMD_RA_POLY_A       0x03
#define CMD_RA_POLY_B       0x04
#define CMD_RA_POLY_C       0x05
#define CMD_DEC_POLY_A      0x06
#define CMD_DEC_POLY_B      0x07
#define CMD_DEC_POLY_C      0x08
#define AZ_POSN_REQ         0x09
#define EL_POSN_REQ         0x0A

#define CAN_MSG_ID_MASK     0x0F


///////////////// MACROS /////////////////
// #define CAN_ID(s,n) (uint32_t) (((s&CAN_SNDR_ID_MASK)<<CAN_SNDR_ID_SHFT) | ((s&CAN_MSG_ID_MASK)<<CAN_MSG_ID_SHFT))

///////////////// TYPES /////////////////
typedef void (*MsgHandler)(char *, int);



///////////////// FUNCTIONS /////////////////
void initializeCanMessages();
void processMessage(const CAN_message_t &msg);
void registerCanMessageHandler(uint16_t canId, MsgHandler fn);