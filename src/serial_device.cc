#include <serial_device.h>
#include <stdlib.h>

#include <map>

namespace PacificScales {

static const std::map<BaudRate, speed_t> baudMap = {
  {BAUD_110, B110},
  {BAUD_300, B300},
  {BAUD_600, B600},
  {BAUD_1200, B1200},
  {BAUD_2400, B2400},
  {BAUD_4800, B4800},
  {BAUD_9600, B9600},
  {BAUD_19200, B19200},
  {BAUD_38400, B38400},
  {BAUD_57600, B57600},
  {BAUD_115200, B115200},
};

/**
 * @brief Destroy the Serial Device:: Serial Device object
 *
 */
SerialDevice::~SerialDevice() {
    Close();
}

/**
 * @brief Open()
 *
 * @param device  Fill path of the serial device, eg: '/dev/ttyUSB0'
 * @param baudRate One of the supported baud rates
 * @return true if Device Open was success
 * @return false  Failure
 */
bool SerialDevice::Open(const std::string device, BaudRate baudRate) {
    termios options = {};

    // validate the inputs
    auto baud = baudMap.find(baudRate);
    if (baud == baudMap.end()) {
        // invalid baud rate
        return false;
    }

    // Open device
    fd = open(device.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);

    // return error if open failed
    if (fd < 0) {
        return false;
    }

    // Set the nodelay option
    fcntl(fd, F_SETFL, FNDELAY);

    // read the current terminal options
    tcgetattr(fd, &options);

    cfsetispeed(&options, baud->second);
    cfsetospeed(&options, baud->second);

    // Configure the device with 8N1 and no Flow control
    options.c_cflag |= (CLOCAL | CREAD | CS8);
    options.c_iflag |= (IGNPAR | IGNBRK);

    tcsetattr(fd, TCSANOW, &options);

    return true;
}

/**
 * @brief Close
 * Closes the serial device and reset the internal file desc
 */
void SerialDevice::Close() {
    close(fd);
    fd = -1;
}

}