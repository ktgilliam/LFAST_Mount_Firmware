#pragma once

#include <NetComms.h>

#define SIM_SCOPE_ENABLED 1

#if SIM_SCOPE_ENABLED
#define SIDEREAL_RATE_DPS (15.041067 / 3600.0)
#define DEFAULT_AZ_RATE (SIDEREAL_RATE_DPS * 4)
#define DEFAULT_ALT_RATE (SIDEREAL_RATE_DPS * 4)
#define SCOPE_PARK_TIME_COUNT 100
#endif

namespace LFAST
{
    class MountControl
    {

    protected:
        enum MountStatus
        {
            MOUNT_IDLE,
            MOUNT_PARKED,
            MOUNT_PARKING,
            MOUNT_SLEWING,
            MOUNT_HOMING,

        };
        double indiTime = 0.0;

        double azParkPosn = 3.14159;
        double altParkPosn = 0.0;

        double currentAzPosn = 0.0;
        double currentAltPosn = 0.0;

        double targetAzPosn = 0.0;
        double targetAltPosn = 0.0;

        double azRate_dps = 0.0;
        double altRate_dps = 0.0;

        double azOffset = 0.0;
        double altOffset = 0.0;

        MountStatus mountStatus;

    public:
        MountControl();

        void printMountStatus();

#if SIM_SCOPE_ENABLED
        void initSimMount();
        void updateSimMount(double);
#endif

        bool mountIsIdle()
        {
            return mountStatus == LFAST::MountControl::MOUNT_IDLE;
        }
        bool mountIsParked() { return mountStatus == LFAST::MountControl::MOUNT_PARKED; }
        bool mountIsParking() { return mountStatus == LFAST::MountControl::MOUNT_PARKING; }
        bool mountIsSlewing() { return mountStatus == LFAST::MountControl::MOUNT_SLEWING; }
        bool mountIsHoming() { return mountStatus == LFAST::MountControl::MOUNT_HOMING; }

        void findHome();
        void park();
        void unpark();
        void setTime(double t) { indiTime = t; }
        void gotoAlt(double);
        void gotoAz(double);
        void syncAlt(double a) { currentAltPosn = a; }
        void syncAz(double a) { currentAzPosn = a; }
        void abortSlew();
        double getCurrentAlt() { return currentAltPosn-altOffset; }
        double getCurrentAz() { return currentAzPosn-azOffset; }
        double getTrackRate();


        enum AXIS
        {
            AZ_AXIS = 0,
            EL_AXIS = 1
        };
    };

}