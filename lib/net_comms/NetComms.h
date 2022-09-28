#pragma once

#include <cstdint>
#include <NativeEthernet.h>
#include <CommService.h>

#define RX_BUFF_SIZE 64
class EthernetCommsService : public CommsService
{
private:
    void getTeensyMacAddr(uint8_t *mac);

public:

    EthernetCommsService();

    void netSendMessage(uint32_t id, char *mBuff, uint8_t len);

    bool checkForNewEnetMessages();
    void checkForNewClientData();

// Overloaded functions:

    bool Status() { return this->commsServiceStatus ; };
    // bool checkForNewMessages(){return false;}
    bool checkForNewMessages(EthernetClient &client);
    bool checkForNewClients();
    virtual void sendMessage(CommsMessage &msg) {};
    // virtual void sendMessage(CommsMessage<int> msg){};
    void stopDisconnectedClients();
    void parseReceivedData(char* rxBuff);
};