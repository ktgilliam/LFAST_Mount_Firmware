
#include <MountControl.h>

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#include <cinttypes>
#include <mathFuncs.h>

// #include <patch.h>
#define MOUNT_CONTROL_LABEL " LFAST MOUNT CONTROL "

LFAST::MountControl_CLI::MountControl_CLI()
{
    CLEAR_CONSOLE();
    debugRowOffset = 0;
    debugMessageCount = 0;
    this->resetPrompt();
};

void LFAST::MountControl_CLI::printMountStatusLabels()
{
    CURSOR_TO_ROW(TOP_HEADER);
    TO_WHITE();

    std::string HEADER_BORDER_STRING = std::string(TERMINAL_WIDTH, '#');
    std::string HEADER_LABEL_STRING = MOUNT_CONTROL_LABEL;
    std::string HEADER_LABEL_ROW_SIDE = std::string((TERMINAL_WIDTH - HEADER_LABEL_STRING.size()) / 2, '#');
    std::string HEADER_LABEL_ROW = HEADER_LABEL_ROW_SIDE + HEADER_LABEL_STRING + HEADER_LABEL_ROW_SIDE;

    TEST_SERIAL.printf("%s\r\n", HEADER_BORDER_STRING.c_str());
    TEST_SERIAL.printf("%s\r\n", HEADER_LABEL_ROW.c_str());
    TEST_SERIAL.printf("%s\r\n", HEADER_BORDER_STRING.c_str());

    // TEST_SERIAL.printf("\033[32m");
    // TEST_SERIAL.printf("\033[%u;%uH", 4, 0);

    CURSOR_TO_ROW(MOUNT_STATUS);
    TEST_SERIAL.printf("\033[0K\033[37mMount Status:\033[22G");

    CURSOR_TO_ROW(SIDEREAL_TIME);
    TEST_SERIAL.printf("\033[37mLocal Sidereal Time:\033[22G");

    CURSOR_TO_ROW(CURRENT_ALT);
    TEST_SERIAL.printf("\033[37mCurrent Altitude:");
    CURSOR_TO_ROW(TARGET_ALT);
    TEST_SERIAL.printf("\033[37mTarget Altitude:");
    CURSOR_TO_ROW(ALT_ERR);
    TEST_SERIAL.printf("\033[37mAltitude error:");
    CURSOR_TO_ROW(ALT_RATE);
    TEST_SERIAL.printf("\033[37mAltitude Rate:");

    CURSOR_TO_ROW(CURRENT_AZ);
    TEST_SERIAL.printf("\033[37mCurrent Azimuth:");
    CURSOR_TO_ROW(TARGET_AZ);
    TEST_SERIAL.printf("\033[37mTarget Azimuth:");
    CURSOR_TO_ROW(AZ_ERR);
    TEST_SERIAL.printf("\033[37mAzimuth error:");
    CURSOR_TO_ROW(AZ_RATE);
    TEST_SERIAL.printf("\033[37mAzimuth Rate:");

    CURSOR_TO_ROW_COL(MOUNT_STATUS, fieldStartCol);
    // TEST_SERIAL.printf("Waiting for connection to INDI Server. ");

    CURSOR_TO_ROW_COL(DEBUG_BORDER_1, 0);
    std::string DEBUG_BORDER_STR = std::string(TERMINAL_WIDTH, '-');
    TEST_SERIAL.printf("%s\r\n", DEBUG_BORDER_STR.c_str());

    this->resetPrompt();
}

void LFAST::MountControl_CLI::updateStatusFields(MountControl &mc)
{
    HIDE_CURSOR();
    // Print the mount status field:
    CURSOR_TO_ROW_COL(MOUNT_STATUS, fieldStartCol);
    switch (mc.mountStatus)
    {
    case LFAST::MountControl::MOUNT_IDLE:
        TO_WHITE();
        TEST_SERIAL.print("IDLE");
        break;
    case LFAST::MountControl::MOUNT_PARKING:
        TO_YELLOW();
        TEST_SERIAL.print("PARKING");
        break;
    case LFAST::MountControl::MOUNT_HOMING:
        TO_CYAN();
        TEST_SERIAL.print("HOMING");
        break;
    case LFAST::MountControl::MOUNT_SLEWING:
        TO_MAGENTA();
        TEST_SERIAL.print("SLEWING");
        break;
    case LFAST::MountControl::MOUNT_PARKED:
        TO_RED();
        TEST_SERIAL.print("PARKED");
        break;
    case LFAST::MountControl::MOUNT_TRACKING:
        TO_GREEN();
        TEST_SERIAL.print("TRACKING");
        break;
    }
    CLEAR_TO_END_OF_ROW();
    TO_WHITE();

    // Print the local sidereal time field:
    CURSOR_TO_ROW_COL(SIDEREAL_TIME, fieldStartCol);
    TEST_SERIAL.print(mc.getClockStr(mc.localSiderealTime).c_str());

    // Print current altitude:
    CURSOR_TO_ROW_COL(CURRENT_ALT, fieldStartCol);
    TEST_SERIAL.printf("%-8.4f", rad2deg(mc.currentAltPosn));

    // Print target altitude:
    CURSOR_TO_ROW_COL(TARGET_ALT, fieldStartCol);
    TEST_SERIAL.printf("%-8.4f", rad2deg(mc.altPosnCmd_rad));

    // Print altitude error:
    CURSOR_TO_ROW_COL(ALT_ERR, fieldStartCol);
    TEST_SERIAL.printf("%-8.4f", rad2deg(mc.AltPosnErr));

    // Print altitude Rate:
    CURSOR_TO_ROW_COL(ALT_RATE, fieldStartCol);
    TEST_SERIAL.printf("%-8.4f", rad2deg(mc.altRateCmd_rps));

    // Print current azimuth:
    CURSOR_TO_ROW_COL(CURRENT_AZ, fieldStartCol);
    TEST_SERIAL.printf("%-8.4f", rad2deg(mc.currentAzPosn));

    // Print target azimuth:
    CURSOR_TO_ROW_COL(TARGET_AZ, fieldStartCol);
    TEST_SERIAL.printf("%-8.4f", rad2deg(mc.azPosnCmd_rad));

    // Print azimuth error:
    CURSOR_TO_ROW_COL(AZ_ERR, fieldStartCol);
    TEST_SERIAL.printf("%-8.4f", rad2deg(mc.AzPosnErr));

    // Print azimuth rate:
    CURSOR_TO_ROW_COL(AZ_RATE, fieldStartCol);
    TEST_SERIAL.printf("%-8.4f", rad2deg(mc.azRateCmd_rps));
}

void LFAST::MountControl_CLI::resetPrompt()
{
    SHOW_CURSOR();
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
#if PRINT_SERVICE_COUNTER
    static uint64_t serviceCounter = 0;
    CURSOR_TO_ROW_COL(SERVICE_COUNTER_ROW, 0);
    TEST_SERIAL.printf("[%o]", serviceCounter++);
#endif

    CURSOR_TO_ROW_COL(PROMPT, currentInputCol);
    if (TEST_SERIAL.available() > 0)
    {
        // read the incoming byte:
        char c = TEST_SERIAL.read();

        if (c == '\r' || c == '\n')
        {
            handleCliCommand();
        }
        else
        {
            // say what you got:
            TEST_SERIAL.printf("%c", c);
            // Put it in the buffer
            if (rxPtr < (rxBuff + CLI_BUFF_LENGTH - 1))
            {
                *rxPtr++ = c;
                currentInputCol++;
            }
        }
    }
}

void LFAST::MountControl_CLI::handleCliCommand()
{
    CURSOR_TO_ROW(PROMPT_FEEDBACK);
    CLEAR_TO_END_OF_ROW();
    TEST_SERIAL.printf("%s: Command Not Found.\r\n", rxBuff);

    resetPrompt();
}

void LFAST::MountControl_CLI::addDebugMessage(std::string msg, uint8_t level)
{
    debugMessageCount++;
    std::string colorStr;
    switch (level)
    {
    case INFO:
        colorStr = WHITE;
        break;
    case DEBUG:
        colorStr = GREEN;
        break;
    case WARNING:
        colorStr = YELLOW;
        break;
    case ERROR:
        colorStr = RED;
        break;
    }
    std::stringstream ss;
    ss << std::setiosflags(std::ios::left) << std::setw(6);
    ss << WHITE << debugMessageCount + 1 << ": " << colorStr << msg;
    std::string msgPrintSr = ss.str();

    // TEST_SERIAL.printf("[%d]", debugMessageCount);

    if (debugMessages.size() < MAX_DEBUG_ROWS)
    {
        CURSOR_TO_ROW_COL((DEBUG_MESSAGE_ROW + debugRowOffset++), 0);
        CLEAR_TO_END_OF_ROW();
        debugMessages.push_back(msgPrintSr);
        TEST_SERIAL.println(msgPrintSr.c_str());
    }
    else
    {
        debugMessages.pop_front();
        debugMessages.push_back(msgPrintSr);
        for (uint16_t ii = 0; ii < MAX_DEBUG_ROWS; ii++)
        {
            CURSOR_TO_ROW_COL((DEBUG_MESSAGE_ROW + ii), 0);
            CLEAR_TO_END_OF_ROW();
            TEST_SERIAL.println(debugMessages.at(ii).c_str());
        }
    }
}
