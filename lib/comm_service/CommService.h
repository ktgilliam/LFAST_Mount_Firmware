#pragma once

#include <cstdint>
// #include <array>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <cstring>
#include <device.h>
#include <debug.h>
// #include <initializer_list>
#include <ArduinoJson.h>
#include <Arduino.h>
#include "Client.h"

#define MAX_ARGS 4
#define RX_BUFF_SIZE 1024

#define MAX_KV_PAIRS 20
#define JSON_PROGMEM_SIZE JSON_OBJECT_SIZE(MAX_KV_PAIRS)
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

namespace LFAST
{
    ///////////////// TYPES /////////////////
    class CommsMessage
    {
    public:
        // CommsMessage(){}
        CommsMessage()
        {
            std::memset(this->jsonInputBuffer, 0, sizeof(this->jsonInputBuffer));
            processed = false;
        }
        virtual ~CommsMessage() {} // TEST_SERIAL.println("Destructor called."); delete[] msgBuff;}
        virtual void placeholder() {}
        void printMessageInfo();
        std::string getMessageStr();
        // std::string MessageKey;
        StaticJsonDocument<JSON_PROGMEM_SIZE> &getJsonDoc() { return this->JsonDoc; }
        StaticJsonDocument<JSON_PROGMEM_SIZE> &deserialize();

        template <typename T>
        inline T getValue(std::string key);
        template <typename T>
        inline void addKeyValuePair(std::string key, T val);
        inline void addDestinationKey(std::string key);

        const char *getBuffPtr() { return jsonInputBuffer; };
        char jsonInputBuffer[JSON_PROGMEM_SIZE];
        void setProcessedFlag() { processed = true; }
        bool hasBeenProcessed() { return processed; }

    protected:
        StaticJsonDocument<JSON_PROGMEM_SIZE> JsonDoc;
        bool processed;
        std::string destKey;
        // Client *client;
        // char *msgBuff;
    };

    template <class T>
    struct MessageHandler
    {
        void (*MsgHandlerFn)(T);

        MessageHandler() { this->MsgHandlerFn = nullptr; }

        MessageHandler(void (*ptr)(T))
        {
            this->MsgHandlerFn = ptr;
        }
        virtual ~MessageHandler(){};

        bool call(T val)
        {
            if (this->MsgHandlerFn)
            {
                MsgHandlerFn(val);
                return true;
            }
            return false;
        }
    };

    struct ClientConnection
    {
        ClientConnection(Client *_client) : client(_client) {}
        Client *client;
        std::vector<CommsMessage*> rxMessageQueue;
        std::vector<CommsMessage*> txMessageQueue;
    };
    class CommsService
    {

    protected:
        static void defaultMessageHandler(std::string);
        void errorMessageHandler(CommsMessage &msg);
        static std::vector<ClientConnection> connections;
        ClientConnection *activeConnection;
        bool commsServiceStatus;

        // std::vector<CommsMessage> rxMessageQueue;
        // std::vector<CommsMessage> txMessageQueue;
        // typedef void (*MsgHandlerFn)(CommsMessage *);
        // typedef std::array<MsgHandlerFn, MAX_CTRL_MESSAGES> MessageHandlerList;
        // typedef std::map<std::string, MsgHandlerFn> MessageHandlerList;
        // MessageHandlerList messageHandlerList;

    private:
        enum HandlerType
        {
            INT_HANDLER,
            UINT_HANDLER,
            DOUBLE_HANDLER,
            BOOL_HANDLER,
            STRING_HANDLER
        };
        std::unordered_map<std::string, HandlerType> handlerTypes;
        std::unordered_map<std::string, MessageHandler<int>> intHandlers;
        std::unordered_map<std::string, MessageHandler<unsigned int>> uIntHandlers;
        std::unordered_map<std::string, MessageHandler<double>> doubleHandlers;
        std::unordered_map<std::string, MessageHandler<bool>> boolHandlers;
        std::unordered_map<std::string, MessageHandler<std::string>> stringHandlers;
        template <class T>
        bool callMessageHandler(std::string key, T val);

    public:
        CommsService();
        virtual ~CommsService() {}

        void setupClientMessageBuffers(Client *client);
        bool getNewMessages(ClientConnection &);
        enum
        {
            ACTIVE_CONNECTION = 1,
            ALL_CONNECTED = 2,
        };
        virtual void sendMessage(CommsMessage &, uint8_t);
        template <class T>
        inline bool registerMessageHandler(std::string key, MessageHandler<T> fn);
        inline bool callMessageHandler(JsonPair kvp);

        virtual bool Status() { return commsServiceStatus; };
        // virtual bool checkForNewMessages() { return false; };

        virtual bool checkForNewClients();
        virtual void stopDisconnectedClients();
        // virtual void processReceived();
        void checkForNewClientData();
        virtual void processClientData();
        virtual void processMessage(CommsMessage *);

        // void clearRxMessageQueue();
        // void queueTxMessage(CommsMessage &);
        // protected:
        //     StaticJsonDocument<500> jsonDoc;
    };

    // NOTE: Teensy build environment doesn't handle build flags properly, so can't use typeid().
    // template <class T>
    // bool LFAST::CommsService::registerMessageHandler(std::string key, MessageHandler<T> fn)
    // {
    //     if (typeid(T) == typeid(int))
    //         this->intHandlers[key] = fn;
    //     else if (typeid(T) == typeid(unsigned int))
    //         this->uIntHandlers[key] = fn;
    //     else if (typeid(T) == typeid(double))
    //         this->doubleHandlers[key] = fn;
    //     else if (typeid(T) == typeid(bool))
    //         this->boolHandlers[key] = fn;
    //     else if (typeid(T) == typeid(std::string))
    //         this->stringHandlers[key] = fn;
    //     else
    //         return false;
    //     return true;
    // }

    template <class T>
    bool LFAST::CommsService::registerMessageHandler(std::string key, MessageHandler<T> fn)
    {
        // TODO: Add exception handling
        return false;
    }

    template <>
    inline bool LFAST::CommsService::registerMessageHandler(std::string key, MessageHandler<int> fn)
    {
        this->intHandlers[key] = fn;
        this->handlerTypes[key] = INT_HANDLER;
        return true;
    }
    template <>
    inline bool LFAST::CommsService::registerMessageHandler(std::string key, MessageHandler<unsigned int> fn)
    {
        this->uIntHandlers[key] = fn;
        this->handlerTypes[key] = UINT_HANDLER;
        return true;
    }
    template <>
    inline bool LFAST::CommsService::registerMessageHandler(std::string key, MessageHandler<double> fn)
    {
        this->doubleHandlers[key] = fn;
        this->handlerTypes[key] = DOUBLE_HANDLER;
        return true;
    }
    template <>
    inline bool LFAST::CommsService::registerMessageHandler(std::string key, MessageHandler<bool> fn)
    {
        this->boolHandlers[key] = fn;
        this->handlerTypes[key] = BOOL_HANDLER;
        return true;
    }
    template <>
    inline bool LFAST::CommsService::registerMessageHandler(std::string key, MessageHandler<std::string> fn)
    {
        this->stringHandlers[key] = fn;
        this->handlerTypes[key] = STRING_HANDLER;
        return true;
    }

    template <>
    inline bool LFAST::CommsService::callMessageHandler(std::string key, int val)
    {
        auto mh = this->intHandlers[key];
        mh.call(val);
        return true;
    }

    template <>
    inline bool LFAST::CommsService::callMessageHandler(std::string key, unsigned int val)
    {
        auto mh = this->uIntHandlers[key];
        mh.call(val);
        return true;
    }

    template <>
    inline bool LFAST::CommsService::callMessageHandler(std::string key, double val)
    {
        auto mh = this->doubleHandlers[key];
        mh.call(val);
        return true;
    }

    template <>
    inline bool LFAST::CommsService::callMessageHandler(std::string key, bool val)
    {
        auto mh = this->boolHandlers[key];
        mh.call(val);
        return true;
    }

    template <>
    inline bool LFAST::CommsService::callMessageHandler(std::string key, std::string val)
    {
        auto mh = this->stringHandlers[key];
        mh.call(val);
        return true;
    }

    template <>
    inline double CommsMessage::getValue(std::string key)
    {
        return (JsonDoc[key.c_str()].as<double>());
    }

    template <>
    inline int CommsMessage::getValue(std::string key)
    {
        return (JsonDoc[key.c_str()].as<int>());
    }

    template <>
    inline unsigned int CommsMessage::getValue(std::string key)
    {
        return (JsonDoc[key.c_str()].as<unsigned int>());
    }

    template <>
    inline bool CommsMessage::getValue(std::string key)
    {
        return (JsonDoc[key.c_str()].as<bool>());
    }

    template <>
    inline std::string CommsMessage::getValue(std::string key)
    {
        return (std::string(JsonDoc[key.c_str()].as<const char *>()));
    }

    inline void CommsMessage::addDestinationKey(std::string key)
    {

        // newDoc.createNestedObject(key);
        // newDoc[key] = JsonDoc.as<JsonObject>();
        destKey = key;

        if (!JsonDoc.isNull())
        {
            StaticJsonDocument<JSON_PROGMEM_SIZE> newDoc;

            for (JsonPairConst kvp : this->JsonDoc.as<JsonObjectConst>())
            {
                newDoc[key][kvp.key()] = kvp.value();
            }
            this->JsonDoc = newDoc;
        }
        else
        {
            JsonDoc.createNestedObject(destKey);
        }

        // = this->getJsonDoc();
        // JsonObject newObj = JsonDoc.
    }

    template <typename T>
    inline void CommsMessage::addKeyValuePair(std::string key, T val)
    {
        if (this->destKey.length() > 0)
            JsonDoc[this->destKey][key] = val;
        else
            JsonDoc[key] = val;
    };
}