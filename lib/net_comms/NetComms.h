#pragma once

#include <cstdint>
#include <NativeEthernet.h>


///////////////// CAN FILTER MASKS
#define CTRL_SNDR_ID_MASK    0xF00
#define CTRL_MSG_ID_MASK     0x0FF

///////////////// SENDER ID MASKS /////////////////
#define CTRL_PC_ID_MASK             0x000
#define PFC_CTRL_ID_MASK            0x100
#define PEDESTAL_CTRL_ID_MASK       0x200
#define TEC_CTRL_ID_MASK            0x400

///////////////// CTRL PC MESSAGE ID's /////////////////
typedef enum
{
 CTRL_MSG_MIN,
 REQUEST_STATS,
 TIMESET_REQ_ACK,
 CMD_RA_POLY_A,
 CMD_RA_POLY_B,
 CMD_RA_POLY_C,
 CMD_DEC_POLY_A,
 CMD_DEC_POLY_B,
 CMD_DEC_POLY_C,
 AZ_POSN_REQ,
 EL_POSN_REQ,
 CTRL_MSG_MAX
} CTRL_PC_MSG_ID;


///////////////// TYPES /////////////////
typedef void (*MsgHandler)(char *, int);
typedef struct
{
    uint32_t id;
    uint32_t len;               
    uint32_t buf[68];           /*!< Unaligned data buffer. */
} NetMessage_t;

///////////////// FUNCTIONS /////////////////
void initNetComms();

void registerNetMessageHandler(uint16_t msgId, MsgHandler fn);

void netSendMessage(uint32_t id, char *mBuff, uint8_t len);

void sendTestFrame();
void listenForEthernetClients();
void checkForNewClientData();
