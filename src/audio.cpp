#include "audio.h"
#include "fft.h"

#include <iostream>

using namespace audio;

// returns volume from 0 to 1 at a given point, reads from 32 bit integer array and converts to float
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

int totalSamples = 0;

void audio::playbackCallback(void* data, uint8_t* stream, int len) {
    if (CURRENT_OFFSET + len >= AUDIO_LENGTH) {
        SDL_PauseAudio(SDL_TRUE);
        return;
    }
	SDL_memcpy(stream, WAV_BUFFER + CURRENT_OFFSET, len);
	SDL_MixAudio(stream, WAV_BUFFER + CURRENT_OFFSET, len, SDL_MIX_MAXVOLUME);
    CURRENT_OFFSET += len;
}

void audio::recordingCallback(void* data, uint8_t* stream, int len) {
    if (totalSamples >= SAMPLE_COUNT)
        SAMPLE_BUFFER.erase(SAMPLE_BUFFER.begin(), SAMPLE_BUFFER.begin() + POLL_INTERVAL);
    for (int i = 0; i < POLL_INTERVAL; i++) {
        SAMPLE_BUFFER.push_back(readBuffer(stream, i));
    }
    totalSamples += POLL_INTERVAL;
}

void audio::loadAudio(const std::string& src) {
    SDL_AudioSpec spec;
    if (SDL_LoadWAV(src.c_str(), &spec, &WAV_BUFFER, &AUDIO_LENGTH) == nullptr) {
        std::cout << "audio file loading failed" << std::endl;
        return;
    }
    SAMPLE_RATE = spec.freq;

    spec.callback = playbackCallback;
    spec.samples = POLL_INTERVAL;
    if (SDL_OpenAudio(&spec, NULL) < 0){
	    std::cout << SDL_GetError() << std::endl;
	}
	SDL_PauseAudio(SDL_FALSE);
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
    desiredRecordingSpec.samples = POLL_INTERVAL;
    desiredRecordingSpec.callback = recordingCallback;

    // open recording device
    SDL_AudioSpec recordingSpec;
    int deviceId = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(selected, SDL_TRUE), SDL_TRUE, &desiredRecordingSpec, &recordingSpec, 0);

    if (deviceId == 0) {
        std::cout << "cannot setup device!" << std::endl;
        return -1;
    }
    return deviceId;
}

void audio::updateBins() {
    std::array<complex<float>, audio::SAMPLE_COUNT> samples;
    if (audio::BUFFER_SOURCE == audio::bufferSource::COPY_TO_SAMPLE_BUFFER) {
        for (int i = 0; i < audio::SAMPLE_COUNT; i++) {
            if (i >= audio::SAMPLE_BUFFER.size()) {
                samples[i] = .5f;
                continue;
            }
            samples[i] = audio::SAMPLE_BUFFER[i];
        }
    }
    else if (audio::BUFFER_SOURCE == audio::bufferSource::USE_WAV_BUFFER) {
        if (audio::CURRENT_OFFSET + (audio::SAMPLE_COUNT << 2) >= audio::AUDIO_LENGTH)
            return;
        int begin = audio::CURRENT_OFFSET >> 2;
        for (int i = 0; i < audio::SAMPLE_COUNT; i++) {
            int pos = (begin + i - audio::SAMPLE_COUNT / 2) << 1;
            if (pos <= 0) {
                samples[i] = .5f;
                continue;
            }
            samples[i] = audio::readBuffer(audio::WAV_BUFFER, pos);
        }
    }
    audio::BINS = fft(std::move(samples));
}

void audio::destroy() {
    SDL_CloseAudio();
    SDL_FreeWAV(WAV_BUFFER);
    WAV_BUFFER = nullptr;
}