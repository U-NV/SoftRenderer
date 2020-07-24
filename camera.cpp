#include"camera.h"

Camera defaultCamera;
Camera::Camera()
{
}

inline void Camera::setViewMatrix() {
	targetPos = camPos + camDir;
	ViewMatrix = lookat(camPos, targetPos, camUp);
}

Camera::Camera(float screenAspect,float near = 0.1f,float far = 20,float fovy = 60)
{
	NEAR = near;
	FAR = far;
	FOVY = fovy;
	aspect = screenAspect;
	ProjectionMatrix = setFrustum(FOVY * DegToRad, aspect, NEAR, FAR);
	setViewMatrix();
}
	

Camera::~Camera()
{
}



void Camera::moveStraight(float amount)
{
	camPos = camPos + camDir * amount;
	setViewMatrix();
}

void Camera::moveTransverse(float amount)
{
	camPos = camPos + (cross(camDir, camUp)).normalize() * amount;
	setViewMatrix();
}

void Camera::moveVertical(float amount)
{
	camPos = camPos + camUp.normalize() * amount;
	setViewMatrix();
}

void Camera::changeFov(float amount)
{
	FOVY += amount;
	FOVY = std::max(0.0f, std::min(90.0f, FOVY));
	ProjectionMatrix = setFrustum(FOVY * DegToRad, aspect, NEAR, FAR);
}

void Camera::rotateCamera(Vec2f offset)
{
	yaw += offset.x;
	pitch += offset.y;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	camDir.x = cos(pitch * DegToRad) * cos(yaw * DegToRad);
	camDir.y = sin(pitch * DegToRad);
	camDir.z = cos(pitch * DegToRad) * sin(yaw * DegToRad);

	camDir.normalize();

	//std::cout << camDir << std::endl;
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
