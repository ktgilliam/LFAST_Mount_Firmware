#pragma once

#include <cinttypes>
#include <DriveControl.h>
#include <NetComms.h>
#include <cliMacros.h>
#include <queue>
#include <TerminalInterface.h>

// #include <vector>

#define SIM_SCOPE_ENABLED 1
#define SLEW_MULT 512

#define SIDEREAL_RATE_RPS (0.000072921) //(15.041067 / 3600.0 * M_PI / 180.0)

#define DEFAULT_LATITUDE 32.096453881991074
#define DEFAULT_LONGITUDE -110.8133679034223
#define DEFAULT_ELEVATION 884 // meters

#define MAX_SLEW_RATE_RPS (SIDEREAL_RATE_RPS * SLEW_MULT)
#define FAST_SLEW_THRESH (1.5 * MAX_SLEW_RATE_RPS)

#define END_SLEW_RATE_RPS (0.1 * MAX_SLEW_RATE_RPS)
#define END_SLEW_THRESH (1.5 * END_SLEW_RATE_RPS)

#define TRACK_RATE_RPS SIDEREAL_RATE_RPS
#define TRACK_ERR_THRESH (1.5 * TRACK_RATE_RPS) //(SIDEREAL_RATE_RPS*4.0)

#define DEFAULT_SERVO_PRD 5000



#define ALT_PARK_POSN_RAD (-20.0 * M_PI / 180.0)

#define MIN_ALT_ANGLE_RAD 0.0
#define MAX_ALT_ANGLE_RAD (M_PI / 2.0)



enum MOUNT_INFO_ROWS
{
    MOUNT_STATUS_ROW,
    SIDEREAL_TIME_ROW,
    EMPTY_2_ROW,
    COMMAND_RA_ROW,
    COMMAND_DEC_ROW,
    EMPTY_3_ROW,
    CURRENT_ALT_ROW,
    TARGET_ALT_ROW,
    ALT_ERR_ROW,
    ALT_RATE_ROW,
    EMPTY_4_ROW,
    CURRENT_AZ_ROW,
    TARGET_AZ_ROW,
    AZ_ERR_ROW,
    AZ_RATE_ROW,
    EMPTY_5_ROW,
    EMPTY_6_ROW,
//     // PROMPT_ROW,
//     // PROMPT_FEEDBACK,
// #if PRINT_SERVICE_COUNTER
//     SERVICE_COUNTER_ROW,
// #endif
//     DEBUG_BORDER_1,
//     DEBUG_MESSAGE_ROW
};

namespace LFAST
{
    enum AXIS
    {
        ALT_AXIS = 0,
        AZ_AXIS = 1,
        RA_AXIS = 2,
        DEC_AXIS = 3
    };
}

class MountControl;

class MountControl
{
private:
    MountControl();
    // static MountControl *mcInstance;
    uint32_t updatePrdUs;
    double localSiderealTime = 0.0;

    // double targetAzPosn = 0.0;
    // double targetAltPosn = 0.0;

    double AltPosnErr = 0.0;
    double AzPosnErr = 0.0;

    double azParkPosn = 0.0;
    double altParkPosn = ALT_PARK_POSN_RAD;

    double targetRaPosn = 0.0;
    double targetDecPosn = 0.0;

    double altPosnCmd_rad = 0.0;
    double azPosnCmd_rad = 0.0;

    double azPosnFb_rad = 0.0;
    double altPosnFb_rad = 0.0;

    double azRateCmd_rps = 0.0;
    double altRateCmd_rps = 0.0;

    double azOffset = 0.0;
    double altOffset = 0.0;

    double localLatitude = DEFAULT_LATITUDE;
    double localLongitude = DEFAULT_LONGITUDE;
    double localElevation = DEFAULT_ELEVATION;

    double deltaTimeSec = 0.0;

    double raGuiderOffset = 0.0;
    double decGuiderOffset = 0.0;
    
    TerminalInterface *cli = nullptr;

protected:
    enum MountStatus
    {
        MOUNT_IDLE,
        MOUNT_PARKED,
        MOUNT_PARKING,
        MOUNT_SLEWING,
        MOUNT_TRACKING,
        MOUNT_HOMING,
        MOUNT_ERROR,
    };

    enum MountCommandEvent
    {
        NO_COMMANDS_RECEIVED,
        PARK_COMMAND_RECEIVED,
        UNPARK_COMMAND_RECEIVED,
        GOTO_COMMAND_RECEIVED,
        HOME_COMMAND_RECEIVED,
        // SYNC_COMMAND_RECEIVED,
        ABORT_COMMAND_RECEIVED,
    };

    std::queue<MountCommandEvent> mountCmdEvents;
    MountStatus mountStatus;

    volatile DriveControl AltDriveControl;
    volatile DriveControl AzDriveControl;

    MountCommandEvent readEvent();
    MountStatus mountIdleHandler();
    MountStatus mountParkingHandler();
    MountStatus mountParkedHandler();
    MountStatus mountSlewingHandler();
    MountStatus mountHomingHandler();
    MountStatus mountTrackingHandler();
    MountStatus mountErrorHandler();

    bool readyFlag;
    bool slewCompleteFlag;

    void updateStatusFields();
public:
    static MountControl &getMountController();

    MountControl(MountControl const &) = delete;
    void operator=(MountControl const &) = delete;

    void printMountStatus();
#if SIM_SCOPE_ENABLED
    void initSimMount();
    void updateSimMount();
#endif
    void setUpdatePeriod(uint32_t prd);

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
        return mountStatus == MOUNT_IDLE;
    }
    bool mountIsParked()
    {
        return mountStatus == MOUNT_PARKED;
    }
    bool mountIsParking()
    {
        return mountStatus == MOUNT_PARKING;
    }
    bool mountIsSlewing()
    {
        return mountStatus == MOUNT_SLEWING;
    }
    bool mountIsHoming()
    {
        return mountStatus == MOUNT_HOMING;
    }
    bool mountIsTracking()
    {
        return mountStatus == MOUNT_TRACKING;
    }
    bool mountSlewCompleted()
    {
        return slewCompleteFlag;
    }

    void getLocalCoordinates(double *lat, double *lon, double *alt);

    friend void updateMountControl_ISR();
    void findHome();
    void park();
    void unpark();
    void updateClock(double);
    void updateTargetRaDec(double ra, double dec);
    void getPosnErrors(double *altErr, double *azErr);
    void syncRaDec(double ra_hrs, double dec_deg);
    void raDecToAltAz(double ra_hrs, double dec_deg, double *alt, double *az);
    void getTrackingRateCommands(double *dAlt, double *dAz);
    bool getSlewingRateCommands(double *dAlt, double *dAz);
    static double getAxisSlewRateCommand(double axErr);
    double getParallacticAngle();

    void setGuiderOffset(uint8_t axis, double rate);
    // void setTargetAltAz(double alt, double az);

    void connectTerminalInterface(TerminalInterface* _cli);
    void setupPersistentFields();
    void serviceCLI();
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
};

void updateMountControl_ISR();