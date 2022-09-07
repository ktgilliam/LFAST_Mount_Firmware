 /// 
 ///  @ Author: Kevin Gilliam
 ///  @ Create Time: 2022-09-06 10:42:33
 ///  @ Modified by: Kevin Gilliam
 ///  @ Modified time: 2022-09-07 15:30:04
 ///  @ Description:
 ///

#pragma once

typedef enum
{
  CAN_TEST_MODE_TALKER,
  CAN_TEST_MODE_LISTENER
} CanTestMode;

void initCanTestExec(CanTestMode mode);
void sendDeadBeef();
void sendTestFrame();