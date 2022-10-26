#pragma once

#include <cinttypes>
#include <DriveControl.h>
#include <NetComms.h>
#include <cliMacros.h>
#include <queue>
#include <deque>
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

#define CLI_BUFF_LENGTH 90

#define DEFAULT_SERVO_PRD 5000

#define MAX_DEBUG_ROWS 10
#define TERMINAL_WIDTH 95
#define PRINT_SERVICE_COUNTER 0

#define ALT_PARK_POSN_RAD (-20.0 * M_PI / 180.0)

#define MIN_ALT_ANGLE_RAD 0.0
#define MAX_ALT_ANGLE_RAD (M_PI / 2.0)

#define MAX_CLOCKBUFF_LEN 64

namespace LFAST
{
    enum AXIS
    {
        ALT_AXIS = 0,
        AZ_AXIS = 1,
        RA_AXIS = 2,
        DEC_AXIS = 3
    };

    enum
    {
        INFO = 0,
        DEBUG = 1,
        WARNING = 2,
        ERROR = 3
    };

}

class MountControl;
class MountControl_CLI
{
private:
    uint16_t debugMessageCount;
    uint16_t debugRowOffset;

protected:
    enum CLI_ROWS
    {
        TOP_HEADER,
        MIDDLE_HEADER,
        LOWER_HEADER,
        EMPTY_1,
        MOUNT_STATUS,
        SIDEREAL_TIME,
        EMPTY_2,
        COMMAND_RA,
        COMMAND_DEC,
        EMPTY_3,
        CURRENT_ALT,
        TARGET_ALT,
        ALT_ERR,
        ALT_RATE,
        EMPTY_4,
        CURRENT_AZ,
        TARGET_AZ,
        AZ_ERR,
        AZ_RATE,
        EMPTY_5,
        EMPTY_6,
        PROMPT,
        PROMPT_FEEDBACK,
#if PRINT_SERVICE_COUNTER
        SERVICE_COUNTER_ROW,
#endif
        DEBUG_BORDER_1,
        DEBUG_MESSAGE_ROW
    };

    const uint16_t fieldStartCol = 24;
    uint32_t currentInputCol;
    char rxBuff[CLI_BUFF_LENGTH];
    char *rxPtr;
    void handleCliCommand();
    void resetPrompt();
    std::deque<std::string> debugMessages;

public:

    MountControl_CLI();
    void updateStatusFields(MountControl &);
    void printMountStatusLabels();
    void serviceCLI();
    // void addDebugMessage(std::string&, uint8_t);
    void addDebugMessage(const std::string &msg, uint8_t level = LFAST::INFO);
    // void clearDebugMessages();
};

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

public:
    static MountControl &getMountController();

    MountControl(MountControl const &) = delete;
    void operator=(MountControl const &) = delete;

    MountControl_CLI cli;
    void initializeCLI();

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
    void serviceCLI()
    {
        cli.updateStatusFields(*this);
        cli.serviceCLI();
    }
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

    friend class MountControl_CLI;
};

void updateMountControl_ISR();