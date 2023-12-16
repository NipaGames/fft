#include <iostream>
#include <SDL2/SDL.h>

#include "wnd.h"
#include "audio.h"
#include "config.h"


int main(int argc, char* argv[]) {
    if (!wnd::initSDL())
        return EXIT_FAILURE;
    
    config cfg;
    cfg.input = inputType::WAV_FILE;
    cfg.srcFile = "goblin.wav";
    if (cfg.input == inputType::WAV_FILE) {
        audio::BUFFER_SOURCE = audio::bufferSource::USE_WAV_BUFFER;
        audio::loadAudio(cfg.srcFile);
    }
    else if (cfg.input == inputType::MICROPHONE) {
        audio::BUFFER_SOURCE = audio::bufferSource::COPY_TO_SAMPLE_BUFFER;
        int deviceId = audio::setupMicrophone();
        if (deviceId == -1)
            return EXIT_FAILURE;
        SDL_PauseAudioDevice(deviceId, SDL_FALSE);
    }

    wnd::createGraphWindow("dft grapher pro");
    return EXIT_SUCCESS;
}