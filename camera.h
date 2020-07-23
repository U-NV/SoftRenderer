#pragma once
#ifndef __CAMERA__
#define __CAMERA__
#include"myVector.h"
#include"myGL.h"

class  Camera
{
public:
	float NEAR;
	float FAR;
	float FOVY;
	float aspect;
	float cameraSpeed;

	Vec3f camUp = { 0, 1, 0 };
	Vec3f camPos = { 0, 0, 5 };
	Vec3f camDir = { 0, 0, -1 };
	Vec3f targetPos = { 0, 0, -1 };
	//Vec3f targetPos = { 0, 0, 0 };

	Matrix ViewMatrix;
	Matrix ProjectionMatrix;

	Camera();
	Camera(float screenAspect, float near, float far, float fovy, float speed);
	~ Camera();

	inline void setViewMatrix();

	void moveForward();
	void moveBackward();
	void moveLeft();
	void moveRight();

	//void moveUp();
	//void moveDown();
	//
	Matrix getViewMatrix();
	Matrix getProjMatrix();
};

#endif