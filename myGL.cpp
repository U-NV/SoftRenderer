#include"myGL.h"

template <typename T>
inline Vector<T> cross(Vector<T> &a, Vector<T> &b) {
	int len = a.len();
	assert(len == 3);
	Vector<T> temp(len);
	temp[0] = a[1] * b[2] - a[2] * b[1];
	temp[1] = a[2] * b[0] - a[0] * b[2];
	temp[2] = a[0] * b[1] - a[1] * b[0];
	return temp;
}

inline double lerp(double a, double b, double rate) {
	return a + (b - a) * rate;
}

inline TGAColor lerp(TGAColor a, TGAColor b, double rate) {
	TGAColor temp;
	temp.r = (unsigned char)lerp(a.r, b.r, rate);
	temp.g = (unsigned char)lerp(a.g, b.g, rate);
	temp.b = (unsigned char)lerp(a.b, b.b, rate);
	temp.a = (unsigned char)lerp(a.a, b.a, rate);
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

void drawLine(int x0, int y0, int x1, int y1, TGAColor color, SDL_Renderer* gRenderer, SDL_Window* gWindow) {
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
			SDLDrawPixel(gRenderer, gWindow, y, x, color);
        }
        else {
			SDLDrawPixel(gRenderer, gWindow, x, y, color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}

void drawLine(Vector<double> a, Vector<double> b,TGAColor color, SDL_Renderer* gRenderer, SDL_Window* gWindow) {
    int x0 = (int)a[0];
    int y0 = (int)a[1];
    int x1 = (int)b[0];
    int y1 = (int)b[1];
    drawLine(x0, y0, x1, y1, color, gRenderer, gWindow);
}

void drawTriangle2D(std::vector<Vector<double>> vertexBuffer, TGAColor color, SDL_Renderer* gRenderer, SDL_Window* gWindow) {
	drawLine(vertexBuffer[0], vertexBuffer[1], color, gRenderer, gWindow);
	drawLine(vertexBuffer[1], vertexBuffer[2], color, gRenderer, gWindow);
	drawLine(vertexBuffer[2], vertexBuffer[0], color, gRenderer, gWindow);
}

void drawTriangle2D(std::vector<Vector<double>> vertexBuffer, std::vector<TGAColor> colorBuffer, SDL_Renderer* gRenderer, SDL_Window* gWindow) {
	int w = 0, h = 0;
	SDL_GetWindowSize(gWindow, &w, &h);
	Vector<double> bboxmin(w - 1, h - 1);
	Vector<double> bboxmax(0, 0);
	Vector<double> clamp(w - 1, h - 1);

	for (int i = 0; i < 3; i++) {
		bboxmin[0] = std::max(0.0, std::min(bboxmin[0], vertexBuffer[i][0]));
		bboxmax[0] = std::min(clamp[0], std::max(bboxmax[0], vertexBuffer[i][0]));

		bboxmin[1] = std::max(0.0, std::min(bboxmin[1], vertexBuffer[i][1]));
		bboxmax[1] = std::min(clamp[1], std::max(bboxmax[1], vertexBuffer[i][1]));
	}
	//std::cout << "min:" << bboxmin << std::endl;
	//std::cout << "max:" << bboxmax << std::endl;
	Vector<double> P(0,0,0,0);
	for (P[0] = bboxmin[0]; P[0] <= bboxmax[0]; P[0]++) {
		for (P[1] = bboxmin[1]; P[1] <= bboxmax[1]; P[1]++) {
			Vector<double> v0 = vertexBuffer[2] - vertexBuffer[0];
			Vector<double> v1 = vertexBuffer[1] - vertexBuffer[0];
			Vector<double> v2 = P - vertexBuffer[0];

			//计算点积
			double dot00 = v0 * v0;
			double dot01 = v0 * v1;
			double dot02 = v0 * v2;
			double dot11 = v1 * v1;
			double dot12 = v1 * v2;

			//计算重心坐标
			double invDenom = 1 / ((double)dot00 * dot11 - dot01 * dot01);
			double u = ((double)dot11 * dot02 - dot01 * dot12) * invDenom;
			double v = ((double)dot00 * dot12 - dot01 * dot02) * invDenom;

			if ((u >= 0) && (v >= 0) && (u + v < 1)) {
				TGAColor leftColor = lerp(colorBuffer[0], colorBuffer[1], u);
				TGAColor rightColor = lerp(colorBuffer[2], colorBuffer[1], u);
				TGAColor color = lerp(leftColor, rightColor, v);
				SDLDrawPixel(gRenderer, gWindow, P[0], P[1], color);
			}
		}
	}
	
}

Matrix<double> translate(double x, double y, double z) {
	Matrix<double> t(4, 4);
	t.identity();
	t[0][3] = x;
	t[1][3] = y;
	t[2][3] = z;
	return t;
};

Matrix<double> scale(double x, double y, double z) {
	Matrix<double> t(4, 4);
	t.identity();
	t[0][0] = x;
	t[1][1] = y;
	t[2][2] = z;
	return t;
};

Matrix<double> rotate(Vector<double> axis,double theta) {
	Matrix<double> t(4, 4);
	axis.normalize();
	axis.print();

	theta = theta * PI / 180.0;
	double cosTheta = std::cos(theta);
	double sinTheta = std::sin(theta);

	std::cout << "sinAngle:" << sinTheta << std::endl;
	std::cout << "cosAngle:" << cosTheta << std::endl;
	double u = axis[0];
	double v = axis[1];
	double w = axis[2];

	t[0][0] = cosTheta + (u * u) * (1. - cosTheta);
	t[0][1] = u * v * (1. - cosTheta) + w * sinTheta;
	t[0][2] = u * w * (1.- cosTheta) - v * sinTheta;
	t[0][3] = 0;

	t[1][0] = u * v * (1. - cosTheta) - w * sinTheta;
	t[1][1] = cosTheta + v * v * (1 - cosTheta);
	t[1][2] = w * v * (1. - cosTheta) + u * sinTheta;
	t[1][3] = 0;

	t[2][0] = u * w * (1. - cosTheta) + v * sinTheta;
	t[2][1] = v * w * (1. - cosTheta) - u * sinTheta;
	t[2][2] = cosTheta + w * w * (1. - cosTheta);
	t[2][3] = 0;

	t[3][0] = 0;
	t[3][1] = 0;
	t[3][2] = 0;
	t[3][3] = 1;

	return t;
};



Matrix<double> lookat(Vector<double> eye, Vector<double> center, Vector<double> up){
	Vector<double> z = (eye - center).normalize();
	Vector<double> x = cross(up, z).normalize();
	Vector<double> y = cross(z, x).normalize();
	Matrix<double> Minv(4, 4);
	Minv.identity();
	Matrix<double> Tr(4, 4);
	Tr.identity();


	for (int i = 0; i < 3; i++) {
		Minv[0][i] = x[i];
		Minv[1][i] = y[i];
		Minv[2][i] = z[i];
		Tr[i][3] = -center[i];
	}
	return Minv * Tr;
}

//Matrix<double> setFrustum(float l, float r, float b, float t, float n, float f)
//{
//	Matrix<double> matrix;
//	matrix[0][0] = 2 * n / (r - l);
//	matrix[1][0] = 2 * n / (t - b);
//	matrix[8][] = (r + l) / (r - l);
//	matrix[9] = (t + b) / (t - b);
//	matrix[2][2] = -(f + n) / (f - n);
//	matrix[3][2] = -1;
//	matrix[14] = -(2 * f * n) / (f - n);
//	matrix[15] = 0;
//	return matrix;
//}

Matrix<double> projection(double width, double height, double zNear, double zFar) {
	Matrix<double> projection(4, 4);
	double r = width / 2.0;
	double t = height / 2.0;
	projection[0][0] = zNear / r;
	projection[1][1] = zNear / t;
	projection[2][2] = -1*(zFar+zNear) /(zFar-zNear);
	projection[2][3] = -2*zFar*zNear / (zFar-zNear);
	projection[3][2] = -1;
	return projection;
}


Matrix<double> viewport(int x, int y, int w, int h) {
	Matrix<double> Viewport(4, 4);
	Viewport.identity();
	Viewport[0][3] = x + w / 2.f;
	Viewport[1][3] = y + h / 2.f;
	Viewport[2][3] = 1.f;
	Viewport[0][0] = w / 2.f;
	Viewport[1][1] = h / 2.f;
	Viewport[2][2] = 0;
	return Viewport;
}
