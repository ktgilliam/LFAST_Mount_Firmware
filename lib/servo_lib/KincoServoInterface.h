#pragma once

#include <cinttypes>
#include "KinkoNamespace.h"
#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>
#include <ServoInterface.h>

class KincoServoInterface : ServoInterface
{
private:
    static uint16_t numDrivers;

protected:
public:
    int driverNodeId;

    template <typename T>
    T readDriverRegister(uint16_t modBusAddr);
    template <typename T>
    uint16_t writeDriverRegister(uint16_t modBusAddr, T reg_value);
    // void updateDriverControlRegisters();

    void setDriverState(uint16_t) override;
    void getDriverState() override;
    void setControlMode(uint16_t) override;
    void getControlMode() override;
    
    void updateVelocityCommand(int32_t) override;
    void updateTorqueCommand(int32_t) override;

    int32_t getVelocityFeedback() override;
    int32_t getCurrentFeedback() override;
    int32_t getPositionFeedback() override;

    // void test_show_drive_struc();
};