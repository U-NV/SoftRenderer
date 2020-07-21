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


template <typename T>
inline T lerp(T a, T b, double rate) {
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
inline void SDLDrawPixel(SDL_Renderer* gRenderer, SDL_Window* gWindow, int x, int y,Vector<int>& color)
{
	int w = 0, h = 0;
	SDL_SetRenderDrawColor(gRenderer, color[0], color[1], color[2], color[3]);
	SDL_GetWindowSize(gWindow, &w, &h);
	SDL_RenderDrawPoint(gRenderer, x, h - 1 - y);
	//SDL_RenderDrawPoint(gRenderer, x,y);
}

void drawLine(int x0, int y0, int x1, int y1, Vector<int> color, SDL_Renderer* gRenderer, SDL_Window* gWindow) {
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

void drawLine(Vector<double> a, Vector<double> b, Vector<int> color, SDL_Renderer* gRenderer, SDL_Window* gWindow) {
    int x0 = (int)a[0];
    int y0 = (int)a[1];
    int x1 = (int)b[0];
    int y1 = (int)b[1];
    drawLine(x0, y0, x1, y1, color, gRenderer, gWindow);
}

void drawTriangle2D(std::vector<Vector<double>> vertexBuffer, Vector<int> color, SDL_Renderer* gRenderer, SDL_Window* gWindow) {
	drawLine(vertexBuffer[0], vertexBuffer[1], color, gRenderer, gWindow);
	drawLine(vertexBuffer[1], vertexBuffer[2], color, gRenderer, gWindow);
	drawLine(vertexBuffer[2], vertexBuffer[0], color, gRenderer, gWindow);
}


static Vector<double> calculate_weights(std::vector<Vector<int>> screen_coords, Vector<int> P) {
	Vector<double> a(screen_coords[0][0], screen_coords[0][1]);
	Vector<double> b(screen_coords[1][0], screen_coords[1][1]);
	Vector<double> c(screen_coords[2][0], screen_coords[2][1]);
	Vector<double> p(P[0], P[1]);
	Vector<double> ab = b - a;
	Vector<double> ac = c - a;
	Vector<double> ap = p - a;
	float factor = 1 / (ab[0] * ac[1] - ab[1] * ac[0]);
	float s = (ac[1] * ap[0] - ac[0] * ap[1]) * factor;
	float t = (ab[0] * ap[1] - ab[1] * ap[0]) * factor;
	return Vector<double>(1 - s - t, s, t);
}

static double interpolate_depth(double screen_depths[3], Vector<double> weights) {
	double depth0 = screen_depths[0] * weights[0];
	double depth1 = screen_depths[1] * weights[1];
	double depth2 = screen_depths[2] * weights[2];
	return depth0 + depth1 + depth2;
}

static Vector<int> interpolate_color(std::vector <Vector<int>> colors, Vector<double> weights) {

	Vector<double> color0 = Vector<double>(colors[0][0] * weights[0], colors[0][1] * weights[0], colors[0][2] * weights[0]);
	Vector<double> color1 = Vector<double>(colors[1][0] * weights[1], colors[1][1] * weights[1], colors[1][2] * weights[1]);
	Vector<double> color2 = Vector<double>(colors[2][0] * weights[2], colors[2][1] * weights[2], colors[2][2] * weights[2]);
	Vector<double> color = color0 + color1 + color2;
	return Vector<int>(color[0], color[1], color[2],255);
}

void drawTriangle2D(std::vector<Vector<double>> vertexBuffer, std::vector<Vector<int>> colorBuffer, double* zbuffer,SDL_Renderer* gRenderer, SDL_Window* gWindow) {
	int w = 0, h = 0;
	SDL_GetWindowSize(gWindow, &w, &h);
	

	std::vector<Vector<int>> screen_coords(3);
	double screen_depths[3];
	for (int i = 0; i < 3; i++) {
		screen_coords[i] = Vector<int>(vertexBuffer[i][0], vertexBuffer[i][1]);
		screen_depths[i] = vertexBuffer[i][2];
	}

	Vector<int> bboxmin(w - 1, h - 1);
	Vector<int> bboxmax(0, 0);
	Vector<int> clamp(w - 1, h - 1);
	for (int i = 0; i < 3; i++) {
		bboxmin[0] = std::max(0, std::min(bboxmin[0], screen_coords[i][0]));
		bboxmax[0] = std::min(clamp[0], std::max(bboxmax[0], screen_coords[i][0]));

		bboxmin[1] = std::max(0, std::min(bboxmin[1], screen_coords[i][1]));
		bboxmax[1] = std::min(clamp[1], std::max(bboxmax[1], screen_coords[i][1]));
	}
	//std::cout << "bboxmin[0]" << std::endl;
	//bboxmin.print();
	//bboxmax.print();
	Vector<int> P(0,0);
	for (P[0] = bboxmin[0]; P[0] <= bboxmax[0]; P[0]++) {
		for (P[1] = bboxmin[1]; P[1] <= bboxmax[1]; P[1]++) {
			Vector<double> weights = calculate_weights(screen_coords, P);
			int weight0_okay = weights[0] > -EPSILON;
			int weight1_okay = weights[1] > -EPSILON;
			int weight2_okay = weights[2] > -EPSILON;
			if (weight0_okay && weight1_okay && weight2_okay) {
				int zbufferInd = (int)P[0] + (int)P[1] * w;
				double frag_depth = interpolate_depth(screen_depths, weights);
				if (frag_depth <= zbuffer[zbufferInd]) {
					zbuffer[zbufferInd] = frag_depth;
					Vector<int> color = interpolate_color(colorBuffer,weights);

					/*int depthColor = frag_depth * 255;
					Vector<int> color = Vector<int>(depthColor, depthColor, depthColor, 255);*/
					//std::cout << depthColor << std::endl;
					
					SDLDrawPixel(gRenderer, gWindow, P[0], P[1], color);
				}
			};
			
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
	//Matrix<double> Tr(4, 4);
	//Tr.identity();
	/*for (int i = 0; i < 3; i++) {
		Minv[0][i] = x[i];
		Minv[1][i] = y[i];
		Minv[2][i] = z[i];
		Tr[i][3] = -1*();
	}
	return Minv * Tr;*/

	for (int i = 0; i < 3; i++) {
		Minv[0][i] = x[i];
		Minv[1][i] = y[i];
		Minv[2][i] = z[i];
	}
	Minv[0][3] = -1*(x * eye);
	Minv[1][3] = -1*(y * eye);
	Minv[2][3] = -1*(z * eye);

	return Minv;
}

Matrix<double> setFrustum(double left, double right, double bottom, double top, double near, double far) {
	double x_range = right - left;
	double y_range = top - bottom;
	double z_range = far - near;

	Matrix<double> matrix(4, 4);
	matrix.identity();
	matrix[0][0] = 2.0 * near / x_range;
	matrix[0][0] = 2.0 * near / y_range;
	matrix[0][0] = (left + right) / x_range;
	matrix[0][0] = (bottom + top) / y_range;
	matrix[0][0] = -(near + far) / z_range;
	matrix[0][0] = -2.0 * near * far / z_range;
	matrix[0][0] = -1.0;
	matrix[0][0] = 0.0;

	//matrix[0][0] = 2 * n / (r - l);
	//matrix[1][1] = 2 * n / (t - b);
	//matrix[2][0] = (r + l) / (r - l);
	//matrix[2][1] = (t + b) / (t - b);
	//matrix[2][2] = -(f + n) / (f - n);
	//matrix[2][3] = -1;
	//matrix[3][2] = -(2 * f * n) / (f - n);
	//matrix[3][3] = 0;
	return matrix;
}

Matrix<double> setFrustum(double fovy, double aspect, double near, double far)
{
	//float tangent = tanf(fovY / 2 * (PI / 180));   // tangent of half fovY
	//float height = front * tangent;           // half height of near plane
	//float width = height * aspectRatio;       // half width of near plane

	//// params: left, right, bottom, top, near, far
	//return setFrustum(-width, width, -height, height, front, back);

	float z_range = far - near;
	Matrix<double> matrix(4, 4);
	matrix.identity();
	assert(fovy > 0 && aspect > 0);
	assert(near > 0 && far > 0 && z_range > 0);
	matrix[1][1] = 1 / (float)tan(fovy / 2);
	matrix[0][0] = matrix[1][1] / aspect;
	matrix[2][2] = -(near + far) / z_range;
	matrix[2][3] = -2 * near * far / z_range;
	matrix[3][2] = -1;
	matrix[3][3] = 0;
	return matrix;

}


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
