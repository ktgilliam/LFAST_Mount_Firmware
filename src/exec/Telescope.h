
void initMountControl();
void serviceMountControl();

#define SIM_SCOPE_ENABLED 1

struct Telescope
{
    enum ScopeStatus
    {
        SCOPE_IDLE,
        SCOPE_PARKING,
        
    };

    #if SIM_SCOPE_ENABLED
    unsigned long int parkingCounter;
    #define SCOPE_PARK_TIME_COUNT 100
    #endif
    ScopeStatus scopeStatus = SCOPE_IDLE;

    double indiTime = 0.0;
    double azPosn = 0.0;
    double elPosn = 0.0;
    bool isParked = true;
    double trackRate = 0.0;
};
