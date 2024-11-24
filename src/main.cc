#include <atomic>
#include <chrono>
#include <iostream>
#include <serial_device.h>
#include <sstream>
#include <string.h>
#include <thread>

#include <circular_buffer.h>
#include <getopt.h>
#include <scale_data_parser.h>
#include <signal.h>

PacificScales::ScaleDataParser g_scaledataParser;
PacificScales::CircularBuffer<uint8_t, 8192> g_dataBuffer;
std::atomic<bool> keepRunning = {true};

static constexpr PacificScales::BaudRate kDEFAULT_BAUD_RATE = PacificScales::BaudRate::BAUD_115200;
static constexpr const char *kDEFAULT_UART_DEVICE = "/dev/ttyUSB0";

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
void DataReaderThread(const std::string device, PacificScales::BaudRate baudRate) {
    PacificScales::SerialDevice dev;
    if (!dev.Open(device, baudRate)) {
        std::cout << "Error: Failed to open device : " << device << "@B" << baudRate << std::endl;
        keepRunning = false;
        return;
    }
    std::cout << "Opened the device : " << device << std::endl;
    dev.Flush();
    while (keepRunning) {
        auto block = g_dataBuffer.GetDataBlock();
        auto numRead = dev.Read(block.data(), block.size(), std::chrono::seconds(1));
        if (numRead <= 0) {
            // EAGAIN
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

void ShowHelpScreen(std::string appName) {
    std::cout << appName << std::endl;
    std::cout << "Parse scale data and show every 10 secs as JSON" << std::endl
              << "Arguments" << std::endl
              << "\t -p <serial_port_device> [" << kDEFAULT_UART_DEVICE << "]" << std::endl
              << "\t -b <baud_rate> [" << kDEFAULT_BAUD_RATE << "]" << std::endl;
}

/**
 * @brief Function to parse commandline args
  * @param argc Argument Count
 * @param argv  Argument Vector
 * @return std::pair<std::string, PacificScales::BaudRate>
 */
std::pair<std::string, PacificScales::BaudRate> ParseCommandlineArgs(int argc, char *argv[]) {
    auto device = std::string(kDEFAULT_UART_DEVICE);
    PacificScales::BaudRate baudRate = kDEFAULT_BAUD_RATE;

    int opt = 0;
    do {
        opt = getopt(argc, argv, "hp:b:");
        switch (opt) {
        case -1:
            break;
        case 'p':
            device = std::string(optarg);
            continue;
            ;
        case 'b':
            baudRate = static_cast<PacificScales::BaudRate>(std::atoi(optarg));
            continue;
        case 'h':
        default:
            ShowHelpScreen(argv[0]);
            exit(0);
        }
    } while (opt != -1);

    return {device, baudRate};
}

/**
 * @brief Main entry point of the application
 * @return returns 0
 */
int main(int argc, char *argv[]) {
    // setup signal handlers
    signal(SIGINT, SignalHandler);

    // Parse commandline args
    auto [device, baudRate] = ParseCommandlineArgs(argc, argv);

    std::thread readerThread(DataReaderThread, device, baudRate);
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
