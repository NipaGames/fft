#include <iostream>
#include <SDL2/SDL.h>

#include "wnd.h"
#include "audio.h"
#include "config.h"


int main(int argc, char* argv[]) {
    if (!wnd::initSDL())
        return EXIT_FAILURE; // literally me
    
    config cfg;
    cfg.input = inputType::MICROPHONE;
    cfg.deviceId = 0;
    cfg.bins = 8192;
    cfg.freq = 22050;
    if (!readConfigFromFile("config.txt", cfg)) {
        return EXIT_FAILURE;
    }
    audio::SAMPLE_COUNT = cfg.bins;
    audio::SAMPLE_RATE = cfg.freq;
    if (cfg.input == inputType::WAV_FILE) {
        audio::BUFFER_SOURCE = audio::bufferSource::USE_WAV_BUFFER;
        audio::loadAudio(cfg.srcFile);
    }
    else if (cfg.input == inputType::MICROPHONE) {
        audio::BUFFER_SOURCE = audio::bufferSource::COPY_TO_SAMPLE_BUFFER;
        int deviceId = audio::setupMicrophone(cfg.deviceId);
        if (deviceId == -1)
            return EXIT_FAILURE; // bloodborne reference!! ??? ?? ohmahgad
        SDL_PauseAudioDevice(deviceId, SDL_FALSE);
    }

    wnd::createGraphWindow("dft grapher pro");
    return EXIT_SUCCESS;
}