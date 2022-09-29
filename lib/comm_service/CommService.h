#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <sstream>

#include <device.h>
#include <debug.h>
// #include <initializer_list>

#define MAX_ARGS 4

///////////////// CAN FILTER MASKS
#define CTRL_SNDR_ID_MASK 0xF00
#define CTRL_MSG_ID_MASK 0x0FF

///////////////// SENDER ID MASKS /////////////////
#define CTRL_PC_ID_MASK 0x000
#define PFC_CTRL_ID_MASK 0x100
#define PEDESTAL_CTRL_ID_MASK 0x200
#define TEC_CTRL_ID_MASK 0x400

///////////////// CTRL PC MESSAGE ID's /////////////////
typedef enum
{
    HANDSHAKE,
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

#define MAX_CTRL_MESSAGES 0x40U // can be increased if needed

///////////////// TYPES /////////////////
class CommsMessage
{
public:
    CommsMessage(){}
    virtual ~CommsMessage(){}
    virtual void placeholder(){}
    CommsMessage(uint16_t _msgId) : id(_msgId) {}
    uint16_t id;
    std::vector<double> args;
    void printMessageInfo();
    std::string getMessageStr();
    void parseReceivedData(char *rxBuff);
};

class CommsService
{

protected:
    static void defaultMessageHandler(CommsMessage &dontCare);
    void errorMessageHandler(CommsMessage &msg);

    bool commsServiceStatus;
    std::vector<CommsMessage *> messageQueue;

    typedef void (*MsgHandlerFn)(CommsMessage &);
    typedef std::array<MsgHandlerFn, MAX_CTRL_MESSAGES> MessageHandlerList;
    MessageHandlerList messageHandlerList;

private:
public:
    CommsService();
    virtual ~CommsService() {}

    void registerMessageHandler(uint16_t msgId, MsgHandlerFn fn);
    virtual bool Status() { return commsServiceStatus; };
    virtual bool checkForNewMessages() { return false; };
    virtual bool checkForNewClients() { return false; };
    virtual void sendMessage(CommsMessage &msg){};
    virtual void stopDisconnectedClients(){};
    virtual void processReceived(){};
    virtual void parseReceivedData(){};
};
