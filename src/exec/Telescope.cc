#include "Telescope.h"

#include <cmath>

#include <NetComms.h>
#include <mathFuncs.h>
#include <device.h>
#include <debug.h>

LFAST::MountControl::MountControl()
{
#if SIM_SCOPE_ENABLED
    initSimMount();
#endif
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

void LFAST::MountControl::printMountStatus()
{
    // TEST_SERIAL.printf("\033[32m");
    TEST_SERIAL.printf("\033[%u;%uH", 10, 0);
    TEST_SERIAL.printf("\033[37mLocal Sidereal Time:\033[22G%s\r\n", getClockStr(this->localSiderealTime).c_str());
    TEST_SERIAL.printf("\033[0K\033[37mMount Status:\033[22G");
    switch (this->mountStatus)
    {
    case LFAST::MountControl::MOUNT_IDLE:
        TEST_SERIAL.println("\033[33mIDLE");
        break;
    case LFAST::MountControl::MOUNT_PARKING:
        TEST_SERIAL.println("\033[31mPARKING");
        break;
    case LFAST::MountControl::MOUNT_HOMING:
        TEST_SERIAL.println("\033[31mHOMING");
        break;
    case LFAST::MountControl::MOUNT_SLEWING:
        TEST_SERIAL.println("\033[32mSLEWING");
        break;
    case LFAST::MountControl::MOUNT_PARKED:
        TEST_SERIAL.println("\033[37mPARKED");
        break;
    case LFAST::MountControl::MOUNT_TRACKING:
        TEST_SERIAL.println("\033[37mTRACKING");
        break;
    }
    TEST_SERIAL.println();

    TEST_SERIAL.printf("\033[37mCurrent Altitude:\033[20G%8.4f\033[0K\r\n", rad2deg(this->currentAltPosn));
    TEST_SERIAL.printf("\033[37mTarget Altitude:\033[20G%8.4f\033[0K\r\n", rad2deg(this->targetAltPosn));
    TEST_SERIAL.printf("\033[37mAltitude Rate:\033[20G%8.4f\033[0K\r\n", this->altRate_dps);
    TEST_SERIAL.println();

    TEST_SERIAL.printf("\033[37mCurrent Azimuth:\033[20G%8.4f\033[0K\r\n", rad2deg(this->currentAzPosn));
    TEST_SERIAL.printf("\033[37mTarget Azimuth:\033[20G%8.4f\033[0K\r\n", rad2deg(this->targetAzPosn));
    TEST_SERIAL.printf("\033[37mAzimuth Rate:\033[20G%8.4f\033[0K\r\n", this->azRate_dps);
    TEST_SERIAL.println();
}

void LFAST::MountControl::findHome()
{
    mountStatus = LFAST::MountControl::MOUNT_HOMING;
#if SIM_SCOPE_ENABLED
    targetAzPosn = 0.0;
    targetAltPosn = 0.0;
#else
#warning HOMING NOT IMPLEMENTED
#endif
}

void LFAST::MountControl::park()
{
    mountStatus = LFAST::MountControl::MOUNT_PARKING;
    targetAltPosn = altParkPosn;
    targetAzPosn = azParkPosn;
#if SIM_SCOPE_ENABLED
#else
#warning PARKING NOT IMPLEMENTED
#endif
}

void LFAST::MountControl::unpark()
{
    mountStatus = LFAST::MountControl::MOUNT_IDLE;
#if SIM_SCOPE_ENABLED
#else
#warning UNPARKING NOT IMPLEMENTED
#endif
}

void LFAST::MountControl::gotoAltAz(double altRad, double azRad)
{
    if (mountStatus != MOUNT_PARKED)
    {
        double altTmp = saturate(altRad, MIN_ALT_ANGLE_RAD, MAX_ALT_ANGLE_RAD);
        if (altTmp != altRad)
        {
            CURSOR_TO_DEBUG_ROW(3);
            TEST_SERIAL.printf("Out of bounds error: %8.4f:%8.4f", rad2deg(altRad), rad2deg(altTmp));
        }
        else
        {
            targetAltPosn = altRad;
        }
        targetAzPosn = azRad;
        if (currentAltPosn == targetAltPosn && currentAzPosn == targetAzPosn)
            mountStatus = MOUNT_IDLE;
        else
            mountStatus = MOUNT_SLEWING;
    }

#if SIM_SCOPE_ENABLED
#else
#warning GOTO ALT/AZ NOT IMPLEMENTED
#endif
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

void LFAST::MountControl::gotoRaDec(double ra, double dec)
{
    double tgtAlt, tgtAz;
    this->raDecToAltAz(ra, dec, &tgtAlt, &tgtAz);
    this->gotoAltAz(tgtAlt, tgtAz);
}

void LFAST::MountControl::syncRaDec(double ra, double dec)
{
    double newAlt, newAz;
    this->raDecToAltAz(ra, dec, &newAlt, &newAz);
    this->gotoAltAz(newAlt, newAz);
}

double LFAST::MountControl::getTrackRate()
{
    return std::sqrt(azRate_dps * azRate_dps + altRate_dps * altRate_dps) / 3200.0;
}



void LFAST::MountControl::abortSlew()
{
    mountStatus = MOUNT_IDLE;
#if SIM_SCOPE_ENABLED
#else
#warning ABORT (ALT) NOT IMPLEMENTED
#endif
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
    this->mountStatus = MOUNT_PARKED;
    currentAltPosn = altParkPosn;
    currentAzPosn = azParkPosn;
}

void LFAST::MountControl::updateSimMount()
{
    // CURSOR_TO_DEBUG_ROW(0);
    // TEST_SERIAL.println("Updating sim...");

    double AltPosnErr = targetAltPosn - currentAltPosn;
    double AzPosnErr = targetAzPosn - currentAzPosn;

    switch (mountStatus)
    {
    case LFAST::MountControl::MOUNT_PARKED:
    // Intentional fall-through
    case LFAST::MountControl::MOUNT_IDLE:
        altRate_dps = 0.0;
        azRate_dps = 0.0;
        break;
    case LFAST::MountControl::MOUNT_PARKING:
    // Intentional fall-through
    case LFAST::MountControl::MOUNT_HOMING:
    // Intentional fall-through
    case LFAST::MountControl::MOUNT_TRACKING:
    // Intentional fall-through
    case LFAST::MountControl::MOUNT_SLEWING:
        if (std::abs(AltPosnErr) < ALT_SLEW_RATE)
        {
            AltPosnErr = 0.0;
            altRate_dps = 0.0;
            currentAltPosn = targetAltPosn;
        }
        // CURSOR_TO_DEBUG_ROW(5);
        // TEST_SERIAL.printf("Az Err: %8.4f", AzPosnErr);
        while (AzPosnErr > M_PI)
        {
            AzPosnErr -= (2 * M_PI);
        }
        while (AzPosnErr < (-1*M_PI))
        {
            AzPosnErr += (2 * M_PI);
        }
        CURSOR_TO_DEBUG_ROW(6);
        TEST_SERIAL.printf("Az Err: %8.4f", AzPosnErr);
        if (std::abs(AzPosnErr) < AZ_SLEW_RATE)
        {
            AzPosnErr = 0.0;
            azRate_dps = 0.0;
            currentAzPosn = targetAzPosn;
        }
        break;
    }

    if (AltPosnErr == 0.0 && AzPosnErr == 0.0)
    {
        mountStatus = MOUNT_IDLE;
    }
    else
    {
        altRate_dps = ALT_SLEW_RATE * sign(AltPosnErr);
        azRate_dps = AZ_SLEW_RATE * sign(AzPosnErr);

        auto deltaAlt = altRate_dps * deltaTimeSec;
        auto deltaAz = azRate_dps * deltaTimeSec;

        currentAltPosn += deltaAlt;
        currentAltPosn = saturate(currentAltPosn, MIN_ALT_ANGLE_RAD, MAX_ALT_ANGLE_RAD);
        // if (currentAltPosn > (2 * M_PI))
        //     currentAltPosn -= (2 * M_PI);
        // else if (currentAltPosn < 0)
        //     currentAltPosn += (2 * M_PI);

        currentAzPosn += deltaAz;
        if (currentAzPosn > (2 * M_PI))
            currentAzPosn -= (2 * M_PI);
        else if (currentAzPosn < 0)
            currentAzPosn += (2 * M_PI);
    }
}
