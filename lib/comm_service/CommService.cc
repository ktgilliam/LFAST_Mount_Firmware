
#include "CommService.h"

#include <array>
#include <sstream>
#include <iterator>
#include <cstring>
#include <string>
#include <cstdlib>
#include <StreamUtils.h>
#include <algorithm>
// #include <string_view>

#include <debug.h>
#include <device.h>

std::vector<LFAST::ClientConnection> LFAST::CommsService::connections{};
///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
LFAST::CommsService::CommsService()
{
    activeConnection = nullptr;
    // for (uint16_t ii = 0; ii < MAX_CTRL_MESSAGES; ii++)
    // {
    //     this->registerMessageHandler(ii, CommsService::defaultMessageHandler);
    // }
    // this->messageHandlerList["Default"] = CommsService::defaultMessageHandler;
}

void LFAST::CommsService::setupClientMessageBuffers(Client *client)
{
    // ClientConnection is created on the stack
    ClientConnection newConnection(client);
    this->connections.push_back(newConnection);
}

void LFAST::CommsService::defaultMessageHandler(std::string info)
{
    TEST_SERIAL.printf("Unregistered Message: [%s].\r\n", info.c_str());
}

void LFAST::CommsService::errorMessageHandler(CommsMessage &msg)
{
    std::stringstream ss;
    ss << "Invalid Message.";
    //
    ss << std::endl;
    TEST_SERIAL.println(ss.str().c_str());
}

bool LFAST::CommsService::checkForNewClients()
{
    bool newClientFlag = false;
    // TODO
    return (newClientFlag);
}

// void LFAST::CommsMessage::printMessageInfo() {}
void LFAST::CommsService::checkForNewClientData()
{
    // check for incoming data from all clients
    for (auto &connection : this->connections)
    {
        if (connection.client->available())
        {
            getNewMessages(connection);
        }
    }
}

bool LFAST::CommsService::getNewMessages(ClientConnection &connection)
{
    //
    // listen for incoming clients
    Client *client = connection.client;
    if (client)
    {
        CommsMessage newMsg;
        unsigned int bytesRead = 0;
        while (client->connected())
        {
            // TEST_SERIAL.println("Checking connected client messages");
            if (client->available())
            {
                char c = client->read();

                if ((c == '\0') || (bytesRead >= RX_BUFF_SIZE))
                {
                    connection.rxMessageQueue.push_back(newMsg);
                    break;
                }
                else
                {
                    // TEST_SERIAL.print(c);
                    newMsg.jsonInputBuffer[bytesRead++] = c;
                }
            }
        }
        TEST_SERIAL.println("");
    }
    return true;
}

void LFAST::CommsMessage::printMessageInfo()
{

    TEST_SERIAL.println("MESSAGE INFO:");
    TEST_SERIAL.printf("\tID: %u\r\n", (unsigned int)this->getBuffPtr());
    TEST_SERIAL.printf("\tJSON: %s\r\n", this->jsonInputBuffer);
    // serializeJson(jsonDoc, TEST_SERIAL);
    // TEST_SERIAL.printf("\tBytes: %u\r\n", measureJson(jsonDoc));
    TEST_SERIAL.println("");
}

void LFAST::CommsService::processClientData()
{
    for (auto &conn : this->connections)
    {
        this->activeConnection = &conn;
        auto itr = conn.rxMessageQueue.begin();
        while (itr != conn.rxMessageQueue.end())
        {
            processMessage(*itr);
            itr = conn.rxMessageQueue.erase(itr);
        }
    }
    this->activeConnection = nullptr;
}
void LFAST::CommsService::processMessage(CommsMessage &msg)
{
    StaticJsonDocument<JSON_PROGMEM_SIZE> &doc = msg.deserialize();
    JsonObject msgRoot = doc.as<JsonObject>();
    JsonObject msgObject = msgRoot["MountMessage"];
    // Test if parsing succeeds.

    for (JsonPair kvp : msgObject)
    {
        // auto keyStr = kvp.key().c_str();
        // TEST_SERIAL.printf("Processing Key: %s\r\n", keyStr);
        this->callMessageHandler(kvp);
    }
}

bool LFAST::CommsService::callMessageHandler(JsonPair kvp)
{
    bool handlerFound = true;
    auto keyStr = std::string(kvp.key().c_str());
    if (this->handlerTypes.find(keyStr) == this->handlerTypes.end())
    {
        handlerFound = false;
        defaultMessageHandler(keyStr);
    }
    else
    {
        auto handlerType = this->handlerTypes[keyStr];

        switch (handlerType)
        {
        case INT_HANDLER:
        {
            auto val = kvp.value().as<int>();
            this->callMessageHandler<int>(keyStr, val);
        }
        break;
        case UINT_HANDLER:
        {
            auto val = kvp.value().as<unsigned int>();
            this->callMessageHandler<unsigned int>(keyStr, val);
        }
        break;
        case DOUBLE_HANDLER:
        {
            auto val = kvp.value().as<double>();
            this->callMessageHandler<double>(keyStr, val);
        }
        break;
        case BOOL_HANDLER:
        {
            auto val = kvp.value().as<bool>();
            this->callMessageHandler<bool>(keyStr, val);
        }
        break;
        case STRING_HANDLER:
        {
            auto val = kvp.value().as<std::string>();
            this->callMessageHandler<std::string>(keyStr, val);
        }
        break;
        default:
            handlerFound = false;
        }
    }

    return handlerFound;
}

void LFAST::CommsService::sendMessage(CommsMessage &msg, uint8_t sendOpt)
{
    if (sendOpt == ACTIVE_CONNECTION)
    {
        WriteBufferingStream bufferedClient(*(activeConnection->client), std::strlen(msg.getBuffPtr()));
        serializeJson(msg.getJsonDoc(), bufferedClient);
        bufferedClient.flush();
        activeConnection->client->write('\0');
    }
    else
    {
        // TODO
    }
}

StaticJsonDocument<JSON_PROGMEM_SIZE> &LFAST::CommsMessage::deserialize()
{
    DeserializationError error = deserializeJson(this->JsonDoc, this->jsonInputBuffer);
    if (error)
    {
        TEST_SERIAL.print(F("deserializeJson() failed: "));
        TEST_SERIAL.println(error.f_str());
    }
    return this->JsonDoc;
}
void LFAST::CommsService::stopDisconnectedClients()
{
    auto itr = connections.begin();
    while (itr != connections.end())
    {
        if (!(*itr).client->connected())
        {
            // TEST_SERIAL.println("Disconnected client.");
            (*itr).client->stop();
            itr = connections.erase(itr);
        }
        else
        {
            itr++;
        }
    }
}
