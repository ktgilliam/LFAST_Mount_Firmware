#include <unity.h>
#include <Arduino.h>

void dumbTest(void)
{
    TEST_ASSERT_EQUAL(32, 32);
}

void setup()
{
    // NOTE!!! Wait for >2 secs
    delay(5000);

    UNITY_BEGIN();
    RUN_TEST(dumbTest);
    UNITY_END();
}

void loop()
{
}