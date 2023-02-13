#pragma once

#include <cinttypes>
#include <string>
// #include <vector>
#include <map>
#include <KincoDriver.h>

class SlewDriveControl
{
public:
    typedef enum
    {
        DISABLED = 0b000,
        POSITION = 0b001,
        VELOCITY = 0b010,
        TORQUE = 0b100
    } COMMAND_MODE;

    SlewDriveControl(std::string label);
    SlewDriveControl(std::string label, int16_t _driveIdA, int16_t _driveIdB);

    virtual ~SlewDriveControl() {}

    void enableDrive() volatile;
    void disableDrive() volatile;
    void setControlMode(COMMAND_MODE _mode) volatile { mode = _mode; }
    void setCurrentCommand() volatile {}
    void setVelocityCommand(double cmdRPM) volatile;
    void setPositionCommand() volatile {}
    void getCurrentFeedback() volatile {}
    void getVelocityFeedback() volatile {}
    void getPositionFeedback() volatile {}
    void printLabel();

    static void configureLoopTimer(uint32_t);
    static void startLoopTimer();
    void initializeServoDrivers(int16_t idA, int16_t idB) volatile;
protected:
    std::string DriveLabel;
    bool enabled;
    COMMAND_MODE mode;
    double CurrentCommand;
    double VelocityCommand;
    double PositionCommand;
    double CurrentFeedback;
    double VelocityFeedback;
    double PositionFeedback;
    static void update_ISR();

    int16_t driveIdA;
    int16_t driveIdB;

    // void initializeServoDriver(int16_t driveId) volatile;
    static std::map<std::string, SlewDriveControl *> Drives;
    // uint8_t DriveIndex;
    KincoDriver *ptrDriveA;
    KincoDriver *ptrDriveB;

private:
    // uint32_t *DriveControlStructA;
    // uint32_t *DriveControlStructB;
};