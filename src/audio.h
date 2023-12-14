#pragma once
#include <array>
#include <vector>
#include <complex>
#include <SDL2/SDL_audio.h>
#include <cmath>
#include <optional>


namespace audio {
    const int SAMPLE_COUNT = 8192;
    const int SAMPLE_RATE = 44100;

    inline std::array<std::complex<float>, audio::SAMPLE_COUNT> BINS;

    // depending on the input type this will be written with samples from a file or microphone input
    inline std::vector<float> SAMPLES;

    float readBuffer(const uint8_t*, unsigned int);
    float readBuffer(const SDL_AudioCVT&, unsigned int);

    void loadAudio(const char*);

    const int MICROPHONE_DEVICE_NUM = 0;
    const int MICROPHONE_POLL_INTERVAL = 64;
    int setupMicrophone();
    void recordingCallback(void*, uint8_t*, int);
};