#pragma once

#include <cinttypes>
#include "KinkoNamespace.h"
#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>
#include <ServoInterface.h>

/* Temporary readability macro to avoid unused variables warnings */
#define KINCO_UNUSED(x) (void)x

namespace KINKO
{
    const int drive_i_peak = 36;
    const double current_conversion_factor = (2048.0 / drive_i_peak) / 1.414;
    const double velocity_conversion_factor =  512 * 10000.0 / 1875;
}
class KincoDriver : ServoInterface
{
private:
    static uint16_t numDrivers;

protected:
    int16_t driverNodeId;
    template <typename T>
    T readDriverRegister(uint16_t modBusAddr);
    template <typename T>
    uint16_t writeDriverRegister(uint16_t modBusAddr, T reg_value);

public:
    KincoDriver(int16_t driverId);
    virtual ~KincoDriver(){};

    void setDriverState(uint16_t) override;
    void getDriverState() override {};
    void setControlMode(uint16_t) override;
    void getControlMode() override {};

    void updateVelocityCommand(int32_t) override;
    void updateTorqueCommand(int32_t) override;

    int32_t getVelocityFeedback() override;
    int32_t getCurrentFeedback() override;
    int32_t getPositionFeedback() override;

    // void test_show_drive_struc();
};