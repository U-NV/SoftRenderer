#include "tgaimage.h"
#include "myVector.h"
#include "myGL.h"
#include "model.h"

#include <vector>
#include <limits>
#include <cmath>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>

#include <time.h>

Vector<int> white(255, 255, 255, 255);
TGAColor red   = TGAColor(255, 0,   0,   255);

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

Matrix<double> ModelMatrix;
Matrix<double> ViewMatrix;
Matrix<double> ProjectionMatrix;

Model* model;

struct Shader : public IShader {
	std::vector<Vector<double>> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
	std::vector<Vector<double>> varying_tri; // triangle coordinates (clip coordinates), written by VS, read by FS
	std::vector<Vector<double>> varying_nrm; // normal per vertex to be interpolated by FS
	std::vector<Vector<double>> ndc_tri;     // triangle in normalized device coordinates

	virtual Vector<double> vertex(int iface, int nthvert) {//对第iface的第nthvert个顶点进行变换
		//获得模型uv
		varying_uv.resize(3);
		varying_uv[nthvert] = (model->uv(iface, nthvert));
		//计算MVP矩阵
		Matrix<double> MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		//法线变换
		varying_nrm.resize(3);
		varying_nrm[nthvert] = (MVP * model->normal(iface, nthvert).homogeneous().toMatrix()).toVector();
		//坐标变换
		varying_tri.resize(3);
		Vector<double> gl_Vertex = (MVP * model->vert(iface, nthvert).homogeneous().toMatrix()).toVector();
		varying_tri[nthvert] = gl_Vertex;
		////转换到标准设备坐标系
		//ndc_tri.resize(3);
		//ndc_tri[nthvert] = gl_Vertex / gl_Vertex[3];
		//将转换后的坐标返回
		return gl_Vertex;
	}

	virtual bool fragment(Vector<double> weights, TGAColor& color) {
		////插值uv
		Vector<double> uv = interpolate_uv(varying_uv,weights);

		////计算法向
		//Vector<double> n = (B * model->normal(uv)).normalize();

		////计算光照
		//float diff = std::max(0.f, n * light_dir);
		//color = model->diffuse(uv) * diff;

		color = model->diffuse(uv);
		return false;
	}
};



void getFPS() {
	static float framesPerSecond = 0.0f;
	static int fps = 0;
	static float lastTime = 0.0f;
	clock_t  currentTime = clock();
	++framesPerSecond;
	std::cout << "Current Frames Per Second:" << fps << std::endl;
	if ((currentTime - lastTime) / CLOCKS_PER_SEC >= 1.0f)
	{
		lastTime = currentTime;
		fps = (int)framesPerSecond;
		framesPerSecond = 0;
	}
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
		//While application is running
		double timer = 0;
		double* zbuffer = new double[SCREEN_HEIGHT * SCREEN_WIDTH];

		std::vector<const char*> modleName = {
			"obj/african_head/african_head.obj",
			"obj/african_head/african_head_eye_inner.obj",
			//"obj/african_head/african_head_eye_outer.obj",
		};
		std::vector<Model> models;
		for (int m = 0; m < modleName.size(); m++) {
			models.push_back(Model(modleName[m]));
		}

		
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
			getFPS();

			timer += 0.02;
			for (int i = SCREEN_WIDTH * SCREEN_HEIGHT; i--; zbuffer[i] = 2);
			//首先执行缩放，接着旋转，最后才是平移
			//Model = translate(0,0,0.001) * rotate(Vector<double>(0, 0, 1), 0) * scale(1000, 1000, 1000);
			//Model = translate(0,0,1)  * scale(1, 1, 1);
			ModelMatrix.identity();

			double radius = 5;
			double camX = sin(timer) * radius;
			double camZ = cos(timer) * radius;
			ViewMatrix = lookat(Vector<double>(camX, 1.0, camZ), Vector<double>(0, 0, 0), Vector<double>(0, 1, 0));
			//ViewMatrix = lookat(Vector<double>(1, 0, -2), Vector<double>(0, 0, 0), Vector<double>(0, 1, 0));

			ProjectionMatrix = setFrustum(70, SCREEN_WIDTH / SCREEN_HEIGHT, 1, 10);
			
			for (int m = 0; m < models.size(); m++) {
				Shader shader;
				model = &models[m];
				for (int i = 0; i < model->nfaces(); i++) {
					for (int j = 0; j < 3; j++) {
						shader.vertex(i, j);
					}
					drawTriangle2D(shader.varying_tri,shader, zbuffer,gRenderer,gWindow);

					//draw2DFrame(shader.varying_tri, white, gRenderer, gWindow);
				}
				
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
	ViewMatrix.resize(4, 4);
	ModelMatrix.resize(4, 4);
	ViewMatrix.identity();
	ModelMatrix.identity();
	ProjectionMatrix.resize(4, 4);
	ProjectionMatrix.identity();

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
