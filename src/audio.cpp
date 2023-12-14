#include "audio.h"

#include <iostream>

using namespace audio;

// returns volume from 0 to 1 at a given point, converts from 32 bit integer array to float
float audio::readBuffer(const uint8_t* buffer, unsigned int offset) {
    int16_t i = ((int16_t*) buffer)[offset];
    return (((float) i) / UINT16_MAX + .5f);
}

float audio::readBuffer(const SDL_AudioCVT& f, unsigned int offset) {
    if (offset >= f.len) {
        std::cout << "buffer size exceeded" << std::endl;
        throw;
    }
    return readBuffer(f.buf, offset);
}

void audio::loadAudio(const char* src) {
    SDL_AudioSpec spec;
    uint8_t* buffer;
    unsigned int len;
    if (SDL_LoadWAV(src, &spec, &buffer, &len) == NULL) {
        std::cout << "audio file loading failed" << std::endl;
        return;
    }
    SDL_AudioCVT cvt;
    SDL_BuildAudioCVT(&cvt, spec.format, spec.channels, spec.freq, AUDIO_S16, 1, SAMPLE_RATE);
    cvt.buf = (uint8_t*) SDL_malloc(len * cvt.len_mult);
    SDL_memcpy(cvt.buf, buffer, len);
    cvt.len = len;
    SDL_ConvertAudio(&cvt);
    SDL_FreeWAV(buffer);

    for (int t = 0; t < len / 4; t++) {
        audio::SAMPLES.push_back(audio::readBuffer(cvt, t));
    }
}

int totalSamples = 0;
SDL_AudioSpec recordingSpec;
void audio::recordingCallback(void* data, uint8_t* stream, int len) {
    if (totalSamples >= SAMPLE_COUNT)
        SAMPLES.erase(SAMPLES.begin(), SAMPLES.begin() + MICROPHONE_POLL_INTERVAL);
    for (int i = 0; i < MICROPHONE_POLL_INTERVAL; i++) {
        SAMPLES.push_back(readBuffer(stream, i));
    }
    totalSamples += MICROPHONE_POLL_INTERVAL;
}

int audio::setupMicrophone() {
    int nDevices = SDL_GetNumAudioDevices(SDL_TRUE);
    if (nDevices < 1) {
        std::cout << "no audio devices found!" << std::endl;
        return -1;
    }
    std::cout << "found " << nDevices << " audio devices:" << std::endl;
    for (int i = 0; i < nDevices; i++) {
        std::cout << "  [" << i << "] " << SDL_GetAudioDeviceName(i, SDL_TRUE) << std::endl;
    }
    int selected = MICROPHONE_DEVICE_NUM;
    std::cout << "selecting [" << selected << "] - " << SDL_GetAudioDeviceName(selected, SDL_TRUE) << std::endl;

    SDL_AudioSpec desiredRecordingSpec;
    SDL_zero(desiredRecordingSpec);
    desiredRecordingSpec.freq = SAMPLE_RATE;
    desiredRecordingSpec.format = AUDIO_S16;
    desiredRecordingSpec.channels = 2;
    desiredRecordingSpec.samples = MICROPHONE_POLL_INTERVAL;
    desiredRecordingSpec.callback = recordingCallback;

    //Open recording device
    SDL_AudioSpec recordingSpec;
    int deviceId = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(selected, SDL_TRUE), SDL_TRUE, &desiredRecordingSpec, &recordingSpec, 0);

    if (deviceId == 0) {
        std::cout << "cannot setup device!" << std::endl;
        return -1;
    }
    return deviceId;
}