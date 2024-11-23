#include <errno.h>
#include <fcntl.h>
#include <serial_device.h>
#include <stdlib.h>
#include <unistd.h>

#include <chrono>
#include <map>
#include <thread>

namespace PacificScales {

static const std::map<BaudRate, speed_t> baudMap = {
  {BaudRate::BAUD_110, B110},
  {BaudRate::BAUD_300, B300},
  {BaudRate::BAUD_600, B600},
  {BaudRate::BAUD_1200, B1200},
  {BaudRate::BAUD_2400, B2400},
  {BaudRate::BAUD_4800, B4800},
  {BaudRate::BAUD_9600, B9600},
  {BaudRate::BAUD_19200, B19200},
  {BaudRate::BAUD_38400, B38400},
  {BaudRate::BAUD_57600, B57600},
  {BaudRate::BAUD_115200, B115200},
};

/**
 * @brief Destroy the Serial Device:: Serial Device object
 *
 */
SerialDevice::~SerialDevice() {
    Close();
}

/**
 * @brief Open the Serial device
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

/**
 * @brief Read data from a serial device
 *
 * @param buffer  - Pointer to a buffer to read data into
 * @param bufferSize  - remaining free size of buffer
 * @param timeout  - maximum timeout while waiting for data
 * @return int  - number of bytes read. < 0 on Error
 */
int SerialDevice::Read(void *dataBuffer, unsigned int bufferSize, std::chrono::milliseconds timeout) {
    const auto startTime = std::chrono::system_clock::now();
    unsigned int totalBytesRead = 0;

    // Read data till timeout or dataBuffer full
    do {
        unsigned char *buff = (unsigned char *)dataBuffer + totalBytesRead;

        // TODO: Calculate actual timeout remaining
        if (!WaitForData(timeout)) {
            return totalBytesRead;
        }
        int bytesRead = read(fd, (void *)buff, bufferSize - totalBytesRead);
        // Error while reading
        if (bytesRead < 0) {
            int errsv = errno;
            std::cout << "Read Error : " << errsv << std::endl;
            return bytesRead;
        }

        if (bytesRead > 0) {
            // Some bytes have been read
            totalBytesRead += bytesRead;
            // Buffer full, return here
            if (totalBytesRead >= bufferSize)
                return totalBytesRead;
        } else {
            std::cout << "No data, sleeping for some time" << std::endl;
            // bytesRead was 0. Sleep for a but to avoid loading the cpu
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime) < timeout);
    // Timeout reached, return the number of bytes read
    return totalBytesRead;
}

/**
 * @brief Clear the input buffer
 *
 */
void SerialDevice::Flush() {
    tcflush(fd, TCIFLUSH);
}

/**
 * @brief Helper function to convert std::chrono duration to timespec
 *
 * @param dur Duration
 * @return constexpr timespec
 */
constexpr timespec durationToTimespec(std::chrono::milliseconds dur) {
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(dur);
    dur -= secs;

    return timespec {secs.count(), dur.count()};
}

/**
 * @brief Block the thread till data is ready
 *
 * @param timeout Timeout duration
 * @return true Data available
 * @return false  Data not available yet
 */
bool SerialDevice::WaitForData(std::chrono::milliseconds timeout) {
    // Setup a select call to block for serial data or a timeout
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    timespec timeout_ts(durationToTimespec(timeout));
    int ret = pselect(fd + 1, &readfds, NULL, NULL, &timeout_ts, NULL);

    if (ret <= 0) {
        // Select was interrupted or timedout
        return false;
    }

    // data is ready
    return true;
}

}