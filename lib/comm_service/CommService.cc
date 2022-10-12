
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
    // TEST_SERIAL.printf("\tBytes: %u\r\n", std::strlen("abc"));

    // TEST_SERIAL.print("\tBuffer Contents: ");
    // TEST_SERIAL.print(this->msgBuff);

    // for(int16_t ii = 0; ii < 100; ii++)
    // {
    //     TEST_SERIAL.print(this->jsonProgMem[ii]);
    // }
    TEST_SERIAL.printf("\tJSON: %s\r\n", this->jsonInputBuffer);
    // serializeJson(jsonDoc, TEST_SERIAL);
    // TEST_SERIAL.printf("\tBytes: %u\r\n", measureJson(jsonDoc));
    TEST_SERIAL.println("");
}

// std::string CommsMessage::getMessageStr()
// {
//     std::stringstream ss;
//     for (uint16_t ii = 0; ii < args.size(); ii++)
//     {
//         ss << this->args.at(ii) << ";";
//     }
//     ss << std::hex << this->id << std::endl;
//     return ss.str();
// }

void LFAST::CommsService::processReceived()
{
    if (this->messageQueue.size() > 0)
    {
        int count = 1;
        for (auto &msg : this->messageQueue)
        {
            TEST_SERIAL.printf("Processing Message #%d...\r\n", count++);

            StaticJsonDocument<JSON_PROGMEM_SIZE> jsonDoc;
            msg.printMessageInfo();
            auto error = deserializeJson(jsonDoc, msg.getBuffPtr());

            JsonObject msgRoot = jsonDoc.as<JsonObject>();
            JsonObject msgObject = msgRoot["MountMessage"];

            if (error)
            {
                TEST_SERIAL.print(F("deserializeJson() failed: "));
                TEST_SERIAL.println(error.f_str());
                return;
            }
            else
            {
                unsigned int tmp = msgObject["Handshake"];
                TEST_SERIAL.printf("Result: %u\r\n", tmp);
                for (JsonPair kvp : msgObject)
                {
                    auto keyStr = kvp.key().c_str();
                    TEST_SERIAL.printf("Processing Key: %s\r\n", keyStr);
                    this->callMessageHandler(kvp);
                }
            }
        }
        this->clearMessageQueue();
    }
}

void LFAST::CommsService::clearMessageQueue()
{
    // TEST_SERIAL.printf("Removing %d messages...\r\n", messageQueue.size());
    auto itr = messageQueue.begin();
    while (itr != messageQueue.end())
    {
        // delete *itr;
        itr = messageQueue.erase(itr);
    }
    // TEST_SERIAL.printf("%d messages remain.\r\n", messageQueue.size());
}