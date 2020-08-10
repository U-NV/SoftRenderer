#pragma once
#ifndef __MY_GL__
#define __MY_GL__

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
extern Matrix normalMatrix;

extern bool enableFaceCulling;
extern bool enableFrontFaceCulling;

extern bool enableZTest;
extern bool enableZWrite;
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

	Vec2i screen_coord;

	double depth;
	double recip_w;
	//double light_recip_w;
	
	//Vec4f clipPosLightSpace;


	static VerInf verLerp(const VerInf& v1, const VerInf& v2, const float& factor);
};

struct IShader {
    virtual ~IShader();
    virtual void vertex(int iface, int nthvert, VerInf& faceVer) = 0;
    virtual bool fragment(VerInf& verInf, Vec4f& color) = 0;
};

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

float LinearizeDepth(float depth, float near, float far);

//视窗变换
class ViewPort
{
public:
	Vec2i v_pos;
	int v_width;
	int v_height;
	ViewPort(Vec2i pos, int width, int height) :v_pos(pos), v_width(width), v_height(height) {};
	Vec3f transform(Vec3f& ndc_coord) const;
};

class Frame
{
public:
	Frame(int w,int h);
	~Frame();
	void setPixel(int& x, int& y, TGAColor& color);
	void setPixel(int& x, int& y, Vec4f& color);

	Vec4f* getPixel(int& x, int& y);
	void fill(const Vec4f& defaultColor);
	int f_width;
	int f_height;
private:
	Vec4f* buffer;
};


//立方贴图
Vec4f CubeMap(TGAImage* skyboxFaces, Vec3f pos);


//绘制三角面片
void triangle(VerInf* vertexs, IShader& shader,
	const ViewPort& port, const float& near, const float& far,
	double* zbuffer, Frame* drawBuffer,
	bool farme, bool fog);

Vec3f reflect(Vec3f& I, Vec3f& N);
#endif // !__MY_GL__
