#include <FlexCAN_T4.h>

#define CAN3_PIN 35
#define CAN1_PIN 34


void initCanInterfaces();

// bool updateCanBusEvents(CAN_message_t &msg);
void sendTestFrame();
void sendMessageOnBus(CAN_message_t &msg, uint8_t busNo);
bool updateCanBusEvents(CAN_message_t &msg);


void canSniff(const CANFD_message_t &msg);
void canSniff20(const CAN_message_t &msg);
