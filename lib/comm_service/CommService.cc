
#include "CommService.h"

#include <array>
#include <sstream>
#include <iterator>
#include <cstring>
#include <string>
#include <cstdlib>
// #include <StreamUtils.h>
#include <algorithm>
// #include <string_view>

#include <debug.h>
#include <device.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
LFAST::CommsService::CommsService()
{

    // for (uint16_t ii = 0; ii < MAX_CTRL_MESSAGES; ii++)
    // {
    //     this->registerMessageHandler(ii, CommsService::defaultMessageHandler);
    // }
    // this->messageHandlerList["Default"] = CommsService::defaultMessageHandler;
}

// void LFAST::CommsService::registerMessageHandler(std::string key, MsgHandlerFn fn)
// {
//     this->messageHandlerList[key] = fn;
// }

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

void LFAST::CommsMessage::loadReceivedData(char *rxBuff, unsigned int numBytes)
{
    // rxBuff->erase(std::remove_if(rxBuff->begin(), rxBuff->end(),
    //                              [](char c)
    //                              {
    //                                  return (c == '\r' || c == '\t' || c == ' ' || c == '\n');
    //                              }),
    //               rxBuff->end());
    // std::memset(this->jsonProgMem, 0, JSON_PROGMEM_SIZE);
    // TEST_SERIAL.printf("Bytes: %d\r\n", numBytes);
    // TEST_SERIAL.println(rxBuff);
    memcpy(this->jsonInputBuffer, rxBuff, numBytes + 1);

    // TEST_SERIAL.println(this->jsonInputBuffer);
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
// void LFAST::CommsMessage::printMessageInfo() {}

void LFAST::CommsMessage::printMessageInfo()
{

    TEST_SERIAL.println("MESSAGE INFO:");
    TEST_SERIAL.printf("\tID: %u\r\n", (unsigned int)this->getBuffPtr());
    TEST_SERIAL.printf("\tJSON: %s\r\n", this->jsonInputBuffer);
    // serializeJson(jsonDoc, TEST_SERIAL);
    // TEST_SERIAL.printf("\tBytes: %u\r\n", measureJson(jsonDoc));
    TEST_SERIAL.println("");
}

void LFAST::CommsService::processReceived(CommsMessage &msg)
{

    // StaticJsonDocument<JSON_PROGMEM_SIZE> jsonDoc;
    // msg.printMessageInfo();
    auto error = deserializeJson(msg.rxJsonDoc, msg.getBuffPtr());
    JsonObject msgRoot = msg.rxJsonDoc.as<JsonObject>();
    JsonObject msgObject = msgRoot["MountMessage"];
    // Test if parsing succeeds.
    if (error)
    {
        TEST_SERIAL.print(F("deserializeJson() failed: "));
        TEST_SERIAL.println(error.f_str());
        return;
    }
    else
    {
        for (JsonPair kvp : msgObject)
        {
            auto keyStr = kvp.key().c_str();
            TEST_SERIAL.printf("Processing Key: %s\r\n", keyStr);
            this->callMessageHandler(kvp);
        }
    }
}

bool LFAST::CommsService::processNewMessages(Client &client)
{
    // listen for incoming clients
    if (client)
    {
        uint8_t readBuff[RX_BUFF_SIZE];
        memset(readBuff, 0, RX_BUFF_SIZE);
        unsigned int bytesRead = 0;
        while (client.connected())
        {
            // TEST_SERIAL.println("Checking connected client messages");
            if (client.available())
            {
                char c = client.read();
                // TEST_SERIAL.print(c);
                // TEST_SERIAL.printf("%u ", (unsigned int) c);
                // if (c == '\0') TEST_SERIAL.printf("%u ", (unsigned int) c);
                
                if ((c == '\0') || (bytesRead >= RX_BUFF_SIZE))
                {
                    // NetCommsMessage newMsg;
                    // TEST_SERIAL.printf("\nReceived Data: [%s]\r\n", (char *)readBuff);
                    // auto newMsg = new NetCommsMessage();
                    CommsMessage newMsg;
                    newMsg.loadReceivedData((char *)readBuff, bytesRead);

                    // TEST_SERIAL.println("Parsing done.");
                    this->processReceived(newMsg);
                    // this->rxMessageQueue.push_back(newMsg);
                    break;
                }
                else
                {
                    readBuff[bytesRead++] = c;
                }
            }
        }
    }
    return true;
}

bool LFAST::CommsService::checkForNewClients()
{
    bool newClientFlag = false;
    // TODO
    return (newClientFlag);
}

void LFAST::CommsService::stopDisconnectedClients()
{
    auto itr = clients.begin();
    while (itr != clients.end())
    {
        if (!(*itr)->connected())
        {
            // TEST_SERIAL.println("Disconnected client.");
            // TEST_SERIAL.println(ii);
            (*itr)->stop();
            itr = clients.erase(itr);
        }
        else
        {
            itr++;
        }
    }


}