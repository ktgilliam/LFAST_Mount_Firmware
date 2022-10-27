#pragma once

#include <TerminalInterface.h>

class ServoInterface
{
protected:
    TerminalInterface *cli = nullptr;

public:
    void connectTerminalInterface(TerminalInterface *_cli);

    virtual void setDriverState(uint16_t) = 0;
    virtual void getDriverState() = 0;
    virtual void setControlMode(uint16_t) = 0;
    virtual void getControlMode() = 0;

    virtual void updateVelocityCommand(int32_t);
    virtual void updateTorqueCommand(int32_t);

    virtual int32_t getVelocityFeedback();
    virtual int32_t getCurrentFeedback();
    virtual int32_t getPositionFeedback();
};