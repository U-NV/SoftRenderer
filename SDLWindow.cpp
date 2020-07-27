#include "SDLWindow.h"
#include <stdio.h>
#include <stdlib.h>

void SDLWindow::init(const int  SCREEN_WIDTH, const int SCREEN_HEIGHT)
{
	//Initialization flag
	initSuccess = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		initSuccess = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Create window
		gWindow = SDL_CreateWindow("SoftRenderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			initSuccess = false;
		}
		else
		{
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_SOFTWARE);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				initSuccess = false;
			}
		}
	}
}

SDLWindow::SDLWindow(const int SCREEN_WIDTH, const int SCREEN_HEIGHT)
{
	init(SCREEN_WIDTH, SCREEN_HEIGHT);
}

void SDLWindow::close()
{
	//Destroy window    
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);

	//SDL_DestroyTexture(gTexture);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

void SDLWindow::refresh()
{
	SDL_RenderPresent(gRenderer);
}

//将坐标系原点改为左下角
void SDLWindow::drawPixel(int& x, int& y, const int& width, const int& height, ColorVec* drawBuffer)
{
	int bufferInd = x + y * width;
	SDL_SetRenderDrawColor(gRenderer, drawBuffer[bufferInd].x, drawBuffer[bufferInd].y, drawBuffer[bufferInd].z, drawBuffer[bufferInd].w);
	SDL_RenderDrawPoint(gRenderer, x, height - 1 - y);
}