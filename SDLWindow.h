#pragma once

#include "SDL.h"
#include "SDL_revision.h"
#include "myGL.h"

class SDLWindow
{
	unsigned int width;
	unsigned int height;
	//The window we'll be rendering to
	SDL_Window* gWindow = NULL;
	//The window renderer
	SDL_Renderer* gRenderer = NULL;
	//初始换SDL并创建Window和renderer
	inline void init(const char* name, const int SCREEN_WIDTH, const int SCREEN_HEIGHT);
	//inline void drawPixel(int& x, int& y, const int& width, const int& height, Frame* drawBuffer);
public:
	bool initSuccess;
	//构造函数调用init
	SDLWindow(const char* name,const int SCREEN_WIDTH, const int SCREEN_HEIGHT);
	//关闭SDL窗口
	void close();
	//更新窗口内容
	void refresh(Frame* buffer);
	//在窗口中绘制像素
	
};

