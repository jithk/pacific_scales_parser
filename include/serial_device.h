#pragma once

#include <chrono>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <termios.h>

namespace PacificScales {

// We only support a limite set of baud rates
enum BaudRate
{
    BAUD_110 = 110,
    BAUD_300 = 300,
    BAUD_600 = 600,
    BAUD_1200 = 1200,
    BAUD_2400 = 2400,
    BAUD_4800 = 4800,
    BAUD_9600 = 9600,
    BAUD_19200 = 19200,
    BAUD_38400 = 38400,
    BAUD_57600 = 57600,
    BAUD_115200 = 115200,
};

class SerialDevice {
public:
    SerialDevice() = default;
    ~SerialDevice();

    bool Open(const std::string device, BaudRate baudRate);
    bool isDeviceOpen() { return m_fd >= 0; };
    void Close();
    int Read(void *dataBuffer, unsigned int bufferSize, std::chrono::milliseconds timeout);
    void Flush();
    bool WaitForData(std::chrono::milliseconds timeout);

private:
    int m_fd = -1;
};

}  // namespace
