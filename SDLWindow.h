#pragma once

#include "SDL.h"
#include "SDL_revision.h"

#include "myVector.h"

class SDLWindow
{
	
	//The window we'll be rendering to
	SDL_Window* gWindow = NULL;
	//The window renderer
	SDL_Renderer* gRenderer = NULL;
	//Starts up SDL and creates window
	inline void init(const int SCREEN_WIDTH, const int SCREEN_HEIGHT);
public:
	bool initSuccess;
	SDLWindow(const int SCREEN_WIDTH, const int SCREEN_HEIGHT);
	//Frees media and shuts down SDL
	void close();

	void refresh();

	void drawPixel(int& x, int& y, const int& width, const int& height, ColorVec* drawBuffer);
};

