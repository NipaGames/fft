#pragma once

#include <string>

enum class inputType {
    MICROPHONE,
    WAV_FILE
};

struct config {
    inputType input;
    std::string srcFile;
};