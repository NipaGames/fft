#include "config.h"

#include <iostream>
#include <fstream>
#include <optional>
#include <unordered_map>
#include <filesystem>
#include <set>
#include <algorithm>

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
    while (std::isspace(ln.at(0))) {
        ln.erase(0, 1);
        if (ln.empty())
            return std::nullopt;
    }
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
    while (std::isspace(ln.at(ln.length() - 1))) {
        ln.erase(1);
    }
    val = ln;
    return symbol{ name, val };
}

// this is overengineered as hell and barely even works
void parseTokens(std::string& str) {
    size_t pos = 0;
    for (std::string::iterator it = str.begin(); it != str.end(); ++it) {
        if (*it == '<') {
            if (pos == str.length() - 1)
                break;
            std::string prev = str.substr(0, pos);
            std::string afterDelim = str.substr(pos + 1);
            size_t end = afterDelim.find_first_of('>');
            if (end == std::string::npos)
                continue;
            std::string token = afterDelim.substr(0, end);
            std::string after = afterDelim.substr(end + 1);
            if (token == "random") {
                bool isDirectory = true;

                std::string prefix = "";
                std::string postfix = "";
                int prefixStart = std::max<int>(
                    prev.find_last_of('/') == std::string::npos ? -1 : prev.find_last_of('/'),
                    prev.find_last_of('\\') == std::string::npos ? -1 : prev.find_last_of('\\'));
                if (prefixStart != -1) {
                    prefix = prev.substr(prefixStart + 1);
                }
                else {
                    isDirectory = false;
                    prefix = prev;
                }
                size_t postfixStart = std::min(after.find_first_of('/'), after.find_first_of('\\'));
                if (postfixStart != std::string::npos) {
                    isDirectory = true;
                    postfix = after.substr(0, postfixStart);
                }
                else {
                    isDirectory = false;
                    postfix = after;
                }

                std::string dir = prev.substr(0, prev.length() - prefix.length());
                std::filesystem::path parent = std::filesystem::absolute(dir.empty() ? std::filesystem::current_path() : dir);
                std::set<std::string> paths;
                for (const auto& p : std::filesystem::directory_iterator(parent)) {
                    if (p.is_directory() != isDirectory)
                        continue;
                    std::string ps = p.path().filename().string();
                    if (ps.length() < prefix.length() + postfix.length())
                        continue;
                    if (!prefix.empty() && ps.substr(0, prefix.length()) != prefix)
                        continue;
                    if (!postfix.empty() && ps.substr(ps.length() - postfix.length()) != postfix)
                        continue;
                    paths.insert(ps);
                }

                std::string file;
                if (paths.empty()) {
                    file = "<noinstances>";
                }
                else {
                    auto randIt = std::begin(paths);
                    srand(time(0));
                    std::advance(randIt, rand() % paths.size());
                    file = *randIt;
                }
                
                size_t startPos = pos - prefix.length();
                str.erase(startPos, 8 + prefix.length() + postfix.length());
                str.insert(startPos, file);
                it = str.begin() + startPos + file.length() - 1;
                pos = std::distance(str.begin(), it);
            }
        }
        ++pos;
    }
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
    if (!cfg.srcFile.empty() && cfg.srcFile.at(0) == '/') {
        cfg.srcFile.erase(0, 1);
    }
    parseTokens(cfg.srcFile);
    cfg.input = (cfg.srcFile == "<mic>" || cfg.srcFile.empty()) ? inputType::MICROPHONE : inputType::WAV_FILE;
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