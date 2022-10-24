#include <MountControl.h>
// LFAST::MountControl *mcInstance = 0;

#include <cmath>
#include <TimerThree.h>

#include <DriveControl.h>
#include <NetComms.h>
#include <mathFuncs.h>
#include <device.h>
#include <debug.h>

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

std::string LFAST::MountControl::getClockStr(double lst)
{
    auto wholeHours = static_cast<unsigned int>(lst);
    auto minutesSeconds = (lst - wholeHours) * 60.0;
    auto wholeMinutes = static_cast<unsigned int>(minutesSeconds);
    auto seconds = (minutesSeconds - wholeMinutes) * 60.0;
    auto wholeSeconds = static_cast<unsigned int>((seconds));
    std::stringstream ss;
    ss << wholeHours << ":" << wholeMinutes << ":" << wholeSeconds;

    return ss.str();
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
    // targetAltPosn = altParkPosn;
    // targetAzPosn = azParkPosn;
#if SIM_SCOPE_ENABLED
#else
#warning PARKING NOT IMPLEMENTED
#endif
}

void LFAST::MountControl::unpark()
{
    std::string msg = "Received unpark Command.";
    cli.addDebugMessage(msg);
    mountCmdEvents.push(UNPARK_COMMAND_RECEIVED);
}

void LFAST::MountControl::updatePosnErrors()
{
    AltPosnErr = altPosnCmd_rad - currentAltPosn;
    AzPosnErr = azPosnCmd_rad - currentAzPosn;
    while (AzPosnErr > M_PI)
    {
        AzPosnErr -= (2 * M_PI);
    }
    while (AzPosnErr < (-1 * M_PI))
    {
        AzPosnErr += (2 * M_PI);
    }
}

#define USE_DUMB_VERSION 1
void LFAST::MountControl::getTrackingRateCommands(double *dAlt, double *dAz)
{
// NOTE: THIS IS WRONG!!!
#if USE_DUMB_VERSION
    *dAlt = SIDEREAL_RATE_RPS * sign(AltPosnErr);
    *dAz = SIDEREAL_RATE_RPS * sign(AzPosnErr);
#else

#endif
    // https://safe.nrao.edu/wiki/pub/Main/RadioTutorial/AzEltoSidereal.pdf
    // double ha_rad = ha2rad(localSiderealTime - targetRaPosn);
    *dAlt = SIDEREAL_RATE_RPS * sin(altPosnCmd_rad) + cos(deg2rad(localLatitude)) * sin(azPosnCmd_rad);
    *dAz = SIDEREAL_RATE_RPS * cos(targetDecPosn) * tan(altPosnCmd_rad);
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
        ha_rad += 2 * M_PI;
    }
    if (ha_rad > M_PI)
    {
        ha_rad = ha_rad - 2 * M_PI;
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
    mountCmdEvents.push(GOTO_COMMAND_RECEIVED);
}

void LFAST::MountControl::syncRaDec(double ra, double dec)
{
    double newAlt, newAz;
    raDecToAltAz(ra, dec, &newAlt, &newAz);
    currentAltPosn = newAlt;
    currentAzPosn = newAz;
}

double LFAST::MountControl::getTrackRate()
{
    return std::sqrt(azRateCmd_rps * azRateCmd_rps + altRateCmd_rps * altRateCmd_rps) / 3200.0;
}

void LFAST::MountControl::abortSlew()
{
    std::string msg = "Received abort Command.";
    cli.addDebugMessage(msg, MountControl_CLI::WARNING);
    altPosnCmd_rad = currentAltPosn;
    azPosnCmd_rad = currentAzPosn;
    azRateCmd_rps = 0.0;
    altRateCmd_rps = 0.0;
    mountCmdEvents.push(ABORT_COMMAND_RECEIVED);
}

void LFAST::MountControl::getCurrentRaDec(double *ra, double *dec)
{
    double haTmp;
    altAzToHADec(currentAltPosn, currentAzPosn, &haTmp, dec);

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
    currentAltPosn = altParkPosn;
    currentAzPosn = azParkPosn;
}

void LFAST::updateMountControl_ISR()
{
    MountControl &mountControl = LFAST::MountControl::getMountController();
    // std::string msg = "INSIDE MOUNT ISR!!";
    // mountControl.cli.addDebugMessage(msg);
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
    case LFAST::MountControl::MOUNT_TRACKING:
        mountControl.mountStatus = mountControl.mountTrackingHandler();
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
        if (mountStatus == MOUNT_PARKED)
            nextStatus = MOUNT_IDLE;
        else
            nextStatus = mountStatus;
        break;
    case GOTO_COMMAND_RECEIVED:
        if (mountStatus == MOUNT_PARKED)
            nextStatus = mountStatus;
        else
            nextStatus = MOUNT_SLEWING;
        break;
    case HOME_COMMAND_RECEIVED:
        nextStatus = MOUNT_HOMING;
        break;
    case ABORT_COMMAND_RECEIVED:
        nextStatus = MOUNT_IDLE;
        break;
    }
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

    updatePosnErrors();

    if (AltPosnErr == 0.0 && AzPosnErr == 0.0)
    {
        return MOUNT_PARKED;
    }
    else
    {
        altRateCmd_rps = ALT_SLEW_RATE * sign(AltPosnErr);
        azRateCmd_rps = AZ_SLEW_RATE * sign(AzPosnErr);
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
    updatePosnErrors();
    if (AltPosnErr == 0.0 && AzPosnErr == 0.0)
    {
        altRateCmd_rps = 0.0;
        azRateCmd_rps = 0.0;
        return MOUNT_IDLE;
    }
    else
    {
        altRateCmd_rps = ALT_SLEW_RATE * sign(AltPosnErr);
        azRateCmd_rps = AZ_SLEW_RATE * sign(AzPosnErr);
        return MOUNT_HOMING;
    }
}

LFAST::MountControl::MountStatus
LFAST::MountControl::mountSlewingHandler()
{
    MountCommandEvent cmdEvent = readEvent();
    if (cmdEvent == ABORT_COMMAND_RECEIVED)
        return MOUNT_IDLE;
    raDecToAltAz(targetRaPosn, targetDecPosn, &altPosnCmd_rad, &azPosnCmd_rad);
    updatePosnErrors();
    if (std::abs(AltPosnErr) < TRACK_ERR_THRESH && std::abs(AzPosnErr) < TRACK_ERR_THRESH)
    {
        getTrackingRateCommands(&altRateCmd_rps, &azRateCmd_rps);
        return MOUNT_TRACKING;
    }
    else
    {
        altRateCmd_rps = ALT_SLEW_RATE * sign(AltPosnErr);
        azRateCmd_rps = AZ_SLEW_RATE * sign(AzPosnErr);
        return MOUNT_SLEWING;
    }
}

LFAST::MountControl::MountStatus
LFAST::MountControl::mountTrackingHandler()
{
    MountCommandEvent cmdEvent = readEvent();
    if (cmdEvent == ABORT_COMMAND_RECEIVED)
        return MOUNT_IDLE;
    raDecToAltAz(targetRaPosn, targetDecPosn, &altPosnCmd_rad, &azPosnCmd_rad);
    updatePosnErrors();
    getTrackingRateCommands(&altRateCmd_rps, &azRateCmd_rps);
    // TODO: Add guider offsets
    return MOUNT_TRACKING;
}

void LFAST::MountControl::updateSimMount()
{
    // TEST_SERIAL.println("Updating sim...");
    if (std::abs(AzPosnErr) < azRateCmd_rps)
    {
        AzPosnErr = 0.0;
        currentAzPosn = azPosnCmd_rad;
    }
    if (std::abs(AltPosnErr) < altRateCmd_rps)
    {
        AltPosnErr = 0.0;
        currentAltPosn = altPosnCmd_rad;
    }

    auto deltaAlt = altRateCmd_rps * deltaTimeSec;
    auto deltaAz = azRateCmd_rps * deltaTimeSec;

    currentAltPosn += deltaAlt;
    currentAltPosn = saturate(currentAltPosn, MIN_ALT_ANGLE_RAD, MAX_ALT_ANGLE_RAD);

    currentAzPosn += deltaAz;
    if (currentAzPosn > (2 * M_PI))
        currentAzPosn -= (2 * M_PI);
    else if (currentAzPosn < 0)
        currentAzPosn += (2 * M_PI);
}
