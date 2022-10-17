#pragma once

#include <NetComms.h>

#define SIM_SCOPE_ENABLED 1

#if SIM_SCOPE_ENABLED
#define SIDEREAL_RATE_DPS (15.041067 / 3600.0)
#define DEFAULT_AZ_RATE (SIDEREAL_RATE_DPS * 4)
#define DEFAULT_ALT_RATE (SIDEREAL_RATE_DPS * 4)
#define SCOPE_PARK_TIME_COUNT 100
#endif

#define MIN_ALT_ANGLE_DEG 0
#define MAX_ALT_ANGLE_DEG 90

namespace LFAST
{
class MountControl
{
    private:
        double localSiderealTime = 0.0;

        double currentAzPosn = 0.0;
        double currentAltPosn = 0.0;

        double targetAzPosn = 0.0;
        double targetAltPosn = 0.0;


        double azParkPosn = 3.14159;
        double altParkPosn = 0.0;



        double azRate_dps = 0.0;
        double altRate_dps = 0.0;

        double azOffset = 0.0;
        double altOffset = 0.0;

        double localLatitude = 0.0;
        double localLongitude = 0.0;

        double deltaTime = 0.0;

    protected:
        enum MountStatus
        {
            MOUNT_IDLE,
            MOUNT_PARKED,
            MOUNT_PARKING,
            MOUNT_SLEWING,
            MOUNT_HOMING,
        };

        MountStatus mountStatus;

    public:
        MountControl();

        void printMountStatus();

#if SIM_SCOPE_ENABLED
        void initSimMount();
        void updateSimMount();
#endif
        void setLatitude(double lat)
        {
            localLatitude = lat;
        }
        void setLongitude(double lon)
        {
            localLongitude = lon;
        }
        bool mountIsIdle()
        {
            return mountStatus == LFAST::MountControl::MOUNT_IDLE;
        }
        bool mountIsParked()
        {
            return mountStatus == LFAST::MountControl::MOUNT_PARKED;
        }
        bool mountIsParking()
        {
            return mountStatus == LFAST::MountControl::MOUNT_PARKING;
        }
        bool mountIsSlewing()
        {
            return mountStatus == LFAST::MountControl::MOUNT_SLEWING;
        }
        bool mountIsHoming()
        {
            return mountStatus == LFAST::MountControl::MOUNT_HOMING;
        }


        void findHome();
        void park();
        void unpark();
        void updateClock(double);
        void gotoAltAz(double alt, double az);
        void gotoRaDec(double ra, double dec);
        void syncRaDec(double ra, double dec);
        void raDecToAltAz(double ra, double dec, double *alt, double *az);

        void abortSlew();

        void getCurrentRaDec(double *ra, double *dec);
        void altAzToHADec(double alt, double az, double *ha, double *dec);


        // double getCurrentAlt()
        // {
        //     return currentAltPosn - altOffset;
        // }
        // double getCurrentAz()
        // {
        //     return currentAzPosn - azOffset;
        // }
        double getTrackRate();


        enum AXIS
        {
            ALT_AXIS = 0,
            AZ_AXIS = 1,
            RA_AXIS = 2,
            DEC_AXIS = 3
        };
};

}