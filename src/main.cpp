#include <iostream>
#include <SDL2/SDL.h>

#include "wnd.h"
#include "audio.h"
#include "config.h"

int err() {
    wnd::showConsole();
    system("pause");
    return EXIT_FAILURE;
}

int main(int argc, char* argv[]) {
    wnd::hideConsole();
    if (!wnd::initSDL())
        return err();
    
    config cfg;
    cfg.input = inputType::MICROPHONE;
    cfg.deviceId = 0;
    cfg.bins = 8192;
    cfg.freq = 22050;
    if (!readConfigFromFile("config.txt", cfg))
        return err();
    audio::SAMPLE_COUNT = cfg.bins;
    audio::SAMPLE_RATE = cfg.freq;
    if (cfg.input == inputType::WAV_FILE) {
        audio::BUFFER_SOURCE = audio::bufferSource::USE_WAV_BUFFER;
        if (!audio::loadAudio(cfg.srcFile))
            return err();
    }
    else if (cfg.input == inputType::MICROPHONE) {
        wnd::showConsole();
        audio::BUFFER_SOURCE = audio::bufferSource::COPY_TO_SAMPLE_BUFFER;
        if (audio::setupMicrophone(cfg.deviceId) == -1)
            return err();
    }
    wnd::createGraphWindow("fft grapher pro");
    return EXIT_SUCCESS;
}