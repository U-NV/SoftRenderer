#include "KeyboardAndMouseHandle.h"

KeyboardAndMouseHandle::KeyboardAndMouseHandle(int SCREEN_WIDTH, int SCREEN_HEIGHT, Camera* camera, float* gamma, float* exposure)
{
	MousePosNow = Vec2f((float)SCREEN_WIDTH / 2.0f, (float)SCREEN_HEIGHT / 2.0f);
	MousePosNow = Vec2f((float)SCREEN_WIDTH / 2.0f, (float)SCREEN_HEIGHT / 2.0f);
	controlCamera = camera;
	this->gamma = gamma;
	this->exposure = exposure;
}

void KeyboardAndMouseHandle::handlerKeyboardEvent(float deltaTime)
{
	float cameraSpeed = 2.0f * deltaTime;
	if (keysEvent[SDLK_ESCAPE]) {
		SDL_Runing = false;
	}
	if (keysEvent[SDLK_w]) {
		controlCamera->moveStraight(cameraSpeed);
	}
	if (keysEvent[SDLK_s]) {
		controlCamera->moveStraight(-cameraSpeed);
	}
	if (keysEvent[SDLK_a]) {
		controlCamera->moveTransverse(-cameraSpeed);
	}
	if (keysEvent[SDLK_d]) {
		controlCamera->moveTransverse(cameraSpeed);
	}
	if (keysEvent[SDLK_e]) {
		controlCamera->moveVertical(cameraSpeed);
	}
	if (keysEvent[SDLK_q]) {
		controlCamera->moveVertical(-cameraSpeed);
	}
	
	if (keysEvent[SDLK_COMMA]) { //<
		*gamma -= 0.05f;
		std::cout << "Gamma:" << *gamma << std::endl;
	}if (keysEvent[SDLK_PERIOD]) {//>
		*gamma += 0.05f;
		std::cout << "Gamma:" << *gamma << std::endl;
	}
	
	if (keysEvent[SDLK_LEFTBRACKET]) { //"["
		*exposure -= 0.05f;
		std::cout << "exposure:" << *exposure << std::endl;
	}if (keysEvent[SDLK_RIGHTBRACKET]) {//']'
		*exposure += 0.05f;
		std::cout << "exposure:" << *exposure << std::endl;
	}

	if (rightKeyPress || leftKeyPress) {
		Vec2f offset = MousePosNow - MousePosPre;
		MousePosPre = MousePosNow;

		float sensitivity = 10;
		offset.x *= sensitivity;
		offset.y *= sensitivity;

		if (rightKeyPress) {
			controlCamera->rotateCamera(offset * deltaTime);
		}
		if (leftKeyPress) {

		}
	}

	if (mouseWheelAmount != 0) {
		controlCamera->changeFov(-mouseWheelAmount * cameraSpeed);
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
			MousePosNow.x = (float)event.motion.x;
			MousePosNow.y = SCREEN_HEIGHT - (float)event.motion.y;
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