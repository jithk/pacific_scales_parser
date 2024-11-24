
#include <scale_data_parser.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

namespace PacificScales {

static std::string trim(std::string str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
                  return !std::isspace(ch);
              }));
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
                  return !std::isspace(ch);
              }).base(),
              str.end());
    return str;
}

static std::vector<std::string> split(std::string str, const std::string &delimiter) {
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        token = str.substr(0, pos);
        tokens.push_back(trim(token));
        str.erase(0, pos + delimiter.length());
    }
    tokens.push_back(trim(str));

    return tokens;
}

static std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return str;
}

static int parseWeightFromString(const std::string &str) {
    int weight = -1;
    auto lowerCaseStr = toLower(str);
    auto val = split(lowerCaseStr, "kg");
    if (val.size() == 0) {
        // invalid value
        return weight;
    }
    weight = std::atoi(val[0].c_str());
    return weight;
}

/**
 * @brief Add a single channel-mass data item to ScaleData
 *
 * @param channel Channel
 * @param mass  Mass for the channel
 */
void ScaleData::AddDataChannel(std::string channel, int32_t mass) {
    m_channelMassMap.insert({channel, mass});
}

/**
 * @brief Convert ScaleData to Json String
 *
 * @return std::string formatted JSON string of scale data
 */
std::string ScaleData::toJson() const {
    std::ostringstream jsonPrinter;
    int totalCalculated = 0;
    int totalFromData = -1;
    jsonPrinter << "{" << std::endl;
    for (auto &itr : m_channelMassMap) {
        if (itr.first.compare("TOTAL") == 0) {
            totalFromData = itr.second;
        } else {
            totalCalculated += itr.second;
        }
        jsonPrinter << "  \"" << itr.first << "\" : " << itr.second << "," << std::endl;
    }
    auto valid = (totalFromData == totalCalculated) ? "true" : "false";
    jsonPrinter << "  \"VALID\" : " << valid << std::endl;
    jsonPrinter << "}" << std::endl;

    return jsonPrinter.str();
}

/**
 * @brief Parse a single line of UART input and update the state accordingly.
 * When a full set of data is parsed, the 'latest' will be updated
 *
 * @param line Line to be parsed
 */
void ScaleDataParser::ParseLine(std::string line) {
    line = trim(line);
    if (line.empty()) {
        return;
    }
    if (line.compare("/") == 0) {
        // Start of block
        if (m_parserState != ParserState::UNKNOWN && m_parserState != ParserState::FINISHED) {
            std::cerr << "Parsing error: Invalid state" << std::endl;
        }
        m_parserState = ParserState::STARTED;
        m_current = {};
        return;
    }
    if (line.compare("\\") == 0) {
        // end of block
        if (m_parserState == ParserState::TOTAL_PARSED) {
            UpdateLatest(m_current);
        } else {
            std::cerr << "Parsing error: Invalid state" << std::endl;
        }
        m_parserState = ParserState::FINISHED;
        m_current = {};
        return;
    }
    if (line.find(":") != std::string::npos) {
        auto tokens = split(line, ":");
        auto channelName = tokens[0];
        auto weight = parseWeightFromString(tokens[1]);
        // std::cout << channelName << " = " << weight /1000 << " Tons" << std::endl;
        m_current.AddDataChannel(tokens[0], weight);
        if (tokens[0].compare("TOTAL") == 0) {
            m_parserState = ParserState::TOTAL_PARSED;
        }
    }
}

void ScaleDataParser::UpdateLatest(const ScaleData latest) {
    const std::lock_guard<std::mutex> lock(m_mutex);
    m_latest = latest;
}

}  // namespace
