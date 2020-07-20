#include"myGL.h"

inline double lerp(double a, double b, double rate) {
	return a + (b - a) * rate;
}

inline TGAColor lerp(TGAColor a, TGAColor b, double rate) {
	TGAColor temp;
	temp.r = lerp(a.r, b.r, rate);
	temp.g = lerp(a.g, b.g, rate);
	temp.b = lerp(a.b, b.b, rate);
	temp.a = lerp(a.a, b.a, rate);
	return temp;
}

//将坐标系原点改为左下角
inline void SDLDrawPixel(SDL_Renderer* gRenderer, SDL_Window* gWindow, int x, int y,const TGAColor &color)
{
	int w = 0, h = 0;
	SDL_SetRenderDrawColor(gRenderer, color.r, color.g, color.b, color.a);
	SDL_GetWindowSize(gWindow, &w, &h);
	SDL_RenderDrawPoint(gRenderer, x, h - 1 - y);
}

void drawLine(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1 - x0;
    int dy = y1 - y0;
    int derror2 = std::abs(dy) * 2;
    int error2 = 0;
    int y = y0;
    for (int x = x0; x <= x1; x++) {
        if (steep) {
            image.set(y, x, color);
        }
        else {
            image.set(x, y, color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}

void drawLine(Vector<int> a, Vector<int> b, TGAImage& image, TGAColor color) {
    int x0 = a[0];
    int y0 = a[1];
    int x1 = b[0];
    int y1 = b[1];
    drawLine(x0, y0, x1, y1, image, color);
}

void drawTriangle2D(std::vector<Vector<int>> vertexBuffer, std::vector<TGAColor> colorBuffer, SDL_Renderer* gRenderer, SDL_Window* gWindow) {
	int vertexLength = vertexBuffer.size();
	for (int i = 0; i < vertexLength; i += 3) {
		if (i + 3 > vertexLength)break;

		int w = 0, h = 0;
		SDL_GetWindowSize(gWindow, &w, &h);
		Vector<int> bboxmin(w - 1, h - 1);
		Vector<int> bboxmax(0, 0);
		Vector<int> clamp(w - 1, h - 1);
		for (int j = 0; j < 3; j++) {
			int vertexId = i + j;
			bboxmin[0] = std::max(0, std::min(bboxmin[0], vertexBuffer[vertexId][0]));
			bboxmax[0] = std::min(clamp[0], std::max(bboxmax[0], vertexBuffer[vertexId][0]));

			bboxmin[1] = std::max(0, std::min(bboxmin[1], vertexBuffer[vertexId][1]));
			bboxmax[1] = std::min(clamp[1], std::max(bboxmax[1], vertexBuffer[vertexId][1]));
		}
		//std::cout << "min:" << bboxmin << std::endl;
		//std::cout << "max:" << bboxmax << std::endl;
		Vector<int> P(0,0);
		for (P[0] = bboxmin[0]; P[0] <= bboxmax[0]; P[0]++) {
			for (P[1] = bboxmin[1]; P[1] <= bboxmax[1]; P[1]++) {
				Vector<int> v0 = vertexBuffer[i + 2] - vertexBuffer[i];
				Vector<int> v1 = vertexBuffer[i + 1] - vertexBuffer[i];
				Vector<int> v2 = P - vertexBuffer[i];

				//计算点积
				int dot00 = v0 * v0;
				int dot01 = v0 * v1;
				int dot02 = v0 * v2;
				int dot11 = v1 * v1;
				int dot12 = v1 * v2;

				//计算重心坐标
				double invDenom = 1 / double(dot00 * dot11 - dot01 * dot01);
				double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
				double v = (dot00 * dot12 - dot01 * dot02) * invDenom;

				if ((u >= 0) && (v >= 0) && (u + v < 1)) {
					TGAColor leftColor = lerp(colorBuffer[i], colorBuffer[i + 1], u);
					TGAColor rightColor = lerp(colorBuffer[i + 2], colorBuffer[i + 1], u);
					TGAColor color = lerp(leftColor, rightColor, v);
					SDLDrawPixel(gRenderer, gWindow, P[0], P[1], color);
				}
			}
		}
	}
}


//void drawTriangle2D(std::vector<Vector> vertexBuffer, std::vector<TGAColor> colorBuffer, TGAImage& image) {
//	int vertexLength = vertexBuffer.size();
//	for (int i = 0; i < vertexLength; i += 3) {
//		if (i + 3 > vertexLength)break;
//		std::vector<Point2D> trianglePoint(3);
//		for (int j = 0; j < 3; j++) {
//			trianglePoint[j].vertex[0] = vertexBuffer[i + j].x;
//			trianglePoint[j].vertex.y = vertexBuffer[i + j].y;
//
//			trianglePoint[j].color.r = colorBuffer[i + j].r;
//			trianglePoint[j].color.g = colorBuffer[i + j].g;
//			trianglePoint[j].color.b = colorBuffer[i + j].b;
//			trianglePoint[j].color.a = colorBuffer[i + j].a;
//
//			//trianglePoint[j].vertex.print();
//			//printf("(%d,%d) - r:%d g:%d b:%d\n", trianglePoint[j].vertex.x, trianglePoint[j].vertex.y, trianglePoint[j].color.r, trianglePoint[j].color.g, trianglePoint[j].color.b);
//		}
//
//		Vector bboxmin(image.get_width() - 1, image.get_height() - 1);
//		Vector bboxmax(0, 0);
//		Vector clamp(image.get_width() - 1, image.get_height() - 1);
//		for (int j = 0; j < 3; j++) {
//			bboxmin.x = std::max(0.0, std::min(bboxmin.x, trianglePoint[j].vertex.x));
//			bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, trianglePoint[j].vertex.x));
//
//			bboxmin.y = std::max(0.0, std::min(bboxmin.y, trianglePoint[j].vertex.y));
//			bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, trianglePoint[j].vertex.y));
//		}
//		//std::cout << "min:" << bboxmin << std::endl;
//		//std::cout << "max:" << bboxmax << std::endl;
//		Vector P;
//		for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
//			for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
//				Vector v0 = trianglePoint[2].vertex - trianglePoint[0].vertex;
//				Vector v1 = trianglePoint[1].vertex - trianglePoint[0].vertex;
//				Vector v2 = P - trianglePoint[0].vertex;
//
//				//计算点积
//				int dot00 = v0 * v0;
//				int dot01 = v0 * v1;
//				int dot02 = v0 * v2;
//				int dot11 = v1 * v1;
//				int dot12 = v1 * v2;
//
//				//计算重心坐标
//				double invDenom = 1 / double(dot00 * dot11 - dot01 * dot01);
//				double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
//				double v = (dot00 * dot12 - dot01 * dot02) * invDenom;
//
//				if ((u >= 0) && (v >= 0) && (u + v < 1)) {
//					TGAColor leftColor = lerp(trianglePoint[0].color, trianglePoint[1].color, u);
//					TGAColor rightColor = lerp(trianglePoint[2].color, trianglePoint[1].color, u);
//					TGAColor color = lerp(leftColor, rightColor, v);
//					image.set(P.x, P.y, color);
//				}
//			}
//		}
//	}
//}