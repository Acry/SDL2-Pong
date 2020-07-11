#include "helper.h"
SDL_Window *Window;
SDL_Renderer *Renderer;
bool running;

bool openDisplay(const char title[], int width, int height)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		return false;
	Window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
	if (!Window)
	{
		return false;
	}
	SDL_ShowCursor(SDL_DISABLE);
	TTF_Init();
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	srand(time(NULL));
	//BEGIN RENDERER
	Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_SetRenderDrawBlendMode(Renderer, SDL_BLENDMODE_BLEND);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	//END 	RENDERER

	//BEGIN ICON
	SDL_Surface *icon;
	icon = IMG_Load("./assets/gfx/icon.png");
	SDL_SetWindowIcon(Window, icon);
	SDL_FreeSurface(icon);
	//END 	ICON
	return true;
}

void updateDisplay(void)
{
	SDL_Event event;
	// process pending events
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
			running = false;

		if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				running = false;
				break;
			case SDLK_r:

				break;
			case SDLK_g:
				break;
			case SDLK_b:
				break;
			default:
				break;
			}
		}
	}
}

void closeDisplay(void)
{
	TTF_Quit();
	Mix_CloseAudio();
	SDL_DestroyRenderer(Renderer);
	SDL_DestroyWindow(Window);
	SDL_Quit();
}
