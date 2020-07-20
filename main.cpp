#include "tgaimage.h"
#include "myVector.h"
#include "myGL.h"
#include <vector>
#include <limits>
#include <cmath>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>




const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);

void printTGAColor(TGAColor color) {
	printf("r:%d g:%d b:%d\n", color.r, color.g, color.b);
}

//Screen dimension constants
const int SCREEN_WIDTH = 200;
const int SCREEN_HEIGHT = 200;

//Starts up SDL and creates window
bool init();

//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;


int main(int argc, char** argv) {
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Main loop flag
		bool quit = false;
		//Event handler
		SDL_Event e;

		//While application is running
		while (!quit)
		{
			//Handle events on queue
			while (SDL_PollEvent(&e) != 0)
			{
				//User requests quit
				if (e.type == SDL_QUIT)
				{
					quit = true;
				}
			}

			

			//Clear screen
			SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0xFF);
			SDL_RenderClear(gRenderer);

			std::vector<Vector<int>> vertexBuffer =
			{
				Vector<int>(0, 0),
				Vector<int>(0, 200),
				Vector<int>(200, 0),
				Vector<int>(86, 45),
				Vector<int>(46, 59),
				Vector<int>(85,49)
			};

			std::vector<TGAColor> colorBuffer =
			{
				TGAColor(255, 0, 0,255),
				TGAColor(0, 255, 0,255),
				TGAColor(0, 0, 255,255),

				TGAColor(0, 0, 255,255),
				TGAColor(0, 255, 0,255),
				TGAColor(255, 0, 0,255)

			};

			drawTriangle2D(vertexBuffer, colorBuffer, gRenderer, gWindow);

			//Draw vertical line of yellow dots

			//Update screen
			SDL_RenderPresent(gRenderer);
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}



bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Create window
		gWindow = SDL_CreateWindow("Taurus", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
		}
	}

	return success;
}

void close()
{
	//Destroy window    
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}
