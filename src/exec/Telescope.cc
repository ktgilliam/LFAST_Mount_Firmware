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
    if(dt > 0.0)
    {
        deltaTime = dt;
        localSiderealTime = lst;
    }
}

void LFAST::MountControl::printMountStatus()
{
    // TEST_SERIAL.printf("\033[32m");
    TEST_SERIAL.printf("\033[%u;%uH", 10, 0);
    TEST_SERIAL.printf("\033[37mINDI Time:\033[20G%8.4f\r\n", this->localSiderealTime);
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
    }
    TEST_SERIAL.println();

    TEST_SERIAL.printf("\033[37mCurrent Altitude:\033[20G%8.4f\033[0K\r\n", this->currentAltPosn);
    TEST_SERIAL.printf("\033[37mTarget Altitude:\033[20G%8.4f\033[0K\r\n", this->targetAltPosn);
    TEST_SERIAL.printf("\033[37mAltitude Rate:\033[20G%8.4f\033[0K\r\n", this->altRate_dps);
    TEST_SERIAL.println();

    TEST_SERIAL.printf("\033[37mCurrent Azimuth:\033[20G%8.4f\033[0K\r\n", this->currentAzPosn);
    TEST_SERIAL.printf("\033[37mTarget Azimuth:\033[20G%8.4f\033[0K\r\n", this->targetAzPosn);
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

void LFAST::MountControl::gotoAltAz(double altDeg, double azDeg)
{
    if (mountStatus != MOUNT_PARKED)
    {
        double altTmp = saturate(altDeg, MIN_ALT_ANGLE_DEG, MAX_ALT_ANGLE_DEG);
        if (altTmp != altDeg)
        {
            CURSOR_TO_DEBUG_ROW(3);
            TEST_SERIAL.printf("Out of bounds error");
        }
        else
        {
            targetAltPosn = altDeg;
        }
        targetAzPosn = azDeg;
        if(currentAltPosn == targetAltPosn && currentAzPosn == targetAzPosn)
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
    double ha = (localSiderealTime - ra);
    if (ha < 0)
    {
        ha += 2 * M_PI;
    }
    if (ha > M_PI)
    {
        ha = ha - 2 * M_PI;
    }
    double AzTmp = atan2(sin(ha),
                         cos(ha) * sin(localLatitude) - tan(dec) * cos(localLatitude)) -
                   M_PI;
    AzTmp = AzTmp >= 0 ? AzTmp : (AzTmp + 2 * M_PI);

    double AltTmp = asin(sin(localLatitude) * sin(dec) + cos(localLatitude) * cos(dec) * cos(ha));

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

void LFAST::MountControl::initSimMount()
{
    this->mountStatus = MOUNT_PARKED;
    currentAltPosn = altParkPosn;
    currentAzPosn = azParkPosn;
}

void LFAST::MountControl::abortSlew()
{
    mountStatus = MOUNT_IDLE;
#if SIM_SCOPE_ENABLED
#else
#warning ABORT (ALT) NOT IMPLEMENTED
#endif
}

void LFAST::MountControl::updateSimMount()
{
    CURSOR_TO_DEBUG_ROW(0);
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
        case LFAST::MountControl::MOUNT_SLEWING:
            if (std::abs(AltPosnErr) < DEFAULT_ALT_RATE)
            {
                AltPosnErr = 0.0;
                altRate_dps = 0.0;
                currentAltPosn = targetAltPosn;
            }

            if (AzPosnErr > M_PI)
            {
                AzPosnErr -= (2 * M_PI);
            }

            if (std::abs(AzPosnErr) < DEFAULT_AZ_RATE)
            {
                AzPosnErr = 0.0;
                azRate_dps = 0.0;
                currentAzPosn = targetAzPosn;
            }
            if (AltPosnErr == 0.0 && AzPosnErr == 0.0)
            {
                mountStatus = MOUNT_IDLE;
            }
            else
            {
                altRate_dps = (AltPosnErr > 0) ? DEFAULT_ALT_RATE : -1 * DEFAULT_ALT_RATE;
                azRate_dps = (AzPosnErr > 0) ? DEFAULT_AZ_RATE : -1 * DEFAULT_AZ_RATE;

                currentAltPosn += altRate_dps;
                if (currentAltPosn > (2 * M_PI))
                    currentAltPosn -= (2 * M_PI);
                else if (currentAltPosn < 0)
                    currentAltPosn += (2 * M_PI);

                currentAzPosn += azRate_dps;
                if (currentAzPosn > (2 * M_PI))
                    currentAzPosn -= (2 * M_PI);
                else if (currentAzPosn < 0)
                    currentAzPosn += (2 * M_PI);
            }
            break;
    }
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
    double haTmp = atan2(-sin(az), tan(alt) * cos(localLatitude) - cos(az) * sin(localLatitude));
    if (ha)
        *ha = haTmp >= 0 ? haTmp : haTmp + 2 * M_PI;

    double decTmp = asin(sin(localLatitude) * sin(alt) + cos(localLatitude) * cos(alt) * cos(az));
    if (dec)
        *dec = decTmp;
    return;
}