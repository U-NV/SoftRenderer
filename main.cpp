#include "tgaimage.h"
#include "myVector.h"
#include "myGL.h"
#include "model.h"
#include "camera.h"

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
const int SCREEN_WIDTH = 400;
const int SCREEN_HEIGHT = 400;

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
Camera camera;

Vec3f lightPos(-1, 0.5, 0);

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
int getMouseKeyEven(void* opaque);

struct Shader : public IShader {
	mat<2, 3, float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
	mat<3, 3, float> varying_nrm; // normal per vertex to be interpolated by FS
	mat<3, 3, float> ndc_tri;
	mat<3, 3, float> varying_vert; // 世界坐标系坐标

	Vec4f varying_tri[3]; // triangle coordinates (clip coordinates), written by VS, read by FS


	virtual Vec4f vertex(int iface, int nthvert) {//对第iface的第nthvert个顶点进行变换
		//获得模型uv
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));

		//法线变换
		varying_nrm.set_col(nthvert, proj<3>(MVP.invert_transpose() * embed<4>(model->normal(iface, nthvert), 0.f)));

		//世界坐标
		Vec4f temp = (ModelMatrix * embed<4>(model->normal(iface, nthvert)));
		temp = temp / temp[3];
		varying_vert.set_col(nthvert, proj<3>(temp));

		//裁切坐标
		Vec4f gl_Vertex = (MVP * embed<4>(model->vert(iface, nthvert)));
		varying_tri[nthvert] = gl_Vertex;

		//ndc_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));

		//将转换后的坐标返回
		return gl_Vertex;
	}

	virtual bool fragment(Vec3f weights, TGAColor& color) {
		//插值uv
		Vec2f uv = varying_uv * weights;

		//插值法向量
		Vec3f normal = (varying_nrm * weights).normalize();


		Vec3f vertPos = varying_vert * weights;

		Vec3f lightDir = (lightPos - vertPos).normalize();
		Vec3f viewDir = (camera.camPos - vertPos).normalize();

		Vec3f hafe = (lightDir + viewDir).normalize();

		float diff = std::max(0.f, lightPos * normal);
		float spec = pow((normal * hafe), model->specular(uv));

		TGAColor c = model->diffuse(uv);
		color = c;
		for (int i = 0; i < 3; i++) {
			float rate = std::max(0.1f, 2 * diff + spec);

			color[i] = std::min<float>(5 + c[i] * rate, 255);
		}
		return false;
	}
};

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

		//SDL_Event e;

		//模型信息
		std::vector<std::string> modleName = {
			"obj/african_head/african_head.obj",
			"obj/african_head/african_head_eye_inner.obj",
			//"obj/african_head/african_head_eye_outer.obj",
		};

		std::vector<Model> models;
		for (int m = 0; m < modleName.size(); m++) {
			models.push_back(Model(modleName[m]));
		}


		camera = Camera((float)SCREEN_WIDTH / SCREEN_HEIGHT,0.1,100,60,0.01);

		//MVP矩阵相关参数
		Vec3f objPos(0, 0, 0);
		Vec3f up(0, 1, 0);
		Vec3f right(1, 0, 0);
		Vec3f forword(0, 0, 1);
		//ProjectionMatrix = setFrustum(70, SCREEN_WIDTH / SCREEN_HEIGHT, 1, 100);
		//ViewMatrix = lookat(camPos, objPos, up);

		//zbuffer
		double* zbuffer = new double[SCREEN_HEIGHT * SCREEN_WIDTH];

		//帧计数器
		double timer = 0;

		//While application is running
		while (!quit){
			//Handle events on queue
			getMouseKeyEven(NULL);

			//Clear screen
			SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0xFF);
			SDL_RenderClear(gRenderer);
			getFPS();

			std::fill(zbuffer, zbuffer + SCREEN_WIDTH * SCREEN_HEIGHT, 1);

			timer += 1;
			//首先执行缩放，接着旋转，最后才是平移
			//translate(0, 1, 0) * rotate(up, timer) * scale(1, 1, 1);
			//ModelMatrix = rotate(right,90) * scale(0.1, 0.1, 0.1);
			//std::cout << ModelMatrix <<std::endl;
			//ModelMatrix= Matrix::identity();

			ViewMatrix = camera.getViewMatrix();
			ProjectionMatrix = camera.getProjMatrix();
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



int getMouseKeyEven(void* opaque) {
	SDL_Event ev;
	int quit = 0;
	//while (!quit) 
	{
		while (SDL_PollEvent(&ev))
		{
			if (SDL_KEYDOWN == ev.type) // SDL_KEYUP
			{
				if (SDLK_DOWN == ev.key.keysym.sym)
				{

					camera.moveBackward();
					printf("SDLK_DOWN ...............\n");

				}
				else if (SDLK_UP == ev.key.keysym.sym)
				{
					camera.moveForward();
					printf("SDLK_UP ...............\n");

				}
				else if (SDLK_LEFT == ev.key.keysym.sym)
				{
					camera.moveLeft();
					printf("SDLK_LEFT ...............\n");

				}
				else if (SDLK_RIGHT == ev.key.keysym.sym)
				{
					camera.moveRight();
					printf("SDLK_RIGHT ...............\n");
				}
			}
			else if (SDL_MOUSEBUTTONDOWN == ev.type)
			{
				if (SDL_BUTTON_LEFT == ev.button.button)
				{
					int px = ev.button.x;
					int py = ev.button.y;
					printf("x, y %d %d ...............\n", px, py);

				}
				else if (SDL_BUTTON_RIGHT == ev.button.button)
				{
					int px = ev.button.x;
					int py = ev.button.y;
					printf("x, y %d %d ...............\n", px, py);
				}
			}
			else if (SDL_MOUSEMOTION == ev.type)
			{
				int px = ev.motion.x;
				int py = ev.motion.y;

				printf("x, y %d %d ...............\n", px, py);
			}
			else if (SDL_QUIT == ev.type)
			{
				printf("SDL_QUIT ...............\n");
				return 0;
			}
		}
	}

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
		gWindow = SDL_CreateWindow("SoftRenderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
