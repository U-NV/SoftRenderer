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

bool SDL_Runing = false;

//Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

//Starts up SDL and creates window
bool init();

//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

Model* model;

Vec3f lightPos(1, 0.5,1);

clock_t lastFrame = 0;
float deltaTime = 0;

bool keysEvent[1024] = {false};
bool mouseEvent[2] = {false};
bool rightKeyPress = false;
bool leftKeyPress = false;
int mouseWheelAmount = 0;
Vec2f MousePosNow(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
Vec2f MousePosPre(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
inline void handlerKeyboardEvent() {
	float cameraSpeed = 2.0f * deltaTime;
	if (keysEvent[SDLK_ESCAPE]) {
		SDL_Runing = false;
	}
	if (keysEvent[SDLK_w]) {
		defaultCamera.moveStraight(cameraSpeed);
	}
	if (keysEvent[SDLK_s]) {
		defaultCamera.moveStraight(-cameraSpeed);
	}
	if (keysEvent[SDLK_a]) {
		defaultCamera.moveTransverse(-cameraSpeed);
	}
	if (keysEvent[SDLK_d]) {
		defaultCamera.moveTransverse(cameraSpeed);
	}
	if (keysEvent[SDLK_e]) {
		defaultCamera.moveVertical(cameraSpeed);
	}
	if (keysEvent[SDLK_q]) {
		defaultCamera.moveVertical(-cameraSpeed);
	}

	if (rightKeyPress || leftKeyPress) {
		Vec2f offset = MousePosNow - MousePosPre;
		MousePosPre = MousePosNow;
		if (rightKeyPress) {
			defaultCamera.rotateCamera(offset * deltaTime);
		}
		if (leftKeyPress) {
			
		}
	}

	if (mouseWheelAmount != 0) {
		defaultCamera.changeFov(- mouseWheelAmount  * cameraSpeed);
		mouseWheelAmount = 0;
	}
}
int getMouseKeyEven(void* opaque) {
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type) {
			case SDL_KEYDOWN:
				if(event.key.keysym.sym < 1024)
					keysEvent[event.key.keysym.sym] = true;
				break;
			case SDL_KEYUP:
				if(event.key.keysym.sym < 1024)
					keysEvent[event.key.keysym.sym] = false;
				break;
			case SDL_MOUSEMOTION:
				MousePosNow.x = event.motion.x;
				MousePosNow.y = SCREEN_HEIGHT - event.motion.y;
				//printf("x, y %d %d ...............\n", MousePosNow.x, MousePosNow.y);
				break;
			case SDL_MOUSEWHEEL:
				mouseWheelAmount = event.wheel.y;
				break;
			case SDL_MOUSEBUTTONDOWN:
				switch (event.button.button)
				{
					case SDL_BUTTON_LEFT:
						leftKeyPress = true;
						break;
					case SDL_BUTTON_RIGHT:
						MousePosPre = MousePosNow;
						rightKeyPress = true;
						break;
					default:
						break;
				}
				break;
			case SDL_MOUSEBUTTONUP:
				switch (event.button.button)
				{
					case SDL_BUTTON_LEFT:
						leftKeyPress = false;
						break;
					case SDL_BUTTON_RIGHT:
						rightKeyPress = false;
						break;
					default:
						break;
				}
				break;
			case SDL_QUIT:
				SDL_Runing = false;
				break;
			default:
				break;
		}
	}

	handlerKeyboardEvent();
	return 0;
}

struct Shader : public IShader {
	virtual void vertex(int iface, int nthvert, VerInf& faceVer) {//对第iface的第nthvert个顶点进行变换
		//获得模型uv
		faceVer.uv = model->uv(iface, nthvert);
		//法线变换
		faceVer.normal = proj<3>(MVP.invert_transpose() * embed<4>(model->normal(iface, nthvert), 0.f));
		//世界坐标
		faceVer.world_pos = proj<3>(ModelMatrix * embed<4>(model->vert(iface, nthvert)));
		//裁切坐标
		faceVer.clip_coord = MVP * embed<4>(model->vert(iface, nthvert));
	}

	virtual bool fragment(VerInf verInf, TGAColor& color) {
		//插值uv
		Vec2f uv = verInf.uv;

		//插值法向量
		//Vec3f normal = verInf.normal;
		Vec3f normal = model->normal(uv); 

		//Vec3f bn = (varying_nrm * weights).normalize();

		//mat<3, 3, float> A;
		//A[0] = ndc_tri.col(1) - ndc_tri.col(0);
		//A[1] = ndc_tri.col(2) - ndc_tri.col(0);
		//A[2] = bn;

		//mat<3, 3, float> AI = A.invert();

		//Vec3f i = AI * Vec3f(varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0);
		//Vec3f j = AI * Vec3f(varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0);

		//mat<3, 3, float> B;
		//B.set_col(0, i.normalize());
		//B.set_col(1, j.normalize());
		//B.set_col(2, bn);

		//Vec3f normal = (B * model->normal(uv)).normalize();

		//设置光照
		float ambient = 0.2f;
		float lightPower = 1.5;

		Vec3f vertPos = verInf.world_pos;
		Vec3f lightDir = (lightPos - vertPos);
		float dis = lightDir.norm();
		lightDir = lightDir / dis;

		Vec3f viewDir = (defaultCamera.camPos - vertPos).normalize();

		Vec3f hafe = (lightDir + viewDir).normalize();

		float diff = clamp<float>(std::abs(lightDir * normal), 0, 1);
		float spec = clamp<float>(std::abs(normal * hafe), 0, 1);
		float uvspecular = clamp<float>((float)model->specular(uv)/100.0f, 0, 1);

		TGAColor c = model->diffuse(uv);
		color = c;
		for (int i = 0; i < 3; i++) {
			float rate = clamp<float>(ambient + (1 * diff + 1 * spec + 1*uvspecular)/dis / dis * lightPower,0,10);
			color[i] = clamp<int>((float)c[i] * rate,0,255);
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
		SDL_Runing = true;

		//模型信息
		std::vector<std::string> modleName = {
			"obj/african_head/african_head.obj",
			"obj/african_head/african_head_eye_inner.obj",
			//"obj/diablo3_pose/diablo3_pose.obj",
			"obj/floor.obj",
		};

		std::vector<Model> models;
		for (int m = 0; m < modleName.size(); m++) {
			models.push_back(Model(modleName[m]));
		}
		//创建相机
		defaultCamera = Camera((float)SCREEN_WIDTH / SCREEN_HEIGHT,0.1,100,60);

		//MVP矩阵相关参数
		Vec3f objPos(0, 0, 0);
		Vec3f up(0, 1, 0);
		Vec3f right(1, 0, 0);
		Vec3f forword(0, 0, 1);

		//zbuffer
		double* zbuffer = new double[SCREEN_HEIGHT * SCREEN_WIDTH];

		//帧计数器
		int timer = 0;
		float angle = 0;
		//While application is running
		while (SDL_Runing){
			timer++;
			
			//Handle events on queue
			getMouseKeyEven(NULL);

			//Clear screen
			SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0xFF);
			SDL_RenderClear(gRenderer);
			
			//get delta time
			clock_t currentFrame = clock();
			deltaTime = float((float)currentFrame - lastFrame)/ CLOCKS_PER_SEC;
			lastFrame = currentFrame;

			if (timer >= 1 / deltaTime) {
				std::cout << "Current Frames Per Second:" << 1 / deltaTime << std::endl;
				timer = 0;
			}

			std::fill(zbuffer, zbuffer + SCREEN_WIDTH * SCREEN_HEIGHT, 1);
			
			//首先执行缩放，接着旋转，最后才是平移
			//translate(0, 1, 0) * rotate(up, timer) * scale(1, 1, 1);
			//ModelMatrix = rotate(up, timer);
			//std::cout << ModelMatrix <<std::endl;
			//ModelMatrix= Matrix::identity();
			ViewMatrix = defaultCamera.getViewMatrix();
			ProjectionMatrix = defaultCamera.getProjMatrix();
			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

			angle += 60 * deltaTime;
			float radio = 2;
			float lightX = radio * cos(angle * DegToRad);
			float lightZ = radio * sin(angle * DegToRad);
			lightPos.x = lightX;
			lightPos.z = lightZ;

			for (int m = 0; m < models.size(); m++) {
				Shader shader;
				VerInf faceVer[3];
				model = &models[m];
				for (int i = 0; i < model->nfaces(); i++) {
					for (int j = 0; j < 3; j++) {
						 shader.vertex(i, j, faceVer[j]);
					}
					triangle(faceVer,shader,zbuffer,gRenderer,gWindow);
					//drawTriangle2D(shader.varying_tri,shader, zbuffer,gRenderer,gWindow);
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


void drawWindowTGA() {
	int imageH = 256;
	int imageW = 256;
	int midH = imageH / 2;
	int midW = imageW / 2;
	int lineW = 10;
	int halfLineW = lineW/2;
	TGAImage frame(imageW, imageH, TGAImage::RGB);

	TGAColor frameColor(100, 80, 100, 255);
	TGAColor windowColor(0, 200, 255, 128);
	
	for (int i = 0; i < imageH; i++) {
		for (int j = 0; j < imageW; j++) {
			if ((i < lineW || i >= imageH - lineW)
				|| (j < lineW || j >= imageW - lineW)
				|| (i > midH - halfLineW && i <= midH + halfLineW)
				|| (j > midW - halfLineW && j <= midW + halfLineW)) 
			{
				frame.set(i, j, frameColor);
			}
			else {
				frame.set(i, j, windowColor);
			}
			
		}
	}
	frame.flip_vertically(); // to place the origin in the bottom left corner of the image
	frame.write_tga_file("alpha.tga");
}

bool init()
{
	//drawWindowTGA();
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
