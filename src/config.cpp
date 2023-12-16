#include "config.h"

#include <iostream>
#include <fstream>
#include <optional>
#include <unordered_map>
#include <set>

typedef std::pair<std::string, std::string> symbol;
typedef std::unordered_map<std::string, std::string> symbolMap;

void updateConfigStrValue(const symbolMap& symbols, const std::string& symbolName, std::string& ref) {
    if (symbols.count(symbolName) != 0)
        ref = symbols.at(symbolName);
}

bool updateConfigIntValue(const symbolMap& symbols, const std::string& symbolName, int& ref) {
    if (symbols.count(symbolName) == 0)
        return true;
    
    char* end;
    int val = std::strtol(symbols.at(symbolName).c_str(), &end, 10);
    if (*end != '\0') {
        std::cout << "cannot parse symbol '" << symbolName << "' as an integer!" << std::endl;
        return false;
    }
    ref = val;
    return true;
}

std::optional<symbol> parseLine(std::string ln) {
    std::string name;
    std::string val;
    size_t delim = ln.find_first_of(":");
    if (delim == std::string::npos || delim >= ln.length() - 1) {
        return std::nullopt;
    }
    name = ln.substr(0, delim);
    ln.erase(0, delim + 1);
    if (ln.empty())
        return std::nullopt;
    while (std::isspace(ln.at(0))) {
        ln.erase(0, 1);
        if (ln.empty())
            return std::nullopt;
    }
    val = ln;
    return symbol{ name, val };
}

bool readConfigFromFile(const std::string& src, config& cfg) {
    symbolMap symbols;

    std::ifstream stream(src);
    std::string ln;
    int lnN = 1;
    while (std::getline(stream, ln)) {
        std::optional<symbol> parsed = parseLine(ln);
        if (!parsed.has_value()) {
            std::cout << "failed parsing config! (line " << std::to_string(lnN) << ")" << std::endl;
            return false;
        }
        symbols.insert(parsed.value());
        ++lnN;  
    }

    updateConfigStrValue(symbols, "src", cfg.srcFile);
    cfg.input = (cfg.srcFile == "mic" || cfg.srcFile.empty()) ? inputType::MICROPHONE : inputType::WAV_FILE;
    if (!updateConfigIntValue(symbols, "freq", cfg.freq) ||
        !updateConfigIntValue(symbols, "bins", cfg.bins) ||
        !updateConfigIntValue(symbols, "device", cfg.deviceId))
    {
        return false;
    }

    // this could be done with some logarithm modulo magic but this is good enough for now
    const std::set<size_t> binSizes = { 512, 1024, 2048, 4096, 8192, 16384, 32768 };
    if (binSizes.count(cfg.bins) == 0) {
        std::cout << "invalid number of bins! (must be 512 <= 2^N <= 32768)" << std::endl;
        return false;
    }

    return true;
}