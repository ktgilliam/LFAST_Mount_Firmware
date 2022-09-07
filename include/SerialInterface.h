#include <array>
#include <vector>
#include <sstream>
#include <string>
#include <Arduino.h>

#if defined(ARDUINO_TEENSY41)
#define NUM_SERIAL_DEVICES 8
#else
#define NUM_SERIAL_DEVICES 7
#endif

#define TEST_SERIAL_NO 2

class SerialSettings
{
private:
    static std::vector<SerialSettings> *serialSettingsPtr;
    HardwareSerial *const hwSerialDevicePtrs[NUM_SERIAL_DEVICES] =
        {
            &Serial1,
            &Serial2,
            &Serial3,
            &Serial4,
            &Serial5,
            &Serial6,
            &Serial7,
#if defined(ARDUINO_TEENSY41)
            &Serial8
#endif
    };

    HardwareSerial *getDevicePtr()
    {
        return (hwSerialDevicePtrs[this->hwDeviceNo - 1]);
    }

public:
    uint8_t hwDeviceNo;
    unsigned int baudRate;
    std::string label;
    SerialSettings(uint8_t _num = 1, unsigned int _baud = 9600, std::string _label = "Unconfigured") : hwDeviceNo(_num), baudRate(_baud), label(_label)
    {
    }

    static SerialSettings *getSettingsPtr(uint8_t devNum)
    {
        SerialSettings *foundPtr = (SerialSettings *)0;
        std::vector<SerialSettings>::iterator itr;
        for (itr = serialSettingsPtr->begin(); itr < serialSettingsPtr->end(); itr++)
        {
            if ((itr)->hwDeviceNo == devNum)
            {
                foundPtr = &(*itr);
            }
        }
        return foundPtr;
    }

    static std::string getPrintLabel(uint8_t _num)
    {
        return getSettingsPtr(_num)->label.append(": ");
    }
    void openChannel()
    {
        this->getDevicePtr()->begin(this->baudRate);
    }

    void printToSerial(uint8_t _num, std::string str)
    {
        HardwareSerial *srlDevPtr = (hwSerialDevicePtrs[_num - 1]);
        std::stringstream ss;
        ss << this->getPrintLabel(_num) << str << std::endl;
        std::string outStr = ss.str();
        srlDevPtr->print(outStr.c_str());
    }

    static void initializeSerialSettings(std::vector<SerialSettings> *settingsRef)
    {
        SerialSettings::serialSettingsPtr = settingsRef;
        std::vector<SerialSettings>::iterator itr;
        for (itr = serialSettingsPtr->begin(); itr < serialSettingsPtr->end(); itr++)
        {
            (itr)->openChannel();
        }
        // printSettings();
    }

    static void printSettings()
    {
        delay(5);
        std::stringstream ss;
        for (std::vector<SerialSettings>::iterator itr = serialSettingsPtr->begin(); itr < serialSettingsPtr->end(); itr++)
        {
            ss << (itr)->label << ";" << (itr)->baudRate << ";" << (itr)->hwDeviceNo << std::endl;
            Serial2.print((ss.str()).c_str());
            delay(2);
        }
    }
};

void initSerialInterface();
