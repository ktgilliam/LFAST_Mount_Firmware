#include <KincoServoInterface.h>

#include <TerminalInterface.h>

#include <cinttypes>

#include "KinkoNamespace.h"
#include "BitFieldUtil.h"

uint16_t KincoServoInterface::numDrivers = 0;

template <typename T>
T KincoServoInterface::readDriverRegister(uint16_t modBusAddr)
{
    uint16_t result_code = 0;
    ConversionBuffer32 rxBuff;
    uint16_t numWords = sizeof(T) / 2;

    // read *len Holding Register values from (slave) *node_id, address *addr
    if (ModbusRTUClient.requestFrom(driverNodeId, HOLDING_REGISTERS, modBusAddr, numWords))
    {
        for (int ii = 0; ii < numWords; ii++)
        {
            if (ModbusRTUClient.available())
            {
                auto value = ModbusRTUClient.read();
                if (value >= 0)
                    rxBuff.U16[ii] = (uint16_t)value;
                else
                    result_code = 2 + ii;
            }
        }
    }
    else
    {
        result_code = 1;
        // TODO send error to terminal interface
        return static_cast<T>(0);
    }
    if (numWords == 1)
        return static_cast<T>(rxBuff.PARTS.UNSIGNED.LOWER);
    else
        return static_cast<T>(rxBuff.U32);
}

template <typename T>
uint16_t KincoServoInterface::writeDriverRegister(uint16_t modBusAddr, T reg_value)
{
    uint16_t result_code = 0;   
    uint16_t numWords = sizeof(T) / 2;

    ConversionBuffer32 txBuff;
    txBuff.U32 = reg_value;

    // write len register values to (slave) id node_id, starting at address addr
    ModbusRTUClient.beginTransmission(driverNodeId, HOLDING_REGISTERS, modBusAddr, numWords);
    for (int ii = 0; ii < numWords; ii++)
    {
        ModbusRTUClient.write(txBuff.PARTS.U16[ii]);
    }
    if (!ModbusRTUClient.endTransmission())
    {
        // Serial.print("failed writing to modbus! ");
        // Serial.println(ModbusRTUClient.lastError());
        result_code = 1;
    }
    else
    {
        // if(print_debug_lib) Serial.println("success");
        result_code = 0;
    }
    return result_code;
}

void KincoServoInterface::setDriverState(uint16_t motor_state)
{
    writeDriverRegister<uint16_t>(KINKO::CONTROL_WORD, motor_state);
}

void KincoServoInterface::setControlMode(uint16_t motor_mode)
{
    writeDriverRegister<uint16_t>(KINKO::OPERATION_MODE, motor_mode);
}