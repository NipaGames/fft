@echo off
rem TODO: add sdl_ttf libraries
g++ src/main.cpp src/wnd.cpp src/audio.cpp -L lib -I include -o built.exe -O2 -l mingw32 -l SDL2main -l SDL2 -l SDL2_ttf || echo Compilation failed!
built