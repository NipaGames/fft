#pragma once

#define WND_WIDTH 640
#define WND_HEIGHT 480
#define FONT_PATH "ARIAL.TTF"

namespace wnd {
    enum class GraphDrawMode {
        POINTS,
        LINEAR_INTERPOLATE
    };
    const GraphDrawMode DRAW_MODE = GraphDrawMode::LINEAR_INTERPOLATE;
    const double GRAPH_RANGE_Y = 500;
    const double EXPONENT = 3.0;

    bool initSDL();

    bool createGraphWindow(const char*);
    
    float wndPosToFreq(int);
    float wndPosToGraphPos(int);
    int binToWndPos(int);
    int freqToWndPos(float);
    void renderGraph();

    void showConsole();
    void hideConsole();
};