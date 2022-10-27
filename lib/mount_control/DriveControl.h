#pragma once

#include <cinttypes>
#include <string>
// #include <vector>
#include <map>

class DriveControl
{
public:
    typedef enum
    {
        DISABLED = 0b000,
        POSITION = 0b001,
        VELOCITY = 0b010,
        CURRENT = 0b100
    } COMMAND_MODE;

    DriveControl(std::string label);
    virtual ~DriveControl() {}

    void enableDrive() volatile {}
    void disableDrive() volatile {}
    void setControlMode(COMMAND_MODE _mode) volatile { mode = _mode; }
    void setCurrentCommand() volatile {}
    void setVelocityCommand() volatile {}
    void setPositionCommand() volatile {}
    void getCurrentFeedback() volatile {}
    void getVelocityFeedback() volatile {}
    void getPositionFeedback() volatile {}
    void printLabel();
    static void configureLoopTimer(uint32_t);
    static void startLoopTimer();

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
    static std::map<std::string, DriveControl *> Drives;
    uint8_t DriveIndex;

private:
    // uint32_t *DriveControlStructA;
    // uint32_t *DriveControlStructB;
};