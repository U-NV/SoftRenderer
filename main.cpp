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

Matrix ModelMatrix;
Matrix ViewMatrix;
Matrix ProjectionMatrix;
Matrix MVP;

Model* model;
Camera camera;

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
		camera.moveStraight(cameraSpeed);
	}
	if (keysEvent[SDLK_s]) {
		camera.moveStraight(-cameraSpeed);
	}
	if (keysEvent[SDLK_a]) {
		camera.moveTransverse(-cameraSpeed);
	}
	if (keysEvent[SDLK_d]) {
		camera.moveTransverse(cameraSpeed);
	}
	if (keysEvent[SDLK_e]) {
		camera.moveVertical(cameraSpeed);
	}
	if (keysEvent[SDLK_q]) {
		camera.moveVertical(-cameraSpeed);
	}

	if (rightKeyPress || leftKeyPress) {
		Vec2f offset = MousePosNow - MousePosPre;
		MousePosPre = MousePosNow;
		if (rightKeyPress) {
			camera.rotateCamera(offset * deltaTime);
		}
		if (leftKeyPress) {
			
		}
	}

	if (mouseWheelAmount != 0) {
		camera.changeFov(- mouseWheelAmount  * cameraSpeed);
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
	mat<2, 3, float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
	mat<3, 3, float> varying_nrm; // normal per vertex to be interpolated by FS
	//mat<3, 3, float> ndc_tri;
	mat<3, 3, float> varying_vert; // ��������ϵ����

	Vec4f varying_tri[3]; // triangle coordinates (clip coordinates), written by VS, read by FS


	virtual Vec4f vertex(int iface, int nthvert) {//�Ե�iface�ĵ�nthvert��������б任
		//���ģ��uv
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));

		//���߱任
		varying_nrm.set_col(nthvert, proj<3>(MVP.invert_transpose() * embed<4>(model->normal(iface, nthvert), 0.f)));

		//��������
		Vec4f temp = (ModelMatrix * embed<4>(model->vert(iface, nthvert)));
		varying_vert.set_col(nthvert, proj<3>(temp));

		//��������
		Vec4f gl_Vertex = (MVP * embed<4>(model->vert(iface, nthvert)));
		varying_tri[nthvert] = gl_Vertex;

		//��׼�豸����
		//ndc_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));

		//��ת��������귵��
		return gl_Vertex;
	}

	virtual bool fragment(Vec3f weights, TGAColor& color) {
		//��ֵuv
		Vec2f uv = varying_uv * weights;

		//��ֵ������
		//Vec3f normal = (varying_nrm * weights).normalize();
		Vec3f normal = model->normal(uv); //(varying_nrm * weights).normalize();

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

		//���ù���
		float ambient = 0.2f;
		float lightPower = 1.5;

		Vec3f vertPos = varying_vert * weights;
		Vec3f lightDir = (lightPos - vertPos);
		float dis = lightDir.norm();
		lightDir = lightDir / dis;

		Vec3f viewDir = (camera.camPos - vertPos).normalize();

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

		//ģ����Ϣ
		std::vector<std::string> modleName = {
			//"obj/african_head/african_head.obj",
			//"obj/african_head/african_head_eye_inner.obj",
			"obj/diablo3_pose/diablo3_pose.obj",
			"obj/floor.obj",
		};

		std::vector<Model> models;
		for (int m = 0; m < modleName.size(); m++) {
			models.push_back(Model(modleName[m]));
		}
		//�������
		camera = Camera((float)SCREEN_WIDTH / SCREEN_HEIGHT,0.1,100,60);

		//MVP������ز���
		Vec3f objPos(0, 0, 0);
		Vec3f up(0, 1, 0);
		Vec3f right(1, 0, 0);
		Vec3f forword(0, 0, 1);

		//zbuffer
		double* zbuffer = new double[SCREEN_HEIGHT * SCREEN_WIDTH];

		//֡������
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
			
			//����ִ�����ţ�������ת��������ƽ��
			//translate(0, 1, 0) * rotate(up, timer) * scale(1, 1, 1);
			//ModelMatrix = rotate(up, timer);
			//std::cout << ModelMatrix <<std::endl;
			//ModelMatrix= Matrix::identity();
			ViewMatrix = camera.getViewMatrix();
			ProjectionMatrix = camera.getProjMatrix();
			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

			//angle += 60 * deltaTime;
			//float radio = 2;
			//float lightX = radio * cos(angle * DegToRad);
			//float lightZ = radio * sin(angle * DegToRad);
			//lightPos.x = lightX;
			//lightPos.z = lightZ;


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
