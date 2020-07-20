#pragma once
#ifndef __MY_GL__
#define __MY_GL__
#include "tgaimage.h"
#include "myVector.h"
#include <vector>
#include <algorithm>

#include "SDL.h"
#include "SDL_revision.h"

inline void SDLDrawPixel(SDL_Renderer* gRenderer, SDL_Window* gWindow, int x, int y, const TGAColor& color);

void drawLine(Vector<int> a, Vector<int> b, TGAImage& image, TGAColor color);
void drawLine(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color);
//void drawTriangle2D(std::vector<Vector> vertexBuffer, std::vector<TGAColor> colorBuffer, TGAImage& image);
void drawTriangle2D(std::vector<Vector<int>> vertexBuffer, std::vector<TGAColor> colorBuffer, SDL_Renderer* gRenderer, SDL_Window* gWindow);

inline double lerp(double a, double b, double rate);
inline TGAColor lerp(TGAColor a, TGAColor b, double rate);
#endif // !__MY_GL__
