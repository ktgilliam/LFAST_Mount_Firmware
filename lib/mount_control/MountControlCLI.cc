
#include <cinttypes>
#include <MountControl.h>

#include <mathFuncs.h>

LFAST::MountControl_CLI::MountControl_CLI()
{

    this->resetPrompt();
};

void LFAST::MountControl_CLI::printMountStatusLabels()
{
    CLEAR_CONSOLE();
    CURSOR_TO_ROW(TOP_HEADER);
    TEST_SERIAL.printf("################################################################################################\r\n");
    TEST_SERIAL.printf("###################################### LFAST MOUNT CONTROL #####################################\r\n");
    TEST_SERIAL.printf("################################################################################################\r\n");

    // TEST_SERIAL.printf("\033[32m");
    // TEST_SERIAL.printf("\033[%u;%uH", 4, 0);

    CURSOR_TO_ROW(MOUNT_STATUS);
    TEST_SERIAL.printf("\033[0K\033[37mMount Status:\033[22G");

    CURSOR_TO_ROW(SIDEREAL_TIME);
    TEST_SERIAL.printf("\033[37mLocal Sidereal Time:\033[22G");

    CURSOR_TO_ROW(CURRENT_ALT);
    TEST_SERIAL.printf("\033[37mCurrent Altitude:\r\n");
    TEST_SERIAL.printf("\033[37mTarget Altitude:\r\n");
    TEST_SERIAL.printf("\033[37mAltitude Rate:\r\n");
    TEST_SERIAL.println();

    TEST_SERIAL.printf("\033[37mCurrent Azimuth:\r\n");
    TEST_SERIAL.printf("\033[37mTarget Azimuth:\r\n");
    TEST_SERIAL.printf("\033[37mAzimuth Rate:\r\n");
    TEST_SERIAL.println();

    CURSOR_TO_ROW_COL(MOUNT_STATUS, fieldStartCol);
    TEST_SERIAL.printf("Waiting to connect. ");
    this->resetPrompt();
}

void LFAST::MountControl_CLI::updateStatusFields(MountControl &mc)
{
    // Print the mount status field:
    CURSOR_TO_ROW_COL(MOUNT_STATUS, fieldStartCol);
    switch (mc.mountStatus)
    {
    case LFAST::MountControl::MOUNT_IDLE:
        TEST_SERIAL.print("IDLE");
        break;
    case LFAST::MountControl::MOUNT_PARKING:
        YELLOW();
        TEST_SERIAL.print("PARKING");
        break;
    case LFAST::MountControl::MOUNT_HOMING:
        TEST_SERIAL.print("HOMING");
        break;
    case LFAST::MountControl::MOUNT_SLEWING:
        MAGENTA();
        TEST_SERIAL.print("SLEWING");
        break;
    case LFAST::MountControl::MOUNT_PARKED:
        RED();
        TEST_SERIAL.print("PARKED");
        break;
    case LFAST::MountControl::MOUNT_TRACKING:
        GREEN();
        TEST_SERIAL.print("TRACKING");
        break;
    }
    CLEAR_TO_END_OF_ROW();
    WHITE();

    // Print the local sidereal time field:
    CURSOR_TO_ROW_COL(SIDEREAL_TIME, fieldStartCol);
    TEST_SERIAL.print(mc.getClockStr(mc.localSiderealTime).c_str());

    // Print current altitude:
    CURSOR_TO_ROW_COL(CURRENT_ALT, fieldStartCol);
    TEST_SERIAL.printf("%-8.4f", rad2deg(mc.currentAltPosn));

    // Print target altitude:
    CURSOR_TO_ROW_COL(TARGET_ALT, fieldStartCol);
    TEST_SERIAL.printf("%-8.4f", rad2deg(mc.altPosnCmd_rad));

    // Print target altitude:
    CURSOR_TO_ROW_COL(ALT_RATE, fieldStartCol);
    TEST_SERIAL.printf("%-8.4f", rad2deg(mc.altRateCmd_rps));

    // Print current azimuth:
    CURSOR_TO_ROW_COL(CURRENT_AZ, fieldStartCol);
    TEST_SERIAL.printf("%-8.4f", rad2deg(mc.currentAzPosn));

    // Print target azimuth:
    CURSOR_TO_ROW_COL(TARGET_AZ, fieldStartCol);
    TEST_SERIAL.printf("%-8.4f", rad2deg(mc.azPosnCmd_rad));

    // Print target azimuth:
    CURSOR_TO_ROW_COL(AZ_RATE, fieldStartCol);
    TEST_SERIAL.printf("%-8.4f", rad2deg(mc.azRateCmd_rps));
}

void LFAST::MountControl_CLI::resetPrompt()
{
    std::memset(rxBuff, '\0', CLI_BUFF_LENGTH);
    rxPtr = rxBuff;
    CURSOR_TO_ROW(PROMPT);
    TEST_SERIAL.print(">> ");
    CLEAR_TO_END_OF_ROW();
    currentInputCol = 4;
    CURSOR_TO_COL(currentInputCol);
    // BLINKING();
}

void LFAST::MountControl_CLI::serviceCLI()
{
    CURSOR_TO_ROW_COL(PROMPT, currentInputCol);
    if (TEST_SERIAL.available() > 0)
    {
        // read the incoming byte:
        char c = TEST_SERIAL.read();
        // Put it in the buffer
        if (rxPtr < (rxBuff + CLI_BUFF_LENGTH - 1))
        {
            *rxPtr++ = c;
            currentInputCol++;
        }
        // say what you got:
        TEST_SERIAL.printf("%c", c);
        // TEST_SERIAL.print(c, HEX);
        if (c == '\r' || c == '\n')
        {
            resetPrompt();
        }
    }
}