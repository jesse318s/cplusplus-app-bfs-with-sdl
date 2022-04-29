#ifndef GLOBALS_H
#define GLOBALS_H
#include <sdl.h>

#define WIDTH 1280
#define HIGHT 720

extern SDL_Renderer * ren;
extern SDL_Window * win;

using namespace std;

static int init = SDL_Init(SDL_INIT_EVERYTHING);

#endif /* GLOBALS_H */

