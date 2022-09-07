
#pragma once

typedef enum
{
  CAN_TEST_MODE_TALKER,
  CAN_TEST_MODE_LISTENER
} CanTestMode;

void initCanTestExec(CanTestMode mode);
void sendDeadBeef();
void sendTestFrame();