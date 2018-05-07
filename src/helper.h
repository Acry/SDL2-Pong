#ifndef _HELPER_H_
#define _HELPER_H_

#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

SDL_Window   *Window;
SDL_Renderer *Renderer;
bool  openDisplay	 (const char title[], int width, int height);
void  updateDisplay	 (void);
void  closeDisplay	 (void);

bool running;
#endif
