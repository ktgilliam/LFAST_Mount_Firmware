#include <device.h>


#define  CLEAR_CONSOLE() TEST_SERIAL.printf("\033[2J")
#define  CLEAR_TO_END_OF_ROW()    TEST_SERIAL.printf("\033[0K")
#define  CURSOR_TO_ZEROZERO()   TEST_SERIAL.printf("\033[%u;%uH", 0, 0)
#define  CURSOR_TO_DEBUG_ROW(n)   TEST_SERIAL.printf("\033[%u;%uH", 24+n, 0)

#define  CURSOR_TO_ROW(n)   TEST_SERIAL.printf("\033[%u;%uH", n, 0)
#define  CURSOR_TO_COL(n)  TEST_SERIAL.printf("\033[%uG", n)
#define  CURSOR_TO_ROW_COL(n, m)   TEST_SERIAL.printf("\033[%u;%uH", n, m)

#define RED()     TEST_SERIAL.printf("\033[31m");
#define GREEN()   TEST_SERIAL.printf("\033[32m");
#define YELLOW()  TEST_SERIAL.printf("\033[33m");
#define BLUE()    TEST_SERIAL.printf("\033[34m");
#define MAGENTA() TEST_SERIAL.printf("\033[35m");
#define CYAN()    TEST_SERIAL.printf("\033[36m");
#define WHITE()   TEST_SERIAL.printf("\033[37m");
#define RESET()   TEST_SERIAL.printf("\033[0m");

#define BLINKING() TEST_SERIAL.printf("\033[5m");
#define NOT_BLINKING() TEST_SERIAL.printf("\033[25m");