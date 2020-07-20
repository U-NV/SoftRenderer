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
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

//Starts up SDL and creates window
bool init();

//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;


Matrix<double> Model;
Matrix<double> View;
Matrix<double> Projection;
Matrix<double> Viewport;

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

		//首先执行缩放，接着旋转，最后才是平移
		Model = translate(0,0,0) * rotate(Vector<double>(0, 0, 1), 0) * scale(5, 5, 1);
		//Model.identity();
		View = lookat(Vector<double>(0, 0, 1), Vector<double>(0, 0, 0), Vector<double>(0, 1, 0));
		Projection = projection(SCREEN_WIDTH, SCREEN_HEIGHT, 1, 5);
		Matrix<double> MVP = Projection * View * Model;
		Viewport = viewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);


		//While application is running
		while (!quit){
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

			std::vector<Vector<double>> vertexBuffer =
			{
				Vector<double>(-10, 0,0.1),
				Vector<double>(0, 10,0.1),
				Vector<double>(10, 0,0.1),

				//Vector<double>(1, 0,0),
				//Vector<double>(0, 1,0),
				//Vector<double>(0, 0,1),

				//Vector<double>(0, 0,0),
				//Vector<double>(0, 1,0),
				//Vector<double>(1, 0,0),

				//Vector<double>(0, 0,1),
				//Vector<double>(0, 0,0),
				//Vector<double>(1, 0,0),
			};

			std::vector<TGAColor> colorBuffer =
			{
				TGAColor(255, 0, 0,255),
				TGAColor(0, 255, 0,255),
				TGAColor(0, 0, 255,255),

				//TGAColor(0, 0, 255,255),
				//TGAColor(0, 255, 0,255),
				//TGAColor(255, 0, 0,255),

				//TGAColor(255, 0, 0,255),
				//TGAColor(0, 255, 0,255),
				//TGAColor(0, 0, 255,255),

				//TGAColor(0, 0, 255,255),
				//TGAColor(0, 255, 0,255),
				//TGAColor(255, 0, 0,255),

			};
			std::cout << "Model" << std::endl;
			Model.print();
			std::cout << "View" << std::endl;
			View.print();
			std::cout << "Projection" << std::endl;
			Projection.print();
			
			std::cout << "Viewport" << std::endl;
			Viewport.print();

			for (int i = 0; i < vertexBuffer.size(); i++) {
				Matrix<double> vecToMat = vertexBuffer[i].homogeneous().toMatrix();
				std::cout << "vecToMat" << std::endl;
				vecToMat.print();
				Matrix<double> afterChange = MVP * vecToMat;
				vertexBuffer[i] = afterChange.toVector();
				std::cout << "afterChange" << std::endl;
				vertexBuffer[i].print();

				double w = vertexBuffer[i][3];
				double rhw = 1.0 / w;
				vertexBuffer[i][0] = (vertexBuffer[i][0] * rhw + 1.0f) * SCREEN_WIDTH * 0.5f;
				vertexBuffer[i][1] = (1.0f - vertexBuffer[i][1] * rhw) * SCREEN_HEIGHT * 0.5f;
				vertexBuffer[i][2] = vertexBuffer[i][2] * rhw;
				vertexBuffer[i][3] = 1.0f;

				std::cout << "view" << std::endl;
				vertexBuffer[i].print();
				
			}

			drawTriangle2D(vertexBuffer, colorBuffer, gRenderer, gWindow);


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
	View.resize(4, 4);
	Model.resize(4, 4);
	View.identity();
	Model.identity();
	Viewport.resize(4, 4);
	Viewport.identity();
	Projection.resize(4, 4);
	Projection.identity();

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
