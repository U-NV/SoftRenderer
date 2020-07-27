#pragma once

#include "SDL.h"
#include "SDL_revision.h"
#include "myVector.h"
#include "camera.h"

class KeyboardAndMouseHandle
{
private:
	bool keysEvent[1024] = { false };
	bool mouseEvent[2] = { false };
	bool rightKeyPress = false;
	bool leftKeyPress = false;
	int mouseWheelAmount = 0;
	int SCREEN_WIDTH = 0;
	int SCREEN_HEIGHT = 0;

	Vec2f MousePosNow;
	Vec2f MousePosPre;
public:
	bool SDL_Runing;
	KeyboardAndMouseHandle(int SCREEN_WIDTH,int SCREEN_HEIGHT) {

		MousePosNow = Vec2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
		MousePosNow = Vec2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
	}

	void handlerKeyboardEvent(float deltaTime);
	int getMouseKeyEven(void* opaque, float deltaTime);
};

