
#include "CommService.h"

#include <array>
#include <sstream>
#include <iterator>
#include <cstring>
#include <string>
#include <cstdlib>

#include <debug.h>
#include <device.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
CommsService::CommsService()
{
    for (uint16_t ii = 0; ii < MAX_CTRL_MESSAGES; ii++)
    {
        this->registerMessageHandler(ii, CommsService::defaultMessageHandler);
    }
}

void CommsService::registerMessageHandler(uint16_t msgId, MsgHandlerFn fn)
{
    this->messageHandlerList[msgId] = fn;
}

void CommsService::defaultMessageHandler(CommsMessage &dontCare)
{

    std::stringstream ss;
    ss << "Unregistered Message ID." << std::endl;
    TEST_SERIAL.println(ss.str().c_str());
}

void CommsService::errorMessageHandler(CommsMessage &msg)
{

    std::stringstream ss;
    ss << "Invalid Message ID. Payload: ";

    ss << std::endl;
    TEST_SERIAL.println(ss.str().c_str());
}


void CommsMessage::parseReceivedData(char *rxBuff)
{
    std::vector<std::string> argStrs;
    char *token = std::strtok(rxBuff, "#;");

    while (token != NULL)
    {
        argStrs.push_back(std::string(token));
        token = strtok(NULL, "#;");
    }

    this->id = std::atoi(argStrs.back().c_str());
    argStrs.pop_back();

    for (uint16_t ii = 0; ii < argStrs.size(); ii++)
    {
        double argDbl = std::atof(argStrs.at(ii).c_str());
        this->args.push_back(argDbl);
    }
}

    void CommsMessage::printMessageInfo()
    {
        TEST_SERIAL.printf("%d:\t", id);
        for (uint16_t ii = 0; ii < args.size(); ii++)
        {
            TEST_SERIAL.printf("%4.2f:\t", args.at(ii));
        }
    }
    std::string CommsMessage::getMessageStr()
    {
        std::stringstream ss;
        for (uint16_t ii = 0; ii < args.size(); ii++)
        {
            ss << this->args.at(ii) << ";";
        }
        ss << std::hex << this->id << std::endl;
        return ss.str();
    }