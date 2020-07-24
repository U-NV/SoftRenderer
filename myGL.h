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


//shader
class VerInf {
public:
	Vec3f world_pos;
	Vec4f clip_coord;
	Vec3f ndc_coord;

	Vec3f normal;
	Vec2f uv;

	
	double recip_w;
};

struct IShader {
    virtual ~IShader();
    virtual void vertex(int iface, int nthvert, VerInf &faceVer) = 0;
    virtual bool fragment(VerInf verInf, TGAColor& color) = 0;
};

//数学
template <typename T>
inline T lerp(T a, T b, double rate) {
    return a + (b - a) * rate;
}

template <typename T>
inline T clamp(T a, T min,T max) {
    return std::min<T>(max, std::max<T>(min, a));
}


//template <typename T>
//inline Vector<T> cross(const Vector<T>& a, const Vector<T>& b);

//Vector<double> interpolate_uv(Vector<double>* uvs, Vector<double>& weights);

//点线面
inline void SDLDrawPixel(SDL_Renderer* gRenderer, SDL_Window* gWindow, int x, int y, TGAColor& color);
void drawLine(int x0, int y0, int x1, int y1, TGAColor& color, SDL_Renderer* gRenderer, SDL_Window* gWindow);
void drawLine(Vec2i& a, Vec2i& b, TGAColor& color, SDL_Renderer* gRenderer, SDL_Window* gWindow);

void drawLine(Vec3f& a, Vec3f& b, TGAColor& color, SDL_Renderer* gRenderer, SDL_Window* gWindow);


void draw2DFrame(Vec4f* vertexBuffer, TGAColor& color, SDL_Renderer* gRenderer, SDL_Window* gWindow);
//void drawTriangle2D(std::vector<Vector<double>> &vertexBuffer, std::vector<Vector<int>> &colorBuffer,double* zbuffer, SDL_Renderer* gRenderer, SDL_Window* gWindow);
void drawTriangle2D(Vec4f* vertexBuffer, IShader& shader, double* zbuffer, SDL_Renderer* gRenderer, SDL_Window* gWindow);
void triangle(VerInf* vertexs, IShader& shader, double* zbuffer, SDL_Renderer* gRenderer, SDL_Window* gWindow);
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
static Vec3f viewport_transform(int width, int height, Vec3f ndc_coord);


#endif // !__MY_GL__
