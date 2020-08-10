#include "SDLWindow.h"
#include "myGL.h"
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

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
//inline void SDLWindow::drawPixel(int& x, int& y, const int& width, const int& height, Frame* drawBuffer)
//{
//	int bufferInd = x + y * width;
//	TGAColor* temp = &drawBuffer->getPixel(x, y);
//	SDL_SetRenderDrawColor(gRenderer, temp->bgra[2], temp->bgra[1], temp->bgra[0], temp->bgra[3]);
//	SDL_RenderDrawPoint(gRenderer, x, height - 1 - y);
//}

void ApplySurface(int x, int y, SDL_Surface* pSrc, SDL_Surface* pDest)
{
	SDL_Rect rect;

	rect.x = x;
	rect.y = y;
	rect.w = pSrc->w;
	rect.h = pSrc->h;

	SDL_BlitSurface(pSrc, NULL, pDest, &rect);
}

SDLWindow::SDLWindow(const char* name, const int SCREEN_WIDTH, const int SCREEN_HEIGHT,float screenGamma)
{
	width = SCREEN_WIDTH;
	height = SCREEN_HEIGHT;
	this->screenGamma = screenGamma;
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

void SDLWindow::refresh(Frame* buffer)
{
	//将缓存内容赋值给屏幕
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			int bufferInd = x + y * width;
			Vec4f* hdrColor = buffer->getPixel(x, y);
			//SDL_SetRenderDrawColor(gRenderer,
			//	(temp->x - int(temp->x)) * 255,
			//	(temp->y - int(temp->y)) * 255, 
			//	(temp->z - int(temp->z)) * 255, 
			//	(temp->w - int(temp->w)) * 255);

			// use the gamma corrected color in the fragment
			for (int i = 0; i < 3; i++) {
				// Gamma校正
				(*hdrColor)[i] = pow((*hdrColor)[i], 1.0f / screenGamma);
			}

			SDL_SetRenderDrawColor(gRenderer,
			int(hdrColor->x * 255),
			int(hdrColor->y * 255),
			int(hdrColor->z * 255),
			int(hdrColor->w * 255));
			SDL_RenderDrawPoint(gRenderer, x, height - 1 - y);
		}
	}
	//更新屏幕显示内容
	SDL_RenderPresent(gRenderer);
}

