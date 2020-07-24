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

	float yaw = 270;
	float pitch =0;

	Vec3f camUp = { 0, 1, 0 };
	Vec3f camPos = { 0, 0, 2 };
	Vec3f camDir = { 0, 0, -1 };
	Vec3f targetPos = { 0, 0, -1 };
	//Vec3f targetPos = { 0, 0, 0 };

	Matrix ViewMatrix;
	Matrix ProjectionMatrix;

	Camera();
	Camera(float screenAspect, float near, float far, float fovy);
	~ Camera();

	inline void setViewMatrix();

	void moveStraight(float amount); //前后移动
	void moveTransverse(float amount); //左右移动
	void moveVertical(float amount); //上下移动
	void rotateCamera(Vec2f offset);  // yaw方法
	void changeFov(float amount);  // yaw方法

	Matrix getViewMatrix();
	Matrix getProjMatrix();
};


extern Camera defaultCamera;

#endif