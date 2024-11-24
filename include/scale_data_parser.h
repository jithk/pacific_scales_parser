#pragma once

#include <cstring>
#include <map>
#include <mutex>
#include <string>

namespace PacificScales {

class ScaleData {
public:
    void AddDataChannel(std::string channel, int32_t mass);
    bool isValid();
    std::string toJson() const;

private:
    std::map<std::string, int32_t> m_channelMassMap;
};

/**
 * @brief ThreadSafe class where you can push raw data and pop latest scale data
 *
 */
class ScaleDataParser {
    enum class ParserState
    {
        UNKNOWN,
        STARTED,  // We got '/' as input
        TOTAL_PARSED,  // All the channels are parsed, waiting for '\\'
        FINISHED,  // All done with this block
    };

public:
    void ParseLine(std::string line);
    const ScaleData Latest() {
        const std::lock_guard<std::mutex> lock(m_mutex);
        return m_latest;
    };

private:
    void UpdateLatest(const ScaleData latest);
    std::mutex m_mutex;
    ScaleData m_latest = {};
    ScaleData m_current = {};
    ParserState m_parserState = ParserState::UNKNOWN;
};

}  //namespace
