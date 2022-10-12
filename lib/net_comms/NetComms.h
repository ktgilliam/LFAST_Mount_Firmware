#pragma once

#include <cstdint>
#include <NativeEthernet.h>
#include <CommService.h>

#define RX_BUFF_SIZE 1024
#define PORT 4400

namespace LFAST
{
    struct NetCommsMessage : public CommsMessage
    {
        NetCommsMessage() {}
        virtual ~NetCommsMessage() {}
        NetCommsMessage(uint16_t _msgId) : client(NULL) {}
        NetCommsMessage(uint16_t _msgId, EthernetClient *_client) : client(_client) {}
        void setClient(EthernetClient *_client) { this->client = _client; }
        EthernetClient *getClient() { return this->client; }
        EthernetClient *client;
    };

    class EthernetCommsService : public CommsService
    {
    private:
        void getTeensyMacAddr(uint8_t *mac);
        static byte mac[6];
        static IPAddress ip;
        static EthernetServer server;
        std::vector<EthernetClient> clients;
        std::vector<NetCommsMessage *> netMessageQueue;

    public:
        EthernetCommsService();

        void netSendMessage(uint32_t id, char *mBuff, uint8_t len);

        bool checkForNewEnetMessages();
        void checkForNewClientData();

        // Overloaded functions:

        bool Status() { return this->commsServiceStatus; };
        // bool checkForNewMessages(){return false;}
        bool checkForNewMessages(EthernetClient &client);
        bool checkForNewClients();
        virtual void sendMessage(CommsMessage &msg);
        // virtual void sendMessage(CommsMessage<int> msg){};
        void stopDisconnectedClients();

        NetCommsMessage *CreateNewMessage(uint16_t idVal, EthernetClient client);
    };
}