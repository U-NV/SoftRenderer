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

inline void SDLDrawPixel(SDL_Renderer* gRenderer, SDL_Window* gWindow, int x, int y, Vector<int>& color);

void drawLine(int x0, int y0, int x1, int y1, Vector<int> color, SDL_Renderer* gRenderer, SDL_Window* gWindow);
void drawLine(Vector<int> a, Vector<int> b, Vector<int> color, SDL_Renderer* gRenderer, SDL_Window* gWindow);
//void drawTriangle2D(std::vector<Vector> vertexBuffer, std::vector<TGAColor> colorBuffer, TGAImage& image);
void drawTriangle2D(std::vector<Vector<double>> vertexBuffer, std::vector<Vector<int>> colorBuffer,double* zbuffer, SDL_Renderer* gRenderer, SDL_Window* gWindow);
void drawTriangle2D(std::vector<Vector<double>> vertexBuffer, Vector<int> color, SDL_Renderer* gRenderer, SDL_Window* gWindow);
template <typename T>
inline T lerp(T a, T b, double rate);
inline TGAColor lerp(TGAColor a, TGAColor b, double rate);
template <typename T>
inline Vector<T> cross(const Vector<T>& a, const Vector<T>& b);

//模型矩阵
Matrix<double> translate(double x, double y, double z);
Matrix<double> rotate(Vector<double> axis, double angle);
Matrix<double> scale(double x, double y, double z);
Matrix<double> lookat(Vector<double> eye, Vector<double> center, Vector<double> up);

//视图
Matrix<double> lookat(Vector<double> eye, Vector<double> center, Vector<double> up);

//透视
Matrix<double> projection(double width, double height, double zNear, double zFar);
Matrix<double> setFrustum(double l, double r, double b, double t, double n, double f);
Matrix<double> setFrustum(double fovY, double aspectRatio, double front, double back);


//视窗
Matrix<double> viewport(int x, int y, int w, int h);
#endif // !__MY_GL__
