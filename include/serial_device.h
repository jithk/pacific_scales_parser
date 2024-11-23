#pragma once

#include <iostream>
#include <stdlib.h>
#include <string>
#include <termios.h>

namespace PacificScales {

// We only support a limite set of baud rates
enum class BaudRate
{
    BAUD_110,
    BAUD_300,
    BAUD_600,
    BAUD_1200,
    BAUD_2400,
    BAUD_4800,
    BAUD_9600,
    BAUD_19200,
    BAUD_38400,
    BAUD_57600,
    BAUD_115200,
};

class SerialDevice {
public:
    SerialDevice() = default;
    ~SerialDevice();

    bool Open(const std::string device, BaudRate baudRate);
    bool isDeviceOpen() { return fd >= 0; };
    void Close();

private:
    int fd = -1;
};

}  // namespace
