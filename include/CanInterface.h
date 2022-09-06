#include <FlexCAN_T4.h>

#define CAN3_TX_PIN 35
#define CAN3_RX_PIN 34
// #define CAN1_TX 

void initCanInterfaces();

bool updateCanBusEvents(CAN_message_t &msg);
void sendTestFrame();
void sendMessageOnBus(CAN_message_t &msg, uint8_t busNo);

void canSniff(const CANFD_message_t &msg);
void canSniff20(const CAN_message_t &msg);
