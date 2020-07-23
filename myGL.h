#pragma once
#ifndef __MY_GL__
#define __MY_GL__
#include "tgaimage.h"
#include "myVector.h"
#include <vector>
#include <algorithm>

#include "SDL.h"
#include "SDL_revision.h"

#define PI acos(-1)
#define EPSILON 1e-5
//shader
struct IShader {
    virtual ~IShader();
    virtual Vec4f vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor& color) = 0;
};

//数学
template <typename T>
inline T lerp(T a, T b, double rate);

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
