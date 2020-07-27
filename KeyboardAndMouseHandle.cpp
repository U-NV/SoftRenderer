#include "KeyboardAndMouseHandle.h"

void KeyboardAndMouseHandle::handlerKeyboardEvent(float deltaTime)
{
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

		double sensitivity = 10;
		offset.x *= sensitivity;
		offset.y *= sensitivity;

		if (rightKeyPress) {
			defaultCamera.rotateCamera(offset * deltaTime);
		}
		if (leftKeyPress) {

		}
	}

	if (mouseWheelAmount != 0) {
		defaultCamera.changeFov(-mouseWheelAmount * cameraSpeed);
		mouseWheelAmount = 0;
	}
}

int KeyboardAndMouseHandle::getMouseKeyEven(void* opaque, float deltaTime)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type) {
		case SDL_KEYDOWN:
			if (event.key.keysym.sym < 1024)
				keysEvent[event.key.keysym.sym] = true;
			break;
		case SDL_KEYUP:
			if (event.key.keysym.sym < 1024)
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

	handlerKeyboardEvent(deltaTime);
	return 0;
}