#pragma once
#ifndef __MY_GL__
#define __MY_GL__

#include "camera.h"
#include "tgaimage.h"
#include "myVector.h"


#include <vector>
#include <algorithm>

#include "SDL.h"
#include "SDL_revision.h"

#define PI 2*acos(0.0)
#define DegToRad PI/180.0
#define EPSILON 1e-5


extern Matrix ModelMatrix;
extern Matrix ViewMatrix;
extern Matrix ProjectionMatrix;
extern Matrix MVP;

//数学
template <typename T>
inline T lerp(T a, T b, double rate) {
	return a + (b - a) * rate;
}

//Vec4f lerp(const Vec4f& v1, const Vec4f& v2, float factor) {
//	return v1 * (1.0f - factor)  + v2 * factor ;
//}
//
//Vec3f lerp(const Vec3f& v1, const Vec3f& v2, float factor) {
//	return v1 * (1.0f - factor)  + v2 * factor ;
//}
//
//Vec2f lerp(const Vec2f& v1, const Vec2f& v2, float factor) {
//	return v1 * (1.0f - factor)  + v2 * factor ;
//}

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

	double recip_w;

	static VerInf verLerp(const VerInf& v1, const VerInf& v2, const float& factor);
};

struct IShader {
    virtual ~IShader();
    virtual void vertex(int iface, int nthvert, VerInf &faceVer) = 0;
    virtual bool fragment(VerInf verInf, TGAColor& color) = 0;
};


//template <typename T>
//inline Vector<T> cross(const Vector<T>& a, const Vector<T>& b);

//Vector<double> interpolate_uv(Vector<double>* uvs, Vector<double>& weights);

//点线面
//inline void SDLDrawPixel(SDL_Renderer* gRenderer, SDL_Window* gWindow, int x, int y, TGAColor& color);
//void draw2DFrame(VerInf* vertexs, TGAColor& color, SDL_Renderer* gRenderer, SDL_Window* gWindow);
void triangle(bool farme,VerInf* vertexs, IShader& shader, double* zbuffer, SDL_Renderer* gRenderer, SDL_Window* gWindow);

//模型矩阵
Matrix translate(double x, double y, double z);
Matrix rotate(Vec3f& axis, double theta);
Matrix scale(double x, double y, double z);

//视图
Matrix lookat(Vec3f& eye, Vec3f& center, Vec3f& up);

//透视
Matrix projection(double width, double height, double zNear, double zFar);
Matrix setFrustum(double l, double r, double b, double t, double n, double f);
Matrix setFrustum(double fovY, double aspectRatio, double front, double back);

//视窗
Vec3f viewport_transform(int width, int height, Vec3f ndc_coord);


#endif // !__MY_GL__
