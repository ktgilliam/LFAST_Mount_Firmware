#include <MountControl.h>
// LFAST::MountControl *mcInstance = 0;

#include <cmath>
#include <TimerThree.h>
#include <iomanip>

#include <DriveControl.h>
#include <NetComms.h>
#include <mathFuncs.h>
#include <device.h>
#include <debug.h>
#include <stdio.h>

const std::string SLEW_COMPLETE_MSG_STR = "Slew is complete.";
const std::string GOTO_WHILE_PARKED_MSG_STR = "Received Goto command while parked.";
const std::string ABORT_COMMAND_MSG_STR = "Received Abort Command.";

LFAST::MountControl::MountControl()
    : AltDriveControl("ALT"), AzDriveControl("AZ")
{
#if SIM_SCOPE_ENABLED
    initSimMount();
#endif
    DriveControl::configureLoopTimer(DEFAULT_SERVO_PRD);
    DriveControl::startLoopTimer();
    initializeCLI();
    readyFlag = false;
}

LFAST::MountControl &LFAST::MountControl::getMountController()
{
    static MountControl instance;
    return instance;
}

void LFAST::MountControl::setUpdatePeriod(uint32_t prd)
{
    updatePrdUs = prd;
    Timer3.stop();
    Timer3.initialize(updatePrdUs);
    Timer3.attachInterrupt(LFAST::updateMountControl_ISR);
    Timer3.start();
}

void LFAST::MountControl::updateClock(double lst)
{
    double dt = lst - localSiderealTime;
    if (dt > 0.0)
    {
        deltaTimeSec = dt * 3600.0;
        localSiderealTime = lst;
    }
}

void LFAST::MountControl::initializeCLI()
{
    cli.printMountStatusLabels();
}
void LFAST::MountControl::printMountStatus()
{
    cli.updateStatusFields(*this);
    // TEST_SERIAL.printf("\033[32m");
}

void LFAST::MountControl::findHome()
{
    mountCmdEvents.push(HOME_COMMAND_RECEIVED);
#if SIM_SCOPE_ENABLED
    // targetAzPosn = 0.0;
    // targetAltPosn = 0.0;
#else
#warning HOMING NOT IMPLEMENTED
#endif
}

void LFAST::MountControl::park()
{
    std::string msg = "Received park Command.";
    cli.addDebugMessage(msg);
    mountCmdEvents.push(PARK_COMMAND_RECEIVED);
}

void LFAST::MountControl::unpark()
{
    std::string msg = "Received unpark Command.";
    cli.addDebugMessage(msg);
    mountCmdEvents.push(UNPARK_COMMAND_RECEIVED);
}

void LFAST::MountControl::getPosnErrors(double *altErr, double *azErr)
{
    double altErrorTmp = altPosnCmd_rad - altPosnFb_rad;
    double azErrorTmp = azPosnCmd_rad - azPosnFb_rad;

    while (std::abs(azErrorTmp) > M_PI)
        azErrorTmp -= 2 * M_PI * sign(azErrorTmp);

    *altErr = altErrorTmp;
    *azErr = azErrorTmp;
}

void LFAST::MountControl::getTrackingRateCommands(double *dAlt, double *dAz)
{
    getPosnErrors(&(this->AltPosnErr), &(this->AzPosnErr));

#if defined(POSITION_CONTROL_ONLY)
    *dAlt = TRACK_RATE_RPS * sign(AltPosnErr);
    *dAz = TRACK_RATE_RPS * sign(AzPosnErr);
#else
    // NOTE: THIS IS WRONG!!!
    // https://safe.nrao.edu/wiki/pub/Main/RadioTutorial/AzEltoSidereal.pdf
    // double ha_rad = ha2rad(localSiderealTime - targetRaPosn);
    *dAlt = SIDEREAL_RATE_RPS * sin(altPosnCmd_rad) + cos(deg2rad(localLatitude)) * sin(azPosnCmd_rad);
    *dAz = SIDEREAL_RATE_RPS * cos(targetDecPosn) * tan(altPosnCmd_rad);
#endif
}

bool LFAST::MountControl::getSlewingRateCommands(double *dAlt, double *dAz)
{
    getPosnErrors(&(this->AltPosnErr), &(this->AzPosnErr));

    double altRate = getAxisSlewRateCommand(this->AltPosnErr);
    double azRate = getAxisSlewRateCommand(this->AzPosnErr);

    bool slewCompleteFlag = (std::abs(azRate) == 0.0) && (std::abs(altRate) == 0.0);

    *dAlt = altRate;
    *dAz = azRate;

    return slewCompleteFlag;
}

double LFAST::MountControl::getAxisSlewRateCommand(double axErr)
{
    double axisRateCmd;
    if (std::abs(axErr) <= TRACK_ERR_THRESH)
        axisRateCmd = 0.0;
    else if ((std::abs(axErr) > TRACK_ERR_THRESH) && (std::abs(axErr) < FAST_SLEW_THRESH))
        axisRateCmd = END_SLEW_RATE_RPS * sign(axErr);
    else
        axisRateCmd = MAX_SLEW_RATE_RPS * sign(axErr);
    return axisRateCmd;
}

double LFAST::MountControl::getParallacticAngle()
{
    double h = localSiderealTime; // hour angle
    // double a = ha2rad(h - targetRaPosn); // Target right ascension
    double d = deg2rad(targetDecPosn); // Target declination
    double p = deg2rad(localLatitude); // Latitude
    double q = atan2(sin(h), (cos(d) * tan(p) - sin(d) * cos(h)));
    return q;
}

void LFAST::MountControl::raDecToAltAz(double ra, double dec, double *alt, double *az)
{
    double ha_rad = ha2rad(localSiderealTime - ra);
    double dec_rad = deg2rad(dec);

    if (ha_rad < 0)
    {
        ha_rad += (2 * M_PI);
    }
    if (ha_rad > M_PI)
    {
        ha_rad = ha_rad - (2 * M_PI);
    }

    double lat_rad = deg2rad(localLatitude);

    double AzTmp = atan2(sin(ha_rad),
                         cos(ha_rad) * sin(lat_rad) - tan(dec_rad) * cos(lat_rad)) -
                   M_PI;
    AzTmp = AzTmp >= 0 ? AzTmp : (AzTmp + 2 * M_PI);

    double AltTmp = asin(sin(lat_rad) * sin(dec_rad) + cos(lat_rad) * cos(dec_rad) * cos(ha_rad));

    *alt = AltTmp;
    *az = AzTmp;
}

void LFAST::MountControl::updateTargetRaDec(double ra, double dec)
{
    targetRaPosn = ra;
    targetDecPosn = dec;
    char dbgMsg[50];
    sprintf(dbgMsg, "Received Goto Command: [%6.4f ; %6.4f]", ra, dec);
    cli.addDebugMessage(std::string(dbgMsg), MountControl_CLI::WARNING);

    mountCmdEvents.push(GOTO_COMMAND_RECEIVED);
}

void LFAST::MountControl::syncRaDec(double ra, double dec)
{
    double newAlt, newAz;
    raDecToAltAz(ra, dec, &newAlt, &newAz);
    altPosnFb_rad = newAlt;
    azPosnFb_rad = newAz;
}

double LFAST::MountControl::getTrackRate()
{
    return std::sqrt(azRateCmd_rps * azRateCmd_rps + altRateCmd_rps * altRateCmd_rps) / 3200.0;
}

void LFAST::MountControl::abortSlew()
{
    cli.addDebugMessage(ABORT_COMMAND_MSG_STR, MountControl_CLI::WARNING);
    altPosnCmd_rad = altPosnFb_rad;
    azPosnCmd_rad = azPosnFb_rad;
    azRateCmd_rps = 0.0;
    altRateCmd_rps = 0.0;
    mountCmdEvents.push(ABORT_COMMAND_RECEIVED);
}

void LFAST::MountControl::getCurrentRaDec(double *ra, double *dec)
{
    double haTmp;
    altAzToHADec(altPosnFb_rad, azPosnFb_rad, &haTmp, dec);

    if (ra)
        *ra = localSiderealTime - haTmp;
}

void LFAST::MountControl::altAzToHADec(double alt, double az, double *ha, double *dec)
{
    double lat_rad = deg2rad(localLatitude);
    double haTmp = atan2(-sin(az), tan(alt) * cos(lat_rad) - cos(az) * sin(lat_rad));

    if (ha)
        *ha = rad2ha(haTmp);

    double decTmp = asin(sin(lat_rad) * sin(alt) + cos(lat_rad) * cos(alt) * cos(az));

    if (dec)
        *dec = rad2deg(decTmp);
    return;
}

void LFAST::MountControl::initSimMount()
{
    mountStatus = MOUNT_PARKED;
    altPosnFb_rad = altParkPosn;
    azPosnFb_rad = azParkPosn;
}

void LFAST::updateMountControl_ISR()
{
    MountControl &mountControl = LFAST::MountControl::getMountController();
    switch (mountControl.mountStatus)
    {
    case MountControl::MOUNT_IDLE:
        mountControl.mountStatus = mountControl.mountIdleHandler();
        break;
    case MountControl::MOUNT_PARKING:
        mountControl.mountStatus = mountControl.mountParkingHandler();
        break;
    case MountControl::MOUNT_PARKED:
        mountControl.mountStatus = mountControl.mountParkedHandler();
        break;
    case MountControl::MOUNT_HOMING:
        mountControl.mountStatus = mountControl.mountHomingHandler();
        break;
    case MountControl::MOUNT_SLEWING:
        mountControl.mountStatus = mountControl.mountSlewingHandler();
        break;
    case MountControl::MOUNT_TRACKING:
        mountControl.mountStatus = mountControl.mountTrackingHandler();
        break;
    case MountControl::MOUNT_ERROR:
    default:
        mountControl.mountStatus = mountControl.mountErrorHandler();
        break;
    }
    mountControl.updateSimMount();
}

LFAST::MountControl::MountCommandEvent
LFAST::MountControl::readEvent()
{
    MountCommandEvent event;
    if (!mountCmdEvents.empty())
    {
        event = mountCmdEvents.front();
        mountCmdEvents.pop();
    }
    else
    {
        event = MountCommandEvent::NO_COMMANDS_RECEIVED;
    }
    return event;
}

LFAST::MountControl::MountStatus
LFAST::MountControl::mountIdleHandler()
{
    MountCommandEvent cmdEvent = readEvent();
    MountStatus nextStatus = MOUNT_IDLE;
    switch (cmdEvent)
    {
    case NO_COMMANDS_RECEIVED:
        nextStatus = MOUNT_IDLE;
        break;
    case PARK_COMMAND_RECEIVED:
        nextStatus = MOUNT_PARKING;
        break;
    case UNPARK_COMMAND_RECEIVED:
        nextStatus = MOUNT_IDLE;
        break;
    case GOTO_COMMAND_RECEIVED:
        nextStatus = MOUNT_SLEWING;
        break;
    case HOME_COMMAND_RECEIVED:
        nextStatus = MOUNT_HOMING;
        break;
    case ABORT_COMMAND_RECEIVED:
        targetDecPosn = 0.0;
        targetRaPosn = 0.0;
        nextStatus = MOUNT_IDLE;
        break;
    }
    altRateCmd_rps = 0.0;
    azRateCmd_rps = 0.0;
    return nextStatus;
}

LFAST::MountControl::MountStatus
LFAST::MountControl::mountParkingHandler()
{
    MountCommandEvent cmdEvent = readEvent();
    if (cmdEvent == ABORT_COMMAND_RECEIVED)
        return MOUNT_IDLE;

    altPosnCmd_rad = altParkPosn;
    azPosnCmd_rad = azParkPosn;
    targetDecPosn = 0.0;
    targetRaPosn = 0.0;
    bool parkingComplete = getSlewingRateCommands(&altRateCmd_rps, &azRateCmd_rps);
    if (parkingComplete)
    {
        return MOUNT_PARKED;
    }
    else
    {

        return MOUNT_PARKING;
    }
}

LFAST::MountControl::MountStatus
LFAST::MountControl::mountParkedHandler()
{
    altRateCmd_rps = 0.0;
    azRateCmd_rps = 0.0;
    MountCommandEvent cmdEvent = readEvent();
    if (cmdEvent == UNPARK_COMMAND_RECEIVED)
        return MOUNT_IDLE;
    else
        return MOUNT_PARKED;
}

LFAST::MountControl::MountStatus
LFAST::MountControl::mountHomingHandler()
{
    MountCommandEvent cmdEvent = readEvent();
    if (cmdEvent == ABORT_COMMAND_RECEIVED)
        return MOUNT_IDLE;

    altPosnCmd_rad = 0.0;
    azPosnCmd_rad = 0.0;
    AltDriveControl.setControlMode(DriveControl::VELOCITY);
    altRateCmd_rps = -1 * MAX_SLEW_RATE_RPS;
    azRateCmd_rps = -1 * MAX_SLEW_RATE_RPS;
    // TODO: upon finding limit switch, back up, slow down, and bump it one more time.
    // PLACEHOLDER CODE:
    if ((azPosnFb_rad == 0.0) && (altPosnFb_rad == 0.0))
    {
        altRateCmd_rps = 0.0;
        azRateCmd_rps = 0.0;
        return MOUNT_IDLE;
    }
    else
    {
        return MOUNT_HOMING;
    }
}

LFAST::MountControl::MountStatus
LFAST::MountControl::mountSlewingHandler()
{
    MountCommandEvent cmdEvent = readEvent();
    if (cmdEvent == ABORT_COMMAND_RECEIVED)
        return MOUNT_IDLE;
    else if (cmdEvent == PARK_COMMAND_RECEIVED)
        return MOUNT_PARKING;
    else if (cmdEvent == HOME_COMMAND_RECEIVED)
        return MOUNT_HOMING;

    raDecToAltAz(targetRaPosn, targetDecPosn, &altPosnCmd_rad, &azPosnCmd_rad);

    bool slewComplete = getSlewingRateCommands(&altRateCmd_rps, &azRateCmd_rps);
    if (slewComplete)
    {
        cli.addDebugMessage(SLEW_COMPLETE_MSG_STR, MountControl_CLI::INFO);
        slewCompleteFlag = true;
        getTrackingRateCommands(&altRateCmd_rps, &azRateCmd_rps);
        return MOUNT_TRACKING;
    }
    else
    {
        slewCompleteFlag = false;
        return MOUNT_SLEWING;
    }
}

LFAST::MountControl::MountStatus
LFAST::MountControl::mountTrackingHandler()
{
    MountCommandEvent cmdEvent = readEvent();
    if (cmdEvent == ABORT_COMMAND_RECEIVED)
        return MOUNT_IDLE;
    else if (cmdEvent == GOTO_COMMAND_RECEIVED)
        return MOUNT_SLEWING;
    else if (cmdEvent == PARK_COMMAND_RECEIVED)
        return MOUNT_PARKING;
    else if (cmdEvent == HOME_COMMAND_RECEIVED)
        return MOUNT_HOMING;

    double altCmdPreSat_rad;
    raDecToAltAz(targetRaPosn, targetDecPosn, &altCmdPreSat_rad, &azPosnCmd_rad);
    altPosnCmd_rad = saturate(altCmdPreSat_rad, MIN_ALT_ANGLE_RAD, MAX_ALT_ANGLE_RAD);
    if (altCmdPreSat_rad != altPosnCmd_rad)
    {
        return MOUNT_ERROR;
    }

    getTrackingRateCommands(&altRateCmd_rps, &azRateCmd_rps);
    // TODO: Add guider offsets
    return MOUNT_TRACKING;
}

LFAST::MountControl::MountStatus
LFAST::MountControl::mountErrorHandler()
{
    altRateCmd_rps = 0.0;
    azRateCmd_rps = 0.0;
    MountCommandEvent cmdEvent = readEvent();
    MountStatus nextStatus = MOUNT_ERROR;
    static bool printedErrorMsg = false;
    if (!printedErrorMsg)
    {
        cli.addDebugMessage("Mount Stopped (Invalid command received).", MountControl_CLI::ERROR);
        printedErrorMsg = true;
    }
    switch (cmdEvent)
    {
    case NO_COMMANDS_RECEIVED:
        nextStatus = MOUNT_ERROR;
        break;
    case PARK_COMMAND_RECEIVED:
        nextStatus = MOUNT_PARKING;
        printedErrorMsg = false;
        break;
    case UNPARK_COMMAND_RECEIVED:
        nextStatus = MOUNT_ERROR;
        break;
    case GOTO_COMMAND_RECEIVED:
        nextStatus = MOUNT_SLEWING;
        printedErrorMsg = false;
        break;
    case HOME_COMMAND_RECEIVED:
        nextStatus = MOUNT_HOMING;
        printedErrorMsg = false;
        break;
    case ABORT_COMMAND_RECEIVED:
        nextStatus = MOUNT_ERROR;
        break;
    }
    return nextStatus;
}

void LFAST::MountControl::getLocalCoordinates(double *lat, double *lon, double *alt)
{
    cli.addDebugMessage("Coordinate request received.");
    *lat = localLatitude;
    *lon = localLongitude;
    *alt = 0.0;
}

void LFAST::MountControl::updateSimMount()
{
    static bool firstTime = true;

    static uint64_t prevTimeMillis = 0;
    static uint64_t currentTimeMillis = 0;
    double deltaTimeSecLocal = 0.0;

    if (firstTime)
    {
        firstTime = false;
        prevTimeMillis = 0;
        currentTimeMillis = millis();
    }
    else
    {
        prevTimeMillis = currentTimeMillis;
        currentTimeMillis = millis();
        double deltaMilliSec = currentTimeMillis - prevTimeMillis;
        deltaTimeSecLocal = deltaMilliSec * 0.001;
    }

    auto deltaAlt = altRateCmd_rps * deltaTimeSecLocal;
    auto deltaAz = azRateCmd_rps * deltaTimeSecLocal;

    altPosnFb_rad += deltaAlt;
    // altPosnFb_rad = saturate(altPosnFb_rad, MIN_ALT_ANGLE_RAD, MAX_ALT_ANGLE_RAD);

    azPosnFb_rad += deltaAz;
    if (azPosnFb_rad > (2 * M_PI))
        azPosnFb_rad -= (2 * M_PI);
    else if (azPosnFb_rad < 0)
        azPosnFb_rad += (2 * M_PI);
}
