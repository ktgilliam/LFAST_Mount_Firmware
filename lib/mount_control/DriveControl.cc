#include <DriveControl.h>

#include <TimerOne.h>
#include <map>
#include <string>

// #include <heartbeat.h>
#include <debug.h>
#include <device.h>

std::map<std::string, DriveControl *> DriveControl::Drives;

DriveControl::DriveControl(std::string label)
    : DriveLabel(label)
{
    this->enabled = false;

    // Command the servos to disabled state
    // Configure servos to default state

    // Set up the control timer
    Drives[label] = this;
}

void DriveControl::update_ISR()
{
    for (const auto &m : Drives)
    {
        DriveControl *dcPtr =  m.second;
        //  std::cout << n.first << " = " << n.second << "; ";
        // dcPtr->printLabel();
    }
    // toggleHeartbeatState();
}

void DriveControl::configureLoopTimer(uint32_t prd)
{
    // DriveControl::timerPerioduS = prd;
    Timer1.initialize(prd);
    Timer1.attachInterrupt(DriveControl::update_ISR); // blinkLED to run every 0.15 seconds
    // Timer1.stop();
}

void DriveControl::startLoopTimer()
{
    Timer1.start();
}
void DriveControl::printLabel()
{
    TEST_SERIAL.printf("%s\033[0K\r\n", this->DriveLabel.c_str());
}