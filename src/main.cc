#include <atomic>
#include <chrono>
#include <iostream>
#include <serial_device.h>
#include <sstream>
#include <string.h>
#include <thread>

#include <circular_buffer.h>
#include <scale_data_parser.h>
#include <signal.h>

PacificScales::ScaleDataParser g_scaledataParser;
PacificScales::CircularBuffer<uint8_t, 1024> g_dataBuffer;
std::atomic<bool> keepRunning = {true};

/**
 * @brief Signal handler for Ctrl-C
 */
void SignalHandler(int) {
    keepRunning = false;
}

/**
 * @brief Thread for reading data from Serial Device
 * @param device Full path to the device file
 */
void DataReaderThread(const std::string device) {
    PacificScales::SerialDevice dev;
    if (!dev.Open(device, PacificScales::BaudRate::BAUD_115200)) {
        std::cout << "Error: Failed to open device" << std::endl;
        return;
    }
    std::cout << "Opened the device : " << device << std::endl;
    dev.Flush();
    while (keepRunning) {
        auto block = g_dataBuffer.GetDataBlock();
        auto numRead = dev.Read(block.data(), block.size(), std::chrono::seconds(1));
        if (numRead <= 0) {
            // EAGAIN
            std::cout << "Got no data :" << numRead << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        // std::cout << "Got " << numRead << " bytes of data" << std::endl;
        block.MarkFilled(numRead);
    }
}

/**
 * @brief Thread for parsing data from circular buffer
  */
void DataParserThread() {
    while (keepRunning) {
        auto str = g_dataBuffer.GetLine();
        if (str.empty()) {
            // std::cout << "Buffer empty, waiting for data" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }
        g_scaledataParser.ParseLine(str);
    }
}

template <typename T>
int32_t getCurrentSeconds(T &now) {
    using namespace std::chrono;
    const auto mins = time_point_cast<minutes>(now);
    const auto secs = duration_cast<seconds>(now - mins);
    return secs.count();
}

/**
 * @brief Main entry point of the application
 * @return returns 0
 */
int main() {
    signal(SIGINT, SignalHandler);
    const auto device = std::string("/tmp/ttyIN");
    std::thread readerThread(DataReaderThread, device);
    std::thread parserThread(DataParserThread);

    while (keepRunning) {
        auto now = std::chrono::system_clock::now();
        auto secs = getCurrentSeconds(now);
        if (secs % 10 == 0) {
            const std::time_t t_c = std::chrono::system_clock::to_time_t(now);
            std::cout << "Latest weight data for: " << std::ctime(&t_c) << std::endl;
            // Print the latest scale data
            std::cout << g_scaledataParser.Latest().toJson() << std::endl;
        }
        // Sleep only 1 second. Longer sleep duration - especially if sleeping all the
        // way to next time boundary will cause an unfriendly delay while shutting down
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Shutting down " << std::endl;
    readerThread.join();
    parserThread.join();
    return 0;
}
