# fft
yeah it's a c++ sdl audio fft grapher.
nothing fancy really, supports both .wav and microphone inputs.
## compiling
build.bat compiles and runs this, mingw and gcc required.
the code relies on some win32-functionality but porting shouldn't be that hard
## a few things future me could add
- option to change between linear and logarithmic scales
- window functions
- more intuitive memory layout, at the moment this relies on std::arrays which are pretty fast but a pain in the ass to manage (shit ton of templating and boilerplate needed), they seem like premature optimization anyway, also with the unclear copies and moves
- better quality playback (the audio clips sometimes)