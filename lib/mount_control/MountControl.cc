#include <MountControl.h>
// MountControl *mcInstance = 0;

#include <cmath>
#include <TimerThree.h>
#include <iomanip>

// #include <TcpCommsService.h>
#include <mathFuncs.h>
#include <stdio.h>
#include <SlewDriveControl.h>
#include <TerminalInterface.h>
#include "teensy41_device.h"

const std::string SLEW_COMPLETE_MSG_STR = "Slew is complete.";
const std::string GOTO_WHILE_PARKED_MSG_STR = "Received Goto command while parked.";
const std::string ABORT_COMMAND_MSG_STR = "Received Abort Command.";

MountControl::MountControl()
    : AltDriveControl("ALT"), AzDriveControl("AZ")
{
#if SIM_SCOPE_ENABLED
    initSimMount();
#endif
    SlewDriveControl::configureLoopTimer(DEFAULT_SERVO_PRD);
    SlewDriveControl::startLoopTimer();
    readyFlag = false;

    AzDriveControl.initializeServoDrivers(AZ_SERVO_DRIVE_ID_A, AZ_SERVO_DRIVE_ID_B);
    AltDriveControl.initializeServoDrivers(ALT_SERVO_DRIVE_ID_A, ALT_SERVO_DRIVE_ID_B);
}

MountControl &MountControl::getMountController()
{
    static MountControl instance;
    return instance;
}

void MountControl::setUpdatePeriod(uint32_t prd)
{
    updatePrdUs = prd;
    Timer3.stop();
    Timer3.initialize(updatePrdUs);
    Timer3.attachInterrupt(updateMountControl_ISR);
    Timer3.start();
}

void MountControl::updateClock(double lst)
{
    double dt = lst - localSiderealTime;
    if (dt > 0.0)
    {
        deltaTimeSec = dt * 3600.0;
        localSiderealTime = lst;
    }
}

void MountControl::printMountStatus()
{
    // cli->updateStatusFields(*this);
    // cli->updateStringPersistentFieldf("\033[32m");
}

void MountControl::findHome()
{
    mountCmdEvents.push(HOME_COMMAND_RECEIVED);
#if SIM_SCOPE_ENABLED
    // targetAzPosn = 0.0;
    // targetAltPosn = 0.0;
#else
#warning HOMING NOT IMPLEMENTED
#endif
}

void MountControl::park()
{
    std::string msg = "Received park Command.";
    cli->printDebugMessage(msg);
    mountCmdEvents.push(PARK_COMMAND_RECEIVED);
}

void MountControl::unpark()
{
    std::string msg = "Received unpark Command.";
    cli->printDebugMessage(msg);
    mountCmdEvents.push(UNPARK_COMMAND_RECEIVED);
}

void MountControl::getPosnErrors(double *altErr, double *azErr)
{
    double altErrorTmp = altPosnCmd_rad - altPosnFb_rad;
    double azErrorTmp = azPosnCmd_rad - azPosnFb_rad;

    while (std::abs(azErrorTmp) > M_PI)
        azErrorTmp -= TWO_PI * sign(azErrorTmp);

    *altErr = altErrorTmp;
    *azErr = azErrorTmp;
}

void MountControl::getTrackingRateCommands(double *dAlt, double *dAz)
{
    getPosnErrors(&(this->AltPosnErr), &(this->AzPosnErr));

#if defined(POSITION_CONTROL_ONLY)
    *dAlt = TRACK_RATE_RPS * sign(AltPosnErr);
    *dAz = TRACK_RATE_RPS * sign(AzPosnErr);
#else
    // NOTE: THIS IS WRONG!!!
    // https://safe.nrao.edu/wiki/pub/Main/RadioTutorial/AzEltoSidereal.pdf
    // double ha_rad = hrs2rad(localSiderealTime - targetRaPosn);
    *dAlt = SIDEREAL_RATE_RPS * sin(altPosnCmd_rad) + cos(deg2rad(localLatitude)) * sin(azPosnCmd_rad);
    *dAz = SIDEREAL_RATE_RPS * cos(targetDecPosn) * tan(altPosnCmd_rad);
#endif
}

bool MountControl::getSlewingRateCommands(double *dAlt, double *dAz)
{
    getPosnErrors(&(this->AltPosnErr), &(this->AzPosnErr));

    double altRate = getAxisSlewRateCommand(this->AltPosnErr);
    double azRate = getAxisSlewRateCommand(this->AzPosnErr);

    bool slewCompleteFlag = (std::abs(azRate) == 0.0) && (std::abs(altRate) == 0.0);

    *dAlt = altRate;
    *dAz = azRate;

    return slewCompleteFlag;
}

double MountControl::getAxisSlewRateCommand(double axErr)
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

double MountControl::getParallacticAngle()
{
    double h = localSiderealTime; // hour angle
    // double a = hrs2rad(h - targetRaPosn); // Target right ascension
    double d = deg2rad(targetDecPosn); // Target declination
    double p = deg2rad(localLatitude); // Latitude
    double q = atan2(sin(h), (cos(d) * tan(p) - sin(d) * cos(h)));
    return q;
}

void MountControl::raDecToAltAz(double ra_hrs, double dec_deg, double *alt, double *az)
{
    double ha_rad = hrs2rad(localSiderealTime - ra_hrs);
    double dec_rad = deg2rad(dec_deg);

    if (ha_rad < 0)
    {
        ha_rad += (TWO_PI);
    }
    if (ha_rad > M_PI)
    {
        ha_rad = ha_rad - (TWO_PI);
    }

    double lat_rad = deg2rad(localLatitude);

    double AzTmp = atan2(sin(ha_rad),
                         cos(ha_rad) * sin(lat_rad) - tan(dec_rad) * cos(lat_rad)) -
                   M_PI;
    AzTmp = AzTmp >= 0 ? AzTmp : (AzTmp + TWO_PI);

    double AltTmp = asin(sin(lat_rad) * sin(dec_rad) + cos(lat_rad) * cos(dec_rad) * cos(ha_rad));

    *alt = AltTmp;
    *az = AzTmp;
}

void MountControl::updateTargetRaDec(double ra, double dec)
{
    targetRaPosn = ra;
    targetDecPosn = dec;
    char dbgMsg[50];
    sprintf(dbgMsg, "Received Goto Command: [%6.4f ; %6.4f]", ra, dec);
    cli->printDebugMessage(std::string(dbgMsg), LFAST::WARNING);

    mountCmdEvents.push(GOTO_COMMAND_RECEIVED);
}

void MountControl::syncRaDec(double ra_hrs, double dec_deg)
{
    double newAlt, newAz;
    raDecToAltAz(ra_hrs, dec_deg, &newAlt, &newAz);
    altPosnFb_rad = newAlt;
    azPosnFb_rad = newAz;
}

double MountControl::getTrackRate()
{
    return std::sqrt(azRateCmd_rps * azRateCmd_rps + altRateCmd_rps * altRateCmd_rps) / 3200.0;
}

void MountControl::abortSlew()
{
    cli->printDebugMessage(ABORT_COMMAND_MSG_STR, LFAST::WARNING);
    altPosnCmd_rad = altPosnFb_rad;
    azPosnCmd_rad = azPosnFb_rad;
    azRateCmd_rps = 0.0;
    altRateCmd_rps = 0.0;
    mountCmdEvents.push(ABORT_COMMAND_RECEIVED);
}

void MountControl::getCurrentRaDec(double *ra, double *dec)
{
    double haTmp;
    altAzToHADec(altPosnFb_rad, azPosnFb_rad, &haTmp, dec);

    if (ra)
        *ra = localSiderealTime - haTmp;
}

void MountControl::altAzToHADec(double alt, double az, double *ha, double *dec)
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

void MountControl::initSimMount()
{
    mountStatus = MOUNT_PARKED;
    altPosnFb_rad = altParkPosn;
    azPosnFb_rad = azParkPosn;
}

void updateMountControl_ISR()
{
    MountControl &mountControl = MountControl::getMountController();
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

MountControl::MountCommandEvent
MountControl::readEvent()
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

MountControl::MountStatus
MountControl::mountIdleHandler()
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

MountControl::MountStatus
MountControl::mountParkingHandler()
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
        altRateCmd_rps = 0.0;
        azRateCmd_rps = 0.0;
        AltDriveControl.setControlMode(SlewDriveControl::DISABLED);
        AzDriveControl.setControlMode(SlewDriveControl::DISABLED);
        return MOUNT_PARKED;
    }
    else
    {
        return MOUNT_PARKING;
    }
}

MountControl::MountStatus
MountControl::mountParkedHandler()
{
    altRateCmd_rps = 0.0;
    azRateCmd_rps = 0.0;
    MountCommandEvent cmdEvent = readEvent();
    if (cmdEvent == UNPARK_COMMAND_RECEIVED)
    {
        AltDriveControl.setControlMode(SlewDriveControl::VELOCITY);
        AzDriveControl.setControlMode(SlewDriveControl::VELOCITY);
        return MOUNT_IDLE;
    }
    else
    {
        return MOUNT_PARKED;
    }
}

MountControl::MountStatus
MountControl::mountHomingHandler()
{
    MountCommandEvent cmdEvent = readEvent();
    if (cmdEvent == ABORT_COMMAND_RECEIVED)
        return MOUNT_IDLE;

    altPosnCmd_rad = 0.0;
    azPosnCmd_rad = 0.0;
    AltDriveControl.setControlMode(SlewDriveControl::VELOCITY);
    altRateCmd_rps = -1 * MAX_SLEW_RATE_RPS;
    azRateCmd_rps = -1 * MAX_SLEW_RATE_RPS;
    // TODO: upon finding limit switch, back up, slow down, and bump it one more time.
    // PLACEHOLDER CODE:
    if ((azPosnFb_rad == 0.0) && (altPosnFb_rad == 0.0))
    {
        altRateCmd_rps = 0.0;
        azRateCmd_rps = 0.0;
        AltDriveControl.setControlMode(SlewDriveControl::DISABLED);
        AzDriveControl.setControlMode(SlewDriveControl::DISABLED);
        return MOUNT_IDLE;
    }
    else
    {
        return MOUNT_HOMING;
    }
}

MountControl::MountStatus
MountControl::mountSlewingHandler()
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
        cli->printDebugMessage(SLEW_COMPLETE_MSG_STR, LFAST::INFO);
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

MountControl::MountStatus
MountControl::mountTrackingHandler()
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

MountControl::MountStatus
MountControl::mountErrorHandler()
{
    altRateCmd_rps = 0.0;
    azRateCmd_rps = 0.0;
    MountCommandEvent cmdEvent = readEvent();
    MountStatus nextStatus = MOUNT_ERROR;
    static bool printedErrorMsg = false;
    if (!printedErrorMsg)
    {
        cli->printDebugMessage("Mount Stopped (Invalid command received).", LFAST::ERROR);
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

void MountControl::getLocalCoordinates(double *lat, double *lon, double *alt)
{
    cli->printDebugMessage("Coordinate request received.");
    *lat = localLatitude;
    *lon = localLongitude;
    *alt = 0.0;
}

void MountControl::setGuiderOffset(uint8_t axis, double rate)
{
    if (axis == LFAST::RA_AXIS)
        raGuiderOffset = arcsec2rad(rate);
    else if (axis == LFAST::DEC_AXIS)
        decGuiderOffset = arcsec2rad(rate);
    else
    {
        raGuiderOffset = 0.0;
        decGuiderOffset = 0.0;
        cli->printDebugMessage("Invalid guide axis", LFAST::WARNING);
    }
}

void MountControl::updateSlewDriveCommands()
{
    AzDriveControl.setVelocityCommand(RPM2radpersec(azRateCmd_rps));
    AltDriveControl.setVelocityCommand(RPM2radpersec(altRateCmd_rps));
}

void MountControl::updateSimMount()
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
    if (azPosnFb_rad > (TWO_PI))
        azPosnFb_rad -= (TWO_PI);
    else if (azPosnFb_rad < 0)
        azPosnFb_rad += (TWO_PI);
}

void MountControl::setupPersistentFields()
{

    // cli->updateStringPersistentFieldf("\033[32m");
    // cli->updateStringPersistentFieldf("\033[%u;%uH", 4, 0);

    if (cli == nullptr)
        return;

    cli->addPersistentField(this->DeviceName, "Mount Status", MOUNT_STATUS_ROW);

    cli->addPersistentField(this->DeviceName, "Local Sidereal Time", SIDEREAL_TIME_ROW);

    cli->addPersistentField(this->DeviceName, "Target RA  (hh:mm:ss)", COMMAND_RA_ROW);

    cli->addPersistentField(this->DeviceName, "Target DEC  (hh:mm:ss)", COMMAND_DEC_ROW);

    cli->addPersistentField(this->DeviceName, "Current Altitude", CURRENT_ALT_ROW);

    cli->addPersistentField(this->DeviceName, "Target Altitude", TARGET_ALT_ROW);

    cli->addPersistentField(this->DeviceName, "Altitude error", ALT_ERR_ROW);

    cli->addPersistentField(this->DeviceName, "Altitude Rate", ALT_RATE_ROW);

    cli->addPersistentField(this->DeviceName, "Current Azimuth", CURRENT_AZ_ROW);

    cli->addPersistentField(this->DeviceName, "Target Azimuth", TARGET_AZ_ROW);

    cli->addPersistentField("Azimuth error", AZ_ERR_ROW);

    cli->addPersistentField("Azimuth Rate", AZ_RATE_ROW);

    cli->printPersistentFieldLabels();
    updateStatusFields();
    // while(1);
}

void MountControl::serviceCLI()
{
    if (cli == nullptr)
        return;
    updateStatusFields();
    cli->serviceCLI();
}

void MountControl::updateStatusFields()
{
    if (cli == nullptr)
        return;
    std::string statusString;
    switch (mountStatus)
    {
    case MountControl::MOUNT_IDLE:
        cli->white();
        cli->updatePersistentField(DeviceName, MOUNT_STATUS_ROW, "IDLE");
        break;
    case MountControl::MOUNT_PARKING:
        cli->yellow();
        cli->updatePersistentField(DeviceName, MOUNT_STATUS_ROW, "PARKING");
        break;
    case MountControl::MOUNT_HOMING:
        cli->yellow();
        cli->updatePersistentField(DeviceName, MOUNT_STATUS_ROW, "HOMING");
        break;
    case MountControl::MOUNT_SLEWING:
        cli->magenta();
        cli->updatePersistentField(DeviceName, MOUNT_STATUS_ROW, "SLEWING");
        break;
    case MountControl::MOUNT_PARKED:
        cli->cyan();
        cli->updatePersistentField(DeviceName, MOUNT_STATUS_ROW, "PARKED");
        break;
    case MountControl::MOUNT_TRACKING:
        cli->green();
        cli->updatePersistentField(DeviceName, MOUNT_STATUS_ROW, "TRACKING");
        break;
    case MountControl::MOUNT_ERROR:
        cli->red();
        cli->updatePersistentField(MOUNT_STATUS_ROW, "ERROR");
    }
    cli->white();

    // const char degSymbol = (176);
    // Print the local sidereal time field:

    char lstBuff[LFAST::MAX_CLOCKBUFF_LEN];
    fs_sexa(lstBuff, localSiderealTime, 2, 3600);
    cli->updatePersistentField(DeviceName, SIDEREAL_TIME_ROW, lstBuff);

    // Print target RA:
    char raBuff[LFAST::MAX_CLOCKBUFF_LEN];
    fs_sexa(raBuff, targetRaPosn, 2, 3600);
    cli->updatePersistentField(DeviceName, COMMAND_RA_ROW, raBuff);

    // Print target DEC:
    char decBuff[LFAST::MAX_CLOCKBUFF_LEN];
    fs_sexa(decBuff, targetDecPosn, 2, 3600);
    cli->updatePersistentField(DeviceName, COMMAND_DEC_ROW, decBuff);

    // Print current altitude:
    char curAltBuf[12];
    sprintf(curAltBuf, "%-+6.4f\u00b0", rad2deg(altPosnFb_rad));
    cli->updatePersistentField(CURRENT_ALT_ROW, curAltBuf);

    // Print target altitude:
    char tgtAltBuf[12];
    sprintf(tgtAltBuf, "%-+6.4f\u00b0", rad2deg(altPosnCmd_rad));
    cli->updatePersistentField(DeviceName, TARGET_ALT_ROW, tgtAltBuf);

    // // Print altitude error:
    char altErrBuff[12];
    sprintf(altErrBuff, "%-+6.4f\u00b0", rad2deg(AltPosnErr));
    cli->updatePersistentField(DeviceName, ALT_ERR_ROW, altErrBuff);

    // Print altitude Rate:
    char altRateBuff[12];
    sprintf(altRateBuff, "%-+6.4f\u00b0/s", rad2deg(altRateCmd_rps));
    cli->updatePersistentField(DeviceName, ALT_RATE_ROW, altRateBuff);

    // Print current azimuth:
    char curAzBuf[12];
    sprintf(curAzBuf, "%-+6.4f\u00b0", rad2deg(azPosnFb_rad));
    cli->updatePersistentField(DeviceName, CURRENT_AZ_ROW, curAzBuf);

    // Print target azimuth:
    char tgtAzBuf[12];
    sprintf(tgtAzBuf, "%-+6.4f\u00b0", rad2deg(azPosnCmd_rad));
    cli->updatePersistentField(DeviceName, TARGET_AZ_ROW, tgtAzBuf);

    // Print azimuth error:
    char azErrBuff[12];
    sprintf(azErrBuff, "%-+6.4f\u00b0", rad2deg(AzPosnErr));
    cli->updatePersistentField(DeviceName, AZ_ERR_ROW, azErrBuff);

    // Print azimuth rate:
    char azRateBuff[12];
    sprintf(azRateBuff, "%-+6.4f\u00b0/s", rad2deg(azRateCmd_rps));
    cli->updatePersistentField(DeviceName, AZ_RATE_ROW, azRateBuff);
}
