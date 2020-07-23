#include"camera.h"


Camera::Camera()
{
}

inline void Camera::setViewMatrix() {
	targetPos = camPos + camDir;
	ViewMatrix = lookat(camPos, targetPos, camUp);
}

Camera::Camera(float screenAspect,float near = 0.1f,float far = 20,float fovy = 60,float speed = 1.0f)
{
	NEAR = near;
	FAR = far;
	FOVY = fovy;
	aspect = screenAspect;
	cameraSpeed = speed;
	ProjectionMatrix = setFrustum(70, aspect, 1, 100);
	setViewMatrix();
}

Camera::~Camera()
{
}



void Camera::moveForward()
{
	camPos = camPos + camDir* cameraSpeed;
	setViewMatrix();
}

void Camera::moveBackward()
{
	camPos = camPos - camDir * cameraSpeed;
	setViewMatrix();
}

void Camera::moveLeft()
{
	camPos = camPos - (cross(camDir, camUp)).normalize() * cameraSpeed;
	setViewMatrix();
}

void Camera::moveRight()
{
	camPos = camPos + (cross(camDir, camUp)).normalize() * cameraSpeed;
	setViewMatrix();
}

Matrix Camera::getViewMatrix()
{
	return ViewMatrix;
}

Matrix Camera::getProjMatrix()
{
	return ProjectionMatrix;
}
