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

TGAColor white(255, 255, 255, 255);
TGAColor red   = TGAColor(255, 0,   0,   255);

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

Matrix ModelMatrix;
Matrix ViewMatrix;
Matrix ProjectionMatrix;
Matrix MVP;

Model* model;

struct Shader : public IShader {
	mat<2, 3, float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
	Vec4f varying_tri[3]; // triangle coordinates (clip coordinates), written by VS, read by FS
	Vec3f varying_nrm[3]; // normal per vertex to be interpolated by FS
	Vec3f ndc_tri[3];     // triangle in normalized device coordinates

	virtual Vec4f vertex(int iface, int nthvert) {//对第iface的第nthvert个顶点进行变换
		//获得模型uv
		//varying_uv[nthvert] = (model->uv(iface, nthvert));
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		//计算MVP矩阵
		
		//法线变换
		//varying_nrm[nthvert] = (MVP * model->normal(iface, nthvert).homogeneous().toMatrix()).toVector();
		//varying_nrm[nthvert] = (MVP * model->normal(iface, nthvert).homogeneous().toMatrix()).toVector();
		//坐标变换
		//Vec4f gl_Vertex = (MVP * model->vert(iface, nthvert).homogeneous());
		Vec3f vert = model->vert(iface, nthvert);
		
		
		Vec4f gl_Vertex = (MVP * embed<4>(model->vert(iface, nthvert)));
		//std::cout<<"after:" << gl_Vertex << std::endl;

		varying_tri[nthvert] = gl_Vertex;
		////转换到标准设备坐标系
		//ndc_tri.resize(3);
		//ndc_tri[nthvert] = gl_Vertex / gl_Vertex[3];
		//将转换后的坐标返回
		return gl_Vertex;
	}

	virtual bool fragment(Vec3f weights, TGAColor& color) {
		////插值uv
		//Vec2f uv = interpolate_uv(varying_uv,weights);
		Vec2f uv = varying_uv * weights;

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
	if ((currentTime - lastTime) / CLOCKS_PER_SEC >= 1.0f)
	{
		std::cout << "Current Frames Per Second:" << fps << std::endl;
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

		Vec3f camPos(0, 0.0, 0);
		Vec3f objPos(0, 0, 0);
		Vec3f up(0, 1, 0);

		

		ProjectionMatrix = setFrustum(70, SCREEN_WIDTH / SCREEN_HEIGHT, 1, 10);

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

			std::fill(zbuffer, zbuffer + SCREEN_WIDTH * SCREEN_HEIGHT, 1);

			//首先执行缩放，接着旋转，最后才是平移
			//Model = translate(0,0,0.001) * rotate(Vector<double>(0, 0, 1), 0) * scale(1000, 1000, 1000);
			//Model = translate(0,0,1)  * scale(1, 1, 1);
			
			timer += 0.02;
			ModelMatrix= Matrix::identity();
			double radius = 3;
			double camX = sin(timer) * radius;
			double camZ = cos(timer) * radius;
			camPos[0] = camX;
			camPos[2] = camZ;
			ViewMatrix = lookat(camPos, objPos, up);

			//std::cout << "M:" << ModelMatrix << std::endl;
			//std::cout << "V:" << ViewMatrix << std::endl;
			//std::cout << "P:" << ProjectionMatrix << std::endl;
			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
			

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
	//Matrix a = translate(1, 1, 1);
	//std::cout << "translate:" << a << std::endl;

	//Vec3f b(1, 0, 0);
	//Vec3f b1(0, 1, 0);


	//Vec4f b4 = embed<4>(b);
	//std::cout << "b:" << b4 << std::endl;

	//Vec3f c = cross(b, b1);
	//std::cout << "result:" << c << std::endl;



	ViewMatrix= Matrix::identity();
	ModelMatrix= Matrix::identity();
	ProjectionMatrix= Matrix::identity();

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
