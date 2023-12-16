#pragma once

#include <string>

enum class inputType {
    MICROPHONE,
    WAV_FILE
};

struct config {
    inputType input;
    int deviceId;
    std::string srcFile;
    int freq;
    int bins;
};

bool readConfigFromFile(const std::string&, config&);