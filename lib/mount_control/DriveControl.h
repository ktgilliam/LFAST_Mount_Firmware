#pragma once

#include <cinttypes>
#include <string>
// #include <vector>
#include <map>

namespace LFAST
{

    class DriveControl
    {
    public:
        DriveControl(std::string label);
        virtual ~DriveControl() {}

        void enableDrive() {}
        void disableDrive() {}
        void setCurrentCommand() {}
        void setVelocityCommand() {}
        void setPositionCommand() {}
        void getCurrentFeedback() {}
        void getVelocityFeedback() {}
        void getPositionFeedback() {}
        void printLabel();
        static void configureLoopTimer(uint32_t);
        static void startLoopTimer();
    protected:
        typedef enum
        {
            DISABLED = 0b000,
            POSITION = 0b001,
            VELOCITY = 0b010,
            CURRENT = 0b100
        } COMMAND_MODE;

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
        uint32_t *DriveControlStructA;
        uint32_t *DriveControlStructB;
    };

}