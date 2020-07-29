#pragma once
#ifndef __MY_GL__
#define __MY_GL__

#include "camera.h"
#include "tgaimage.h"
#include "myVector.h"
#include <vector>
#include <algorithm>

#define PI 2*acos(0.0)
#define DegToRad PI/180.0
#define EPSILON 1e-5

extern Matrix ModelMatrix;
extern Matrix ViewMatrix;
extern Matrix ProjectionMatrix;
extern Matrix MVP;
extern Matrix lightSpaceMatrix;

extern bool enableFaceCulling;
extern bool enableFrontFaceCulling;

//数学
template <typename T>
inline T lerp(T a, T b, double rate) {
	return a + (b - a) * rate;
}

template <typename T>
inline T clamp(T a, T min, T max) {
	return std::min<T>(max, std::max<T>(min, a));
}

//shader
class VerInf {
public:
	Vec3f world_pos;
	Vec4f clip_coord;
	Vec3f ndc_coord;
	Vec3f normal;
	Vec2f uv;

	double depth;
	double recip_w;
	//double light_recip_w;
	
	//Vec4f clipPosLightSpace;


	static VerInf verLerp(const VerInf& v1, const VerInf& v2, const float& factor);
};

struct IShader {
    virtual ~IShader();
    virtual void vertex(int iface, int nthvert, VerInf& faceVer) = 0;
    virtual bool fragment(VerInf verInf, TGAColor& color) = 0;
};

//绘制三角面片
void triangle(bool farme, VerInf* vertexs, IShader& shader, int width, int height, double* zbuffer, ColorVec* drawBuffer);
void drawTriangle2D(VerInf** verInf, IShader& shader, int& width, int& height, double* zbuffer, ColorVec* drawBuffer);
//模型矩阵
Matrix translate(double x, double y, double z);
Matrix rotate(Vec3f& axis, double theta);
Matrix scale(double x, double y, double z);

//视图矩阵
Matrix lookat(Vec3f& eye, Vec3f& center, Vec3f& up);

//透视矩阵
Matrix projection(double width, double height, double zNear, double zFar);
/*Matrix setFrustum(double l, double r, double b, double t, double n, double f);*/
Matrix setFrustum(double fovY, double aspectRatio, double front, double back);
Matrix mat4_orthographic(float right, float top, float near, float far);

float LinearizeDepth(float depth);

//视窗变换
Vec3f viewport_transform(int width, int height, Vec3f ndc_coord);


#endif // !__MY_GL__
