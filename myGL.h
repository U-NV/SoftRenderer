#pragma once
#ifndef __MY_GL__
#define __MY_GL__
#include "tgaimage.h"
#include "myVector.h"
#include <vector>
#include <algorithm>
class Point3D
{
public:
	Vector3 vertex;
	TGAColor color;
	Point3D();
	~Point3D();
private:

};

class Point2D
{
public:
	Vector2i vertex;
	TGAColor color;
	Point2D();
	~Point2D();
private:

};


void drawLine(Vector2i a, Vector2i b, TGAImage& image, TGAColor color);
void drawLine(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color);
void drawTriangle2D(std::vector<Vector2i> vertexBuffer, std::vector<TGAColor> colorBuffer, TGAImage& image);

inline double lerp(double a, double b, double rate);
inline TGAColor lerp(TGAColor a, TGAColor b, double rate);
#endif // !__MY_GL__
