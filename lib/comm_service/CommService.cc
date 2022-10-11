
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

    std::stringstream ss;
    ss << "Unregistered Message." << std::endl;
    TEST_SERIAL.println(ss.str().c_str());
}

void LFAST::CommsService::errorMessageHandler(CommsMessage &msg)
{
    std::stringstream ss;
    ss << "Invalid Message.";
    //
    ss << std::endl;
    TEST_SERIAL.println(ss.str().c_str());
}

void LFAST::CommsMessage::parseReceivedData(char *rxBuff)
{
    // rxBuff->erase(std::remove_if(rxBuff->begin(), rxBuff->end(),
    //                              [](char c)
    //                              {
    //                                  return (c == '\r' || c == '\t' || c == ' ' || c == '\n');
    //                              }),
    //               rxBuff->end());
    auto error = deserializeJson(jsonDoc, rxBuff);
    // Test if parsing succeeds.
    if (error)
    {
        TEST_SERIAL.print(F("deserializeJson() failed: "));
        TEST_SERIAL.println(error.f_str());
        return;
    }
}

// void CommsMessage::printMessageInfo()
// {
//     TEST_SERIAL.printf("%d:\t", id);
//     for (uint16_t ii = 0; ii < args.size(); ii++)
//     {
//         TEST_SERIAL.printf("%4.2f:\t", args.at(ii));
//     }
// }

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

    for (auto &msg : this->messageQueue)
    {
        auto msgRoot = msg->jsonDoc.to<JsonObject>();
        for (JsonPair kvp : msgRoot)
            this->callMessageHandler(kvp);
        // {
            // TEST_SERIAL.println(kv.key().c_str());
            // TEST_SERIAL.println(kv.value().as<char *>());
            // auto keyStr = std::string(kv.key().c_str());
            // ArduinoJson6194_F1
            // auto fnPtr = messageHandlerList[keyStr];
        // }
        // root["city"] = "Paris";
    }

    // for (MessageHandlerList::iterator iter = messageHandlerList.begin(); iter != messageHandlerList.end(); ++iter)
    // {
    //     auto key = iter->first;
    //     // JsonVariant error = root["error"];
    //     if (!error.isNull())
    //     {
    //         Serial.println(error.as<char *>());
    //         return;
    //     }

    //     // ignore value
    //     // Value v = iter->second;
    // }
}