
#include <MountControl.h>

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdio.h>

#include <cinttypes>
#include <mathFuncs.h>

// #include <patch.h>
#define MOUNT_CONTROL_LABEL " LFAST MOUNT CONTROL "

int fs_sexa(char *out, double a, int w, int fracbase);

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

    CURSOR_TO_ROW(TOP_HEADER);
    TEST_SERIAL.printf("%s", HEADER_BORDER_STRING.c_str());
    CURSOR_TO_ROW(MIDDLE_HEADER);
    TEST_SERIAL.printf("%s", HEADER_LABEL_ROW.c_str());
    CURSOR_TO_ROW(LOWER_HEADER);
    TEST_SERIAL.printf("%s", HEADER_BORDER_STRING.c_str());

    // TEST_SERIAL.printf("\033[32m");
    // TEST_SERIAL.printf("\033[%u;%uH", 4, 0);

    CURSOR_TO_ROW(MOUNT_STATUS);
    TEST_SERIAL.printf("\033[0K\033[37mMount Status:\033[22G");

    CURSOR_TO_ROW(SIDEREAL_TIME);
    TEST_SERIAL.printf("\033[37mLocal Sidereal Time:\033[22G");

    CURSOR_TO_ROW(COMMAND_RA);
    TEST_SERIAL.printf("\033[37mTarget RA  (hh:mm:ss):\033[22G");
    CURSOR_TO_ROW(COMMAND_DEC);
    TEST_SERIAL.printf("\033[37mTarget DEC (dd:mm:ss):\033[22G");

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
    TEST_SERIAL.printf("%s", DEBUG_BORDER_STR.c_str());

    this->resetPrompt();
}

void LFAST::MountControl_CLI::updateStatusFields(MountControl &mc)
{
    HIDE_CURSOR();
    // Print the mount status field:
    CURSOR_TO_ROW_COL(MOUNT_STATUS, fieldStartCol);
    switch (mc.mountStatus)
    {
    case MountControl::MOUNT_IDLE:
        TO_WHITE();
        TEST_SERIAL.print("IDLE");
        break;
    case MountControl::MOUNT_PARKING:
        TO_YELLOW();
        TEST_SERIAL.print("PARKING");
        break;
    case MountControl::MOUNT_HOMING:
        TO_CYAN();
        TEST_SERIAL.print("HOMING");
        break;
    case MountControl::MOUNT_SLEWING:
        TO_MAGENTA();
        TEST_SERIAL.print("SLEWING");
        break;
    case MountControl::MOUNT_PARKED:
        TO_CYAN();
        TEST_SERIAL.print("PARKED");
        break;
    case MountControl::MOUNT_TRACKING:
        TO_GREEN();
        TEST_SERIAL.print("TRACKING");
        break;
    case MountControl::MOUNT_ERROR:
        TO_RED();
        TEST_SERIAL.print("ERROR");
    }
    CLEAR_TO_END_OF_ROW();
    TO_WHITE();

    // const char degSymbol = (176);
    // Print the local sidereal time field:
    char lstBuff[MAX_CLOCKBUFF_LEN];
    fs_sexa(lstBuff, mc.localSiderealTime, 2, 3600);
    CURSOR_TO_ROW_COL(SIDEREAL_TIME, fieldStartCol);
    TEST_SERIAL.print(lstBuff);

    // Print target RA:
    char raBuff[MAX_CLOCKBUFF_LEN];
    fs_sexa(raBuff, mc.targetRaPosn, 2, 3600);
    CURSOR_TO_ROW_COL(COMMAND_RA, fieldStartCol);
    // TEST_SERIAL.printf("%-+6.4f", (mc.targetRaPosn));
    TEST_SERIAL.print(raBuff);

    // Print target DEC:
    char decBuff[MAX_CLOCKBUFF_LEN];
    fs_sexa(decBuff, mc.targetDecPosn, 2, 3600);
    CURSOR_TO_ROW_COL(COMMAND_DEC, fieldStartCol);
    // TEST_SERIAL.printf("%-+6.4f", (mc.targetDecPosn));
    TEST_SERIAL.print(decBuff);

    // Print current altitude:
    CURSOR_TO_ROW_COL(CURRENT_ALT, fieldStartCol);
    TEST_SERIAL.printf("%-+6.4f\u00b0", rad2deg(mc.altPosnFb_rad));
    CLEAR_TO_END_OF_ROW();

    // Print target altitude:
    CURSOR_TO_ROW_COL(TARGET_ALT, fieldStartCol);
    TEST_SERIAL.printf("%-+6.4f\u00b0", rad2deg(mc.altPosnCmd_rad));
    CLEAR_TO_END_OF_ROW();

    // Print altitude error:
    CURSOR_TO_ROW_COL(ALT_ERR, fieldStartCol);
    TEST_SERIAL.printf("%-+6.4f\u00b0", rad2deg(mc.AltPosnErr));
    CLEAR_TO_END_OF_ROW();

    // Print altitude Rate:
    CURSOR_TO_ROW_COL(ALT_RATE, fieldStartCol);
    TEST_SERIAL.printf("%-+6.4f\u00b0/s", rad2deg(mc.altRateCmd_rps));
    CLEAR_TO_END_OF_ROW();

    // Print current azimuth:
    CURSOR_TO_ROW_COL(CURRENT_AZ, fieldStartCol);
    TEST_SERIAL.printf("%-+6.4f\u00b0", rad2deg(mc.azPosnFb_rad));
    CLEAR_TO_END_OF_ROW();

    // Print target azimuth:
    CURSOR_TO_ROW_COL(TARGET_AZ, fieldStartCol);
    TEST_SERIAL.printf("%-+6.4f\u00b0", rad2deg(mc.azPosnCmd_rad));
    CLEAR_TO_END_OF_ROW();

    // Print azimuth error:
    CURSOR_TO_ROW_COL(AZ_ERR, fieldStartCol);
    TEST_SERIAL.printf("%-+6.4f\u00b0", rad2deg(mc.AzPosnErr));
    CLEAR_TO_END_OF_ROW();

    // Print azimuth rate:
    CURSOR_TO_ROW_COL(AZ_RATE, fieldStartCol);
    TEST_SERIAL.printf("%-+6.4f\u00b0/s", rad2deg(mc.azRateCmd_rps));
    CLEAR_TO_END_OF_ROW();
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

void LFAST::MountControl_CLI::addDebugMessage(const std::string &msg, uint8_t level)
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
    ss << WHITE << debugMessageCount << ": " << colorStr << msg;
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


int fs_sexa(char *out, double a, int w, int fracbase)
{
    char *out0 = out;
    unsigned long n;
    int d;
    int f;
    int m;
    int s;
    int isneg;

    /* save whether it's negative but do all the rest with a positive */
    isneg = (a < 0);
    if (isneg)
        a = -a;

    /* convert to an integral number of whole portions */
    n = (unsigned long)(a * fracbase + 0.5);
    d = n / fracbase;
    f = n % fracbase;

    /* form the whole part; "negative 0" is a special case */
    if (isneg && d == 0)
        out += snprintf(out, MAX_CLOCKBUFF_LEN, "%*s-0", w - 2, "");
    else
        out += snprintf(out, MAX_CLOCKBUFF_LEN, "%*d", w, isneg ? -d : d);

    /* do the rest */
    switch (fracbase)
    {
    case 60: /* dd:mm */
        m = f / (fracbase / 60);
        out += snprintf(out, MAX_CLOCKBUFF_LEN, ":%02d", m);
        break;
    case 600: /* dd:mm.m */
        out += snprintf(out, MAX_CLOCKBUFF_LEN, ":%02d.%1d", f / 10, f % 10);
        break;
    case 3600: /* dd:mm:ss */
        m = f / (fracbase / 60);
        s = f % (fracbase / 60);
        out += snprintf(out, MAX_CLOCKBUFF_LEN, ":%02d:%02d", m, s);
        break;
    case 36000: /* dd:mm:ss.s*/
        m = f / (fracbase / 60);
        s = f % (fracbase / 60);
        out += snprintf(out, MAX_CLOCKBUFF_LEN, ":%02d:%02d.%1d", m, s / 10, s % 10);
        break;
    case 360000: /* dd:mm:ss.ss */
        m = f / (fracbase / 60);
        s = f % (fracbase / 60);
        out += snprintf(out, MAX_CLOCKBUFF_LEN, ":%02d:%02d.%02d", m, s / 100, s % 100);
        break;
    default:
        printf("fs_sexa: unknown fracbase: %d\n", fracbase);
        return -1;
    }

    return (out - out0);
}
