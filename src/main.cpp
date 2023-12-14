#include <iostream>
#include <complex>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#include <chrono>
#include <SDL2/SDL.h>

#include "wnd.h"
#include "audio.h"

using std::complex;

enum inputType {
    MICROPHONE,
    WAV_FILE
};

inputType AUDIO_INPUT = inputType::MICROPHONE;
const char* FILE_PATH = "goblin.wav";


float freq(float f, float x) {
    return cos(2 * M_PI * f * x);
}

float convertAmplitude(float k, complex<float> sum) {
    float amplitude = abs(sum) * k / (audio::SAMPLE_RATE / 2.0);
    return 10.0 * log10(1.0 + amplitude);
}

template <size_t N>
void dft(std::array<complex<float>, N>& a) {
    for (int k = 0; k < N; k++) {
        const complex i(0.0f, 1.0f);
        complex sum(0.0f, 0.0f);
        for (int n = 0; n < N; n++) {
            complex y = a[n];
            complex factor = exp(-i * 2.0 * (float) M_PI * (k * n / (float) N));
            sum += y * factor;
        }
        a[k] = sum;
    }
}

template<size_t N>
void fftInPlace(std::array<complex<float>, N>& a) {
    if (N == 1)
        return;

    // might want to change these to vectors if a greater sample size is used in case of stack overflow
    // at the moment these should be quite faster than vectors
    std::array<complex<float>, N / 2> evens;
    std::array<complex<float>, N / 2> odds;
    for (int k = 0; k < N / 2; k++) {
        evens[k] = a[2 * k];
        odds[k] = a[2 * k + 1];
    }
    fftInPlace(evens);
    fftInPlace(odds);

    const complex i(0.0f, 1.0f);
    complex W = exp(-i * 2.0f * (float) M_PI / (float) N);
    complex w(1.0f);
    for (int k = 0; k < N / 2; k++) {
        a[k] = evens[k] + w * odds[k];
        a[k + N / 2] = evens[k] - w * odds[k];
        w *= W;
    }
}

template<size_t N>
std::array<complex<float>, N> fft(std::array<complex<float>, N> a) {
    fftInPlace(a);
    return a;
}

int playbackStart = -1;
int getAudioPos() {
    if (playbackStart == -1)
        return 0;
    return (int) SDL_GetTicks() - playbackStart;
}

std::array<complex<float>, audio::SAMPLE_COUNT> samples;
void wnd::updateBins() {
    int begin;
    if (AUDIO_INPUT == inputType::WAV_FILE) {
        begin = (getAudioPos() / 1000.0) * audio::SAMPLE_RATE;
        if (begin < 0)
            return;
    }
    else if (AUDIO_INPUT == inputType::MICROPHONE) {
        begin = 0;
    }
    if (begin + audio::SAMPLE_COUNT > audio::SAMPLES.size())
        return;
    
    for (int i = 0; i < audio::SAMPLE_COUNT; i++) {
        samples[i] = audio::SAMPLES[i + begin];
    }
    audio::BINS = fft(samples);
}

int main(int argc, char* argv[]) {
    if (!wnd::initSDL())
        return EXIT_FAILURE;
    
    if (AUDIO_INPUT == inputType::WAV_FILE) {
        audio::loadAudio(FILE_PATH);
        playbackStart = SDL_GetTicks() + 2000;
    }
    else if (AUDIO_INPUT == inputType::MICROPHONE) {
        int deviceId = audio::setupMicrophone();
        if (deviceId == -1)
            return EXIT_FAILURE;
        SDL_PauseAudioDevice(deviceId, SDL_FALSE);
    }

    wnd::createGraphWindow("dft grapher pro");
    return EXIT_SUCCESS;
}