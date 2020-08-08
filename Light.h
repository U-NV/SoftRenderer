#pragma once
#include "myVector.h"
#include "myGL.h"
#include "Camera.h"

struct Material
{
	Vec3f ambient;
	Vec3f diffuse;
	Vec3f specular;
	float shininess;
};

class Light
{
public:
	Vec3f lightColor;
	float lightPower;


	//shadow
	bool enableShadow = false;
	//º∆À„“ı”∞Ã˘Õº
	Camera lightCamera;
	Matrix lightMatrix;
	double* depthBuffer = NULL;
	ViewPort* ShadowPort = NULL;
	

	inline float beIlluminated(const Vec3f& world_pos, float bias);
	virtual Vec3f calcLightColor(const Vec3f& normal, const  Vec3f& worldPos, const  Vec3f& viewDir, const  Material& material) = 0;

	~Light() {
		if (depthBuffer != NULL)
			delete[] depthBuffer;
		if (ShadowPort != NULL)
			delete ShadowPort;
	}
};

class PointLight:public Light
{
public:
	Vec3f lightPos;

	float constant = 1.0f;
	float linear = 0.22f;
	float quadratic = 0.20f;
	PointLight() {};
	PointLight(Vec3f pos,Vec3f color,float power,bool shadow = false){
		lightPos = pos;
		lightColor = color;
		lightPower = power;
		enableShadow = shadow;
	};
	Vec3f calcLightColor(const Vec3f& normal, const  Vec3f& worldPos, const  Vec3f& viewDir, const  Material& material);
};

class DirLight:public Light
{
public:
	Vec3f lightDir;
	DirLight() {};
	DirLight(Vec3f dir, Vec3f color, float power) {
		lightDir = dir;
		lightColor = color;
		lightPower = power;
	};
	Vec3f calcLightColor(const Vec3f& normal, const  Vec3f& worldPos, const  Vec3f& viewDir, const  Material& material);
};