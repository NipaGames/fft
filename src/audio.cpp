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

bool audio::loadAudio(const std::string& src) {
    SDL_AudioSpec spec;
    if (SDL_LoadWAV(src.c_str(), &spec, &WAV_BUFFER, &AUDIO_LENGTH) == nullptr) {
        std::cout << "audio file loading failed ('" << src <<"' doesn't exist or is of non-wav format)" << std::endl;
        return false;
    }
    SAMPLE_RATE = spec.freq;

    spec.callback = playbackCallback;
    spec.samples = POLL_INTERVAL;
    if (SDL_OpenAudio(&spec, NULL) < 0){
	    std::cout << SDL_GetError() << std::endl;
        return false;
	}
	SDL_PauseAudio(SDL_FALSE);
    return true;
}

int audioDeviceId = 0;
int audio::setupMicrophone(int device) {
    int nDevices = SDL_GetNumAudioDevices(SDL_TRUE);
    if (nDevices < 1) {
        std::cout << "no audio devices found!" << std::endl;
        return -1;
    }
    std::cout << "found " << nDevices << " audio devices:" << std::endl;
    for (int i = 0; i < nDevices; i++) {
        std::cout << "  [" << i << "] " << SDL_GetAudioDeviceName(i, SDL_TRUE) << std::endl;
    }
    int selected = device;
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
    AUDIO_DEVICE_ID = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(selected, SDL_TRUE), SDL_TRUE, &desiredRecordingSpec, &recordingSpec, 0);

    if (AUDIO_DEVICE_ID == 0) {
        std::cout << "cannot setup device!" << std::endl;
        return -1;
    }
    SDL_PauseAudioDevice(AUDIO_DEVICE_ID, SDL_FALSE);
    return AUDIO_DEVICE_ID;
}


std::array<complex<float>, audio::MAX_SAMPLE_COUNT> samples;
void audio::updateBins() {
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
        if (audio::WAV_BUFFER == nullptr)
            return;
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
    fft(samples, audio::SAMPLE_COUNT);
    audio::BINS = std::vector<complex<float>>(samples.begin(), samples.begin() + audio::SAMPLE_COUNT);
}

void audio::destroy() {
    SDL_CloseAudioDevice(AUDIO_DEVICE_ID);
    SDL_CloseAudio();
    SDL_FreeWAV(WAV_BUFFER);
    WAV_BUFFER = nullptr;
}