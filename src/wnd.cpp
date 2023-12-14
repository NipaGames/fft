#include <iostream>
#include <algorithm>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "wnd.h"
#include "audio.h"

using namespace wnd;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;

int prevX, prevY;
float maxAmplitude = 0.0f;
int maxBin = -1;
int binCount = 0;

SDL_Texture* graphTexture = nullptr;
SDL_Rect graphRect = { 0, 0, WND_WIDTH, WND_HEIGHT };

SDL_Texture* cursorFrequencyTextTexture = nullptr;
SDL_Rect cursorFrequencyTextRect = { 0, 80 };
SDL_Texture* cursorAmplitudeTextTexture = nullptr;
SDL_Rect cursorAmplitudeTextRect = { 0, 100 };

SDL_Texture* maxFrequencyTextTexture = nullptr;
SDL_Rect maxFrequencyTextRect = { 0, 20 };
SDL_Texture* maxAmplitudeTextTexture = nullptr;
SDL_Rect maxAmplitudeTextRect = { 0, 40 };


void renderText(const std::string& text, SDL_Texture*& texture, SDL_Rect& rect) {
    SDL_DestroyTexture(texture);
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), { 0xFF, 0xFF, 0xFF, 0xFF }); 
    if (surface != nullptr) {
        rect.w = surface->w;
        rect.h = surface->h;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
}

void centerAndClampRect(SDL_Rect& rect, int margin = 5) {
    rect.x -= rect.w / 2;
    rect.x = std::max(margin, rect.x);
    rect.x = std::min(WND_WIDTH - margin - rect.w, rect.x);
}

float wnd::wndPosToFreq(int x) {
    return pow(10.0, EXPONENT * x / (float) WND_WIDTH - EXPONENT) * audio::SAMPLE_RATE / 2.0f;
}

float wnd::wndPosToGraphPos(int x) {
    return wndPosToFreq(x) / audio::SAMPLE_RATE * audio::SAMPLE_COUNT;
}

int wnd::freqToWndPos(float x) {
    return (WND_WIDTH / EXPONENT) * log10((2.0f * x) / (float) audio::SAMPLE_RATE) + WND_WIDTH;
}

void drawLine(float freq) {
    int pos = freqToWndPos(freq);
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x40);
    SDL_RenderDrawLine(renderer, pos, 0, pos, WND_HEIGHT);
}

void wnd::renderGraph() {
    SDL_SetRenderTarget(renderer, graphTexture);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);

    drawLine(100);
    drawLine(1000);
    drawLine(10000);

    maxAmplitude = 0.0f;
    maxBin = -1;
    binCount = audio::BINS.size() / 2;
    int margin = 10;
    int actual_height = WND_HEIGHT - margin * 2;
    bool first = true;

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

    // start with 1 since 0 is irrelevant
    for (int i = 1; i < binCount; i++) {
        int xPos = (WND_WIDTH / EXPONENT) * log10(i / (float) binCount) + WND_WIDTH;
        float calc = abs(audio::BINS.at(i));
        if (calc > maxAmplitude) {
            maxBin = i;
            maxAmplitude = calc;
        }
        int yPos = actual_height - (calc / GRAPH_RANGE_Y) * actual_height + margin;

        // clamp to graph edges
        if (yPos < margin)
            yPos = margin;
        if (yPos > WND_HEIGHT - margin)
            yPos = WND_HEIGHT - margin;
        
        switch (DRAW_MODE) {
            case POINTS:
                SDL_RenderDrawPoint(renderer, xPos, yPos); 
                break;
            
            case LINEAR_INTERPOLATE:
                if (!first) {
                    SDL_RenderDrawLine(renderer, prevX, prevY, xPos, yPos);
                }
                else {
                    SDL_RenderDrawPoint(renderer, xPos, yPos); 
                    first = false;
                }
                prevX = xPos;
                prevY = yPos;
                break;
        }
    }

    SDL_SetRenderTarget(renderer, NULL);
}

bool wnd::initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("Error: SDL failed to initialize\nSDL Error: '%s'\n", SDL_GetError());
        return false;
    }
    TTF_Init();
    return true;
}

bool wnd::createGraphWindow(const char* title) {
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WND_WIDTH, WND_HEIGHT, 0);
    if (!window) {
        printf("Error: Failed to open window\nSDL Error: '%s'\n", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer){
        printf("Error: Failed to create renderer\nSDL Error: '%s'\n", SDL_GetError());
        return false;
    }
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderPresent(renderer);

    font = TTF_OpenFont(FONT_PATH, 16);
    
    graphTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, WND_WIDTH, WND_HEIGHT);
    updateBins();
    renderGraph();

    bool running = true;
    int mouseX;
    int prevMouseX = 0;
    while (running) {
        SDL_Event event;
        updateBins();

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);
        renderGraph();
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, graphTexture, NULL, &graphRect);

        while (SDL_PollEvent(&event)){
            switch (event.type) {
                case SDL_MOUSEMOTION:
                    mouseX = event.motion.x;
                    break;
                case SDL_QUIT:
                    running = false;
                    break;
                default:
                    break;
            }
        }

        if (maxBin != -1 && maxAmplitude > 1.0f) {
            int pos = (WND_WIDTH / EXPONENT) * log10(maxBin / (float) binCount) + WND_WIDTH;
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x60);
            SDL_RenderDrawLine(renderer, pos, 0, pos, WND_HEIGHT);

            renderText(std::to_string((int) wndPosToFreq(pos)) + " Hz", maxFrequencyTextTexture, maxFrequencyTextRect);
            maxFrequencyTextRect.x = pos;
            centerAndClampRect(maxFrequencyTextRect);
            SDL_RenderCopy(renderer, maxFrequencyTextTexture, NULL, &maxFrequencyTextRect);

            renderText(std::to_string(maxAmplitude), maxAmplitudeTextTexture, maxAmplitudeTextRect);
            maxAmplitudeTextRect.x = pos;
            centerAndClampRect(maxAmplitudeTextRect);
            SDL_RenderCopy(renderer, maxAmplitudeTextTexture, NULL, &maxAmplitudeTextRect);
        }

        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x80);
        SDL_RenderDrawLine(renderer, mouseX, 0, mouseX, WND_HEIGHT);

        int freq = (int) wndPosToFreq(mouseX);
        int bin = round(wndPosToGraphPos(mouseX));
        
        if (prevMouseX != mouseX) {
            renderText(std::to_string(freq) + " Hz", cursorFrequencyTextTexture, cursorFrequencyTextRect);
        }
        cursorFrequencyTextRect.x = mouseX;
        centerAndClampRect(cursorFrequencyTextRect);
        SDL_RenderCopy(renderer, cursorFrequencyTextTexture, NULL, &cursorFrequencyTextRect);

        renderText(std::to_string(std::abs(audio::BINS.at(bin))), cursorAmplitudeTextTexture, cursorAmplitudeTextRect);
        cursorAmplitudeTextRect.x = mouseX;
        centerAndClampRect(cursorAmplitudeTextRect);
        SDL_RenderCopy(renderer, cursorAmplitudeTextTexture, NULL, &cursorAmplitudeTextRect);

        SDL_RenderPresent(renderer);

        prevMouseX = mouseX;
    }
    SDL_DestroyTexture(graphTexture);
    SDL_DestroyTexture(cursorFrequencyTextTexture);
    SDL_DestroyTexture(maxFrequencyTextTexture);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return true;
}