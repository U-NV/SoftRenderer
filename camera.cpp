#include"camera.h"

Camera::Camera()
{
	ProjectionMode = true;
}

inline void Camera::setViewMatrix() {
	camTargetPos = camPos + camDir;
	ViewMatrix = lookat(camPos, camTargetPos, camUp);
}

void Camera::setCamera(Vec3f& pos, Vec3f& targetPos, Vec3f& up)
{
	camPos = pos;
	camTargetPos = targetPos;
	camUp = up;
	ViewMatrix = lookat(camPos, camTargetPos, camUp);
}


Camera::Camera(ViewPort* targetViewPort,float near = 0.1f,float far = 20,float fovy = 60)
{
	NEAR = near;
	FAR = far;
	FOVY = fovy;
	this->targetViewPort = targetViewPort;
	aspect = float(targetViewPort->v_width) / targetViewPort->v_height;
	ProjectionMode = true;
	if(ProjectionMode)
		ProjectionMatrix = setFrustum(FOVY * DegToRad, aspect, NEAR, FAR);
	else
		ProjectionMatrix = mat4_orthographic(6,4, NEAR, FAR);
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
	Vec3f x = cross(camUp, camDir).normalize();
	Vec3f y = cross(camDir, x).normalize();
	camPos = camPos + y * amount;
	setViewMatrix();
}

void Camera::changeFov(float amount)
{
	FOVY += amount;
	FOVY = std::max(0.0f, std::min(90.0f, FOVY));
	if (ProjectionMode)
		ProjectionMatrix = setFrustum(FOVY * DegToRad, aspect, NEAR, FAR);
	else
		ProjectionMatrix = mat4_orthographic(6, 4, NEAR, FAR);
}

void Camera::setFov(float Fov)
{
	FOVY = Fov;
	if (ProjectionMode)
		ProjectionMatrix = setFrustum(FOVY * DegToRad, aspect, NEAR, FAR);
	else
		ProjectionMatrix = mat4_orthographic(6, 4, NEAR, FAR);
}

void Camera::changeViewPort(ViewPort* newViewPort)
{
	targetViewPort = newViewPort;
	aspect = newViewPort->v_width / newViewPort->v_height;
	ProjectionMatrix = setFrustum(FOVY * DegToRad, aspect, NEAR, FAR);
}


void Camera::setClipPlane(float near,float far)
{
	NEAR = near;
	FAR = far;
	if (ProjectionMode)
		ProjectionMatrix = setFrustum(FOVY * DegToRad, aspect, NEAR, FAR);
	else
		ProjectionMatrix = mat4_orthographic(6, 4, NEAR, FAR);
}

Vec3f Camera::getPos()
{
	return camPos;
}

float Camera::getNear() {
	return NEAR;
}

float Camera::getFar()
{
	return FAR;
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

bool Camera::getProjectMode()
{
	return ProjectionMode;
}

void Camera::setProjectMode(bool flag)
{
	ProjectionMode = flag;
	if (ProjectionMode)
		ProjectionMatrix = setFrustum(FOVY * DegToRad, aspect, NEAR, FAR);
	else
		ProjectionMatrix = mat4_orthographic(6, 4, NEAR, FAR);
}
