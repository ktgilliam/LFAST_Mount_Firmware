#include <device.h>


#define  CLEAR_CONSOLE() TEST_SERIAL.printf("\033[2J")
#define  CLEAR_TO_END_OF_ROW()    TEST_SERIAL.printf("\033[0K")
#define  CURSOR_TO_ZEROZERO()   TEST_SERIAL.printf("\033[%u;%uH", 0, 0)
#define  CURSOR_TO_DEBUG_ROW(n)   TEST_SERIAL.printf("\033[%u;%uH", 24+n, 0)

#define  CURSOR_TO_ROW(n)   TEST_SERIAL.printf("\033[%u;%uH", n, 0)
#define  CURSOR_TO_COL(n)  TEST_SERIAL.printf("\033[%uG", n)
#define  CURSOR_TO_ROW_COL(n, m)   TEST_SERIAL.printf("\033[%u;%uH", n, m)


#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define RESET   "\033[0m"


#define TO_RED()     TEST_SERIAL.printf("\033[31m")
#define TO_GREEN()   TEST_SERIAL.printf("\033[32m")
#define TO_YELLOW()  TEST_SERIAL.printf("\033[33m")
#define TO_BLUE()    TEST_SERIAL.printf("\033[34m")
#define TO_MAGENTA() TEST_SERIAL.printf("\033[35m")
#define TO_CYAN()    TEST_SERIAL.printf("\033[36m")
#define TO_WHITE()   TEST_SERIAL.printf("\033[37m")
#define TO_RESET()   TEST_SERIAL.printf("\033[0m")

#define BLINKING() TEST_SERIAL.printf("\033[5m")
#define NOT_BLINKING() TEST_SERIAL.printf("\033[25m")
#define HIDE_CURSOR() TEST_SERIAL.printf("\033[?25l")
#define SHOW_CURSOR() TEST_SERIAL.printf("\033[?25h")