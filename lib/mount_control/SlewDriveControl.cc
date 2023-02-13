#include <SlewDriveControl.h>

#include <TimerOne.h>
#include <map>
#include <string>

// #include <heartbeat.h>
#include <debug.h>
#include <device.h>
#include <KincoDriver.h>

std::map<std::string, SlewDriveControl *> SlewDriveControl::Drives;

SlewDriveControl::SlewDriveControl(std::string label)
    : DriveLabel(label)
{
    this->enabled = false;

    // Command the servos to disabled state
    // Configure servos to default state

    // Set up the control timer
    Drives[label] = this;
}

SlewDriveControl::SlewDriveControl(std::string label, int16_t _driveIdA, int16_t _driveIdB)
    : SlewDriveControl(label)
{
    initializeServoDrivers(_driveIdA, _driveIdA);
}

void SlewDriveControl::initializeServoDrivers(int16_t idA, int16_t idB) volatile
{
    driveIdA = idA;
    driveIdB = idB;

    ptrDriveA = new KincoDriver(driveIdA);
    ptrDriveB = new KincoDriver(driveIdB);
}

void SlewDriveControl::update_ISR()
{
    for (const auto &m : Drives)
    {
        SlewDriveControl *dcPtr = m.second;
        dcPtr->getCurrentFeedback();
        //  std::cout << n.first << " = " << n.second << "; ";
        // dcPtr->printLabel();
    }
    // toggleHeartbeatState();
}

void SlewDriveControl::configureLoopTimer(uint32_t prd)
{
    // DriveControl::timerPerioduS = prd;
    Timer1.initialize(prd);
    Timer1.attachInterrupt(SlewDriveControl::update_ISR); // blinkLED to run every 0.15 seconds
    // Timer1.stop();
}

void SlewDriveControl::startLoopTimer()
{
    Timer1.start();
}
void SlewDriveControl::printLabel()
{
    TEST_SERIAL.printf("%s\033[0K\r\n", this->DriveLabel.c_str());
}

void SlewDriveControl::enableDrive() volatile
{
    this->mode = VELOCITY;
    ptrDriveA->setControlMode(VELOCITY);
    // ptrDriveB->setControlMode(VELOCITY);
    ptrDriveB->setControlMode(TORQUE);
}
void SlewDriveControl::disableDrive() volatile
{
    this->mode = DISABLED;
    ptrDriveA->setControlMode(DISABLED);
    ptrDriveB->setControlMode(DISABLED);
}

void SlewDriveControl::setVelocityCommand(double cmdRPM) volatile
{
    ptrDriveA->updateVelocityCommand(cmdRPM);
    ptrDriveB->updateVelocityCommand(cmdRPM);
}