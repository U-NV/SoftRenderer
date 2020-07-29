#pragma once

#include "SDL.h"
#include "SDL_revision.h"
#include "myVector.h"

class SDLWindow
{
	unsigned int width;
	unsigned int height;
	//The window we'll be rendering to
	SDL_Window* gWindow = NULL;
	//The window renderer
	SDL_Renderer* gRenderer = NULL;
	//��ʼ��SDL������Window��renderer
	inline void init(const char* name, const int SCREEN_WIDTH, const int SCREEN_HEIGHT);
	inline void drawPixel(int& x, int& y, const int& width, const int& height, ColorVec* drawBuffer);
public:
	bool initSuccess;
	//���캯������init
	SDLWindow(const char* name,const int SCREEN_WIDTH, const int SCREEN_HEIGHT);
	//�ر�SDL����
	void close();
	//���´�������
	void refresh(ColorVec* buffer);
	//�ڴ����л�������
	
};

