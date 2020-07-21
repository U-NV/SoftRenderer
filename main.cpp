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
const int SCREEN_HEIGHT = 100;

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


static Vector<double> viewport_transform(int width, int height, Vector<double> ndc_coord) {
	float x = (ndc_coord[0] + 1) * 0.5f * (float)width;   /* [-1, 1] -> [0, w] */
	float y = (ndc_coord[1] + 1) * 0.5f * (float)height;  /* [-1, 1] -> [0, h] */
	float z = (ndc_coord[2] + 1) * 0.5f;                  /* [-1, 1] -> [0, 1] */
	return Vector<double>(x, y, z);
}

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
		//Model = translate(0,0,0.001) * rotate(Vector<double>(0, 0, 1), 0) * scale(1000, 1000, 1000);
		//Model = translate(0,0,1)  * scale(1, 1, 1);
		Model.identity();
		View = lookat(Vector<double>(1, 0, -2), Vector<double>(0, 0, 0), Vector<double>(0, 1, 0));
		Projection = setFrustum(70, SCREEN_WIDTH/ SCREEN_HEIGHT, 1, 10);
		//Viewport = viewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		//While application is running
		double timer = 0;

		double* zbuffer = new double[SCREEN_HEIGHT * SCREEN_WIDTH];
		while (!quit){
			timer+=0.02;
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
			
			for (int i = SCREEN_WIDTH * SCREEN_HEIGHT; i--; zbuffer[i] = 2);

			std::vector<Vector<double>> vertexBuffer =
			{
				 Vector<double>(-0.5f, -0.5f, -0.5f),
				 Vector<double>( 0.5f, -0.5f, -0.5f),
				 Vector<double>(-0.5f,   0.5f, -0.5f),
				 Vector<double>( 0.5f,   0.5f, -0.5f),
				 Vector<double>(-0.5f, -0.5f,   0.5f),
				 Vector<double>( 0.5f, -0.5f,   0.5f),
				 Vector<double>(-0.5f,   0.5f,   0.5f),
				 Vector<double>( 0.5f,   0.5f,   0.5f),
			};

			std::vector<Vector<int>> colorBuffer =
			{
				Vector<int>(255, 0, 0,255),
				Vector<int>(0, 255, 0,255),
				Vector<int>(0, 0, 255,255),
				Vector<int>(0, 255, 0,255),
				Vector<int>(255, 0, 0,255),
				Vector<int>(0, 255, 0,255),
				Vector<int>(0, 0, 255,255),
				Vector<int>(0, 255, 0,255),
			};

			static const int index_list[][3] = {
				 0, 2, 3,
				 0, 3, 1,

				 0, 4, 6,
				 0, 6, 2,
				 
				 0, 1, 5,
				 0, 5, 4,
				 
				 4, 5, 7,
				 4, 7, 6,
				 
				 1, 3, 7,
				 1, 7, 5,
				 
				 2, 6, 7,
				 2, 7, 3,
			};

			
			/*std::cout << "Model" << std::endl;
			Model.print();
			std::cout << "View" << std::endl;
			View.print();
			std::cout << "Projection" << std::endl;
			Projection.print();
			std::cout << "Viewport" << std::endl;
			Viewport.print();*/

			double radius = 2;
			double camX = sin(timer) * radius;
			double camZ = cos(timer) * radius;
			View = lookat(Vector<double>(camX, 1.0, camZ), Vector<double>(0, 0, 0), Vector<double>(0, 1, 0));

			std::vector<Vector<double>> faceVertexBuffer;
			std::vector<Vector<int>> faceColorBuffer;
			for (int i = 0; i < 12; ++i)          // 有12个面，循环12次
			{
				for (int j = 0; j < 3; j++) { //将面的三个顶点的信息存储起来
					faceVertexBuffer.push_back(vertexBuffer[index_list[i][j]]);
					faceColorBuffer.push_back(colorBuffer[index_list[i][j]]);
				}

				for (int k = 0; k < 3; k++) {
					Matrix<double> vecToMat = faceVertexBuffer[k].homogeneous().toMatrix();
					//std::cout << "vecToMat" << std::endl;
					//vecToMat.print();
					Matrix<double> MVP = Projection * View * Model;
					Matrix<double> afterChange = MVP * vecToMat;
					faceVertexBuffer[k] = afterChange.toVector();
					//std::cout << "afterChange" << std::endl;
					//faceVertexBuffer[k].print();

					faceVertexBuffer[k] = faceVertexBuffer[k] / faceVertexBuffer[k][3];//ndc坐标
					faceVertexBuffer[k] = viewport_transform(SCREEN_WIDTH,SCREEN_HEIGHT, faceVertexBuffer[k]);
					//std::cout << "view" << std::endl;
					//faceVertexBuffer[k].print();
				}

				drawTriangle2D(faceVertexBuffer, faceColorBuffer,zbuffer, gRenderer, gWindow);
				//drawTriangle2D(faceVertexBuffer, Vector<int>(255,255,255,255), gRenderer, gWindow);

				faceVertexBuffer.clear();
				faceColorBuffer.clear();
			}
			//Update screen
			SDL_RenderPresent(gRenderer);
		}
		delete[] zbuffer;
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
