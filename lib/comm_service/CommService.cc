
#include "CommService.h"

#include <array>
#include <sstream>
#include <iterator>

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

void CommsService::processReceived()
{
    std::vector<CommsMessage>::iterator itr;
    for (itr = messageQueue.begin(); itr != messageQueue.end();)
    {
        CommsMessage msg = (*itr);

        uint32_t id = msg.id;
        MsgHandlerFn handlerFn = defaultMessageHandler;

        TOGGLE_DEBUG_PIN();
        if (id <= MAX_CTRL_MESSAGES)
        {
            handlerFn = messageHandlerList[id];
        }
        handlerFn(msg);
        messageQueue.erase(itr);
    }
}
