#pragma once
#include <cstdint>

void initHeartbeat();
bool pingHeartBeat();
void resetHeartbeat();
void setHeartBeatPeriod(uint32_t cnts);
