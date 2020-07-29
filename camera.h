#pragma once
#ifndef __CAMERA__
#define __CAMERA__
#include"myVector.h"
#include"myGL.h"

class  Camera
{
private:
	float NEAR;
	float FAR;
	float FOVY;
	float aspect;

	float yaw = 270;
	float pitch =0;

	Vec3f camUp = { 0, 1, 0 };
	Vec3f camPos = { 0, 0, 3 };
	Vec3f camDir = { 0, 0, -1 };
	Vec3f camTargetPos = { 0, 0, -1 };
	//Vec3f targetPos = { 0, 0, 0 };

	Matrix ViewMatrix;
	Matrix ProjectionMatrix;
public:
	Camera();
	Camera(float screenAspect, float near, float far, float fovy);
	~ Camera();

	inline void setViewMatrix();

	void setCamera(Vec3f& pos, Vec3f& targetPos, Vec3f& up);

	void moveStraight(float amount); //ǰ���ƶ�
	void moveTransverse(float amount); //�����ƶ�
	void moveVertical(float amount); //�����ƶ�
	void rotateCamera(Vec2f offset);  // yaw����
	void changeFov(float amount);  // �ı���Ұ
	void setFov(float Fov);  // �ı���Ұ

	Vec3f getPos();
	float getNear();
	float getFar();

	Matrix getViewMatrix();
	Matrix getProjMatrix();
};


extern Camera defaultCamera;

#endif