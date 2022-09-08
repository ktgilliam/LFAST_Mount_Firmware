#include <fakeMount.h>
#include <Arduino.h>
#include <CanComms.h>
#include <device.h>
#include <appTypes.h>
#include <debug.h>
#include <math.h>
#include <string>

#define FAKE_MOUNT_UPDATE_PRD 10000
IntervalTimer mountUpdateTimer;

void updateMountStates();
void sendAzPosition(char *guts, int len);
void sendDumbyMessage();

double currentAzimuthPosition;
double currentElevationPosition;

#define AZ_VELOCITY_AMPLITUDE 1.0
#define EL_VELOCITY_AMPLITUDE 0.0
double azVelocity;
double elVelocity;

double currentTime;
double deltaTime;

void enableFakeMount()
{
    currentAzimuthPosition = 0.0;
    currentElevationPosition = 0.0;
    currentTime = 0.0;
    deltaTime = FAKE_MOUNT_UPDATE_PRD / 1000000;
    mountUpdateTimer.begin(updateMountStates, FAKE_MOUNT_UPDATE_PRD);
    registerCanMessageHandler(CTRL_PC_ID_MASK | AZ_POSN_REQ, sendAzPosition);
    setRxMailboxFilterRange(CTRL_PC_FILT_BEGIN, CTRL_PC_FILT_END);
}

void updateMountStates()
{
    // TOGGLE_DEBUG_PIN();
    currentTime += deltaTime;
    double currentAzVelocity = AZ_VELOCITY_AMPLITUDE;
    double currentElVelocity = EL_VELOCITY_AMPLITUDE;
    // double currentAzVelocity = AZ_VELOCITY_AMPLITUDE * sin(currentTime);
    // double currentElVelocity = EL_VELOCITY_AMPLITUDE * sin(currentTime);

    currentAzimuthPosition += deltaTime * currentAzVelocity;
    currentElevationPosition += deltaTime * currentElVelocity;
    // TOGGLE_DEBUG_PIN();
    // sendDumbyMessage();
}

void sendAzPosition(char *guts, int len)
{
    TOGGLE_DEBUG_PIN();
    TEST_SERIAL.println("Az Request Received.");
    CAN_message_t msg;
    Double2Char_t payload;
    payload.dblVal = currentAzimuthPosition;
    msg.id = PDSTL_CTRL_ID_MASK | AZ_POSN_RSP;
    // msg.flags =
    // {.remote = 0, .extended = 0}
    msg.len = sizeof(currentAzimuthPosition);
    memcpy(msg.buf, payload.charVals, msg.len);
    sendCanBusMessage(msg); // write to can
    TOGGLE_DEBUG_PIN();
}

void sendDumbyMessage()
{
    CAN_message_t msg;
    msg.id = PDSTL_CTRL_ID_MASK | AZ_POSN_REQ;
    std::string sendStr = "DUMB.";
    memcpy(msg.buf, sendStr.c_str(), sendStr.length());
    sendCanBusMessage(msg); // write to can
}