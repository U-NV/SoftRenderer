#include "SDLWindow.h"
#include <stdio.h>
#include <stdlib.h>

inline void SDLWindow::init(const char* name,const int  SCREEN_WIDTH, const int SCREEN_HEIGHT)
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
		gWindow = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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

//将坐标系原点改为左下角
inline void SDLWindow::drawPixel(int& x, int& y, const int& width, const int& height, ColorVec* drawBuffer)
{
	int bufferInd = x + y * width;
	SDL_SetRenderDrawColor(gRenderer, drawBuffer[bufferInd].x, drawBuffer[bufferInd].y, drawBuffer[bufferInd].z, drawBuffer[bufferInd].w);
	SDL_RenderDrawPoint(gRenderer, x, height - 1 - y);
}

SDLWindow::SDLWindow(const char* name, const int SCREEN_WIDTH, const int SCREEN_HEIGHT)
{
	width = SCREEN_WIDTH;
	height = SCREEN_HEIGHT;
	init(name, width, height);
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

void SDLWindow::refresh(ColorVec* buffer)
{
	//将缓存内容赋值给屏幕
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			drawPixel(x, y, width, height, buffer);
		}
	}
	//更新屏幕显示内容
	SDL_RenderPresent(gRenderer);
}

