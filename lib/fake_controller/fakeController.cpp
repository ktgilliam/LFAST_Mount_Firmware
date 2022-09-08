#include <fakeController.h>
#include <CanComms.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <appTypes.h>
#include <device.h>

IntervalTimer controllerSendTimer;
#define TEST_MODE_MSG_PERIOD_US 100000

void requestAzPosition();
void receivedAzPosition(char *guts, int len);

void enableFakeController()
{
    controllerSendTimer.begin(requestAzPosition, TEST_MODE_MSG_PERIOD_US);
    registerCanMessageHandler(PDSTL_CTRL_ID_MASK | AZ_POSN_RSP, receivedAzPosition);
    setRxMailboxFilterRange(PDSTL_PC_FILT_BEGIN, PDSTL_PC_FILT_END);
}

void disableFakeController()
{
    controllerSendTimer.end();
}
void requestAzPosition()
{
    CAN_message_t msg;
    msg.id = CTRL_PC_ID_MASK | AZ_POSN_REQ;
    msg.flags.remote = 1;
    sendCanBusMessage(msg); // write to can
}

void receivedAzPosition(char *guts, int len)
{
    Double2Char_t payload;
    memcpy(payload.charVals, guts, len);
    std::stringstream ss;
    ss << "Az Response Received.: " << std::setprecision(2) << payload.dblVal;
    TEST_SERIAL.println(ss.str().c_str());

}