#pragma once

#include <array>
#include <vector>
#include <complex>
#include <SDL2/SDL_audio.h>
#include <cmath>
#include <optional>

namespace audio {
    constexpr size_t MAX_SAMPLE_COUNT = 32768;
    // NOT in bytes
    // going over 32768 will cause stack overflows, so don't
    // (or alternatively heap-allocate everything)
    inline size_t SAMPLE_COUNT;
    inline uint32_t SAMPLE_RATE;
    const int POLL_INTERVAL = 64;

    // in bytes
    inline uint32_t AUDIO_LENGTH = 0;
    // in bytes
    inline uint32_t CURRENT_OFFSET = 0;

    inline std::vector<std::complex<float>> BINS;

    inline std::vector<float> SAMPLE_BUFFER;
    inline uint8_t* WAV_BUFFER = nullptr;

    enum class bufferSource {
        COPY_TO_SAMPLE_BUFFER,
        USE_WAV_BUFFER
    };
    inline bufferSource BUFFER_SOURCE;

    float readBuffer(const uint8_t*, unsigned int);
    float readBuffer(const SDL_AudioCVT&, unsigned int);

    void updateBins();

    bool loadAudio(const std::string&);
    void destroy();

    inline int AUDIO_DEVICE_ID = 0;
    int setupMicrophone(int);
    void playbackCallback(void*, uint8_t*, int);
    void recordingCallback(void*, uint8_t*, int);
};