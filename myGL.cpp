#include"myGL.h"

IShader::~IShader() {}

//template <typename T>
//inline Vector<T> cross(Vector<T> &a, Vector<T> &b) {
//	int len = a.len();
//	assert(len == 3);
//	Vector<T> temp(len);
//	temp.x = a.y * b.z - a.z * b.y;
//	temp.y = a.z * b.x - a.x * b.z;
//	temp.z = a.x * b.y - a.y * b.x;
//	return temp;
//}

//将坐标系原点改为左下角
inline void SDLDrawPixel(SDL_Renderer* gRenderer, SDL_Window* gWindow, int x, int y,TGAColor & color)
{
	int w = 0, h = 0;
	SDL_SetRenderDrawColor(gRenderer, color[2], color[1], color[0], color[3]);
	SDL_GetWindowSize(gWindow, &w, &h);
	SDL_RenderDrawPoint(gRenderer, x, h - 1 - y);
}

void drawLine(int x0, int y0, int x1, int y1, TGAColor& color, SDL_Renderer* gRenderer, SDL_Window* gWindow) {
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

void drawLine(Vec2i& a, Vec2i& b, TGAColor& color, SDL_Renderer* gRenderer, SDL_Window* gWindow) {
    drawLine(a.x, a.y, b.x, b.y, color, gRenderer, gWindow);
}


void drawLine(Vec3f& a, Vec3f& b, TGAColor& color, SDL_Renderer* gRenderer, SDL_Window* gWindow) {
    drawLine(a.x, a.y, b.x, b.y, color, gRenderer, gWindow);
}


static Vec3f calculate_weights(Vec2i& A, Vec2i& B, Vec2i& C, Vec2i& P) {
	Vec2f ab = B - A;//  = b - a;
	Vec2f ac = C - A;//  = c - a;
	Vec2f ap = P - A;//  = p - a;

	double factor = 1 / (ab.x * ac.y - ab.y * ac.x);
	double s = ((double)ac.y * ap.x - (double)ac.x * ap.y) * factor;
	double t = ((double)ab.x * ap.y - (double)ab.y * ap.x) * factor;

	return Vec3f(1 - s - t, s, t);
}

static double interpolate_depth(double *screen_depths, Vec3f &weights) {
	double depth0 = screen_depths[0] * weights.x;
	double depth1 = screen_depths[1] * weights.y;
	double depth2 = screen_depths[2] * weights.z;
	return depth0 + depth1 + depth2;
}

//Vector<double> interpolate_uv(Vector<double> *uvs, Vector<double> &weights) {
//	Vector<double> uv0 = uvs[0] * weights[0];
//	Vector<double> uv1 = uvs[1] * weights[1];
//	Vector<double> uv2 = uvs[2] * weights[2];
//	return uv0 + uv1 + uv2;
//}

static bool is_back_facing(Vec3f *ndc_coords) {
	Vec3f *a = &ndc_coords[0];
	Vec3f *b = &ndc_coords[1];
	Vec3f *c = &ndc_coords[2];

	float signed_area = (a->x * b->y) - (a->y * b->x) +
						(b->x * c->y) - (b->y * c->x) +
						(c->x * a->y) - (c->y * a->x);

	return signed_area <= 0;
}

void draw2DFrame(Vec4f* vertexBuffer, TGAColor& color, SDL_Renderer* gRenderer, SDL_Window* gWindow) {
	int w = 0, h = 0;
	SDL_GetWindowSize(gWindow, &w, &h);

	Vec3f ndc_coords[3];
	Vec2i screen_coords[3];

	/* perspective division */
	for (int i = 0; i < 3; i++) {
		Vec3f clip_coord = proj<3>(vertexBuffer[i]);
		ndc_coords[i] = clip_coord / vertexBuffer[i][3];
	}

	/* back-face culling */
	if (is_back_facing(ndc_coords)) {
		//std::cout << "back_facing" << std::endl;
		return;
	}

	/* viewport mapping */
	for (int i = 0; i < 3; i++) {
		Vec3f window_coord = viewport_transform(w, h, ndc_coords[i]);
		screen_coords[i] = Vec2i((int)window_coord[0], (int)window_coord[1]);
	}

	drawLine(screen_coords[0], screen_coords[1], color, gRenderer, gWindow);
	drawLine(screen_coords[1], screen_coords[2], color, gRenderer, gWindow);
	drawLine(screen_coords[2], screen_coords[0], color, gRenderer, gWindow);
}

void drawTriangle2D(Vec4f* vertexBuffer, IShader& shader,double* zbuffer, SDL_Renderer* gRenderer, SDL_Window* gWindow) {
	
	int width = 0, height = 0;
	SDL_GetWindowSize(gWindow, &width, &height);

	Vec3f ndc_coords[3];
	Vec2i screen_coords[3];
	double screen_depths[3];
	double recip_w[3];

	/* reciprocals of w */
	for (int i = 0; i < 3; i++) {
		recip_w[i] = 1 / vertexBuffer[i][3];
	}

	/* perspective division */
	for (int i = 0; i < 3; i++) {
		Vec3f clip_coord = proj<3>(vertexBuffer[i]);
		ndc_coords[i] = clip_coord * recip_w[i];
	}

	/* back-face culling */
	if (is_back_facing(ndc_coords)) {
		//std::cout << "back_facing" << std::endl;
		return;
	}

	/* viewport mapping */
	for (int i = 0; i < 3; i++) {
		Vec3f window_coord = viewport_transform(width, height, ndc_coords[i]);
		screen_coords[i] = Vec2i((int)window_coord.x, (int)window_coord.y);
		screen_depths[i] = window_coord.z;
	}


	//std::cout << "clip_coords:" << vertexBuffer[0] << std::endl;
	//std::cout << "ndc_coords:" << ndc_coords[0] <<" 1/w:"<< recip_w[0] << std::endl;
	//std::cout << "screen_coords:" << screen_coords[0] <<" z:"<< screen_depths[0]  << std::endl;


	Vec2i bboxmin(width - 1, height - 1);
	Vec2i bboxmax(0, 0);
	Vec2i clamp(width - 1, height - 1);

	for (int i = 0; i < 3; i++) {
		bboxmin.x = std::max(0, std::min(bboxmin.x, screen_coords[i].x));
		bboxmin.y = std::max(0, std::min(bboxmin.y, screen_coords[i].y));

		bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, screen_coords[i].x));
		bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, screen_coords[i].y));
	}

	Vec2i P(0, 0);
	TGAColor color;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f weights = calculate_weights(screen_coords[0], screen_coords[1], screen_coords[2], P);

			int weight0_okay = weights.x > -EPSILON;
			int weight1_okay = weights.y > -EPSILON;
			int weight2_okay = weights.z > -EPSILON;
			if (weight0_okay && weight1_okay && weight2_okay) {
				int zbufferInd = P.x + P.y * width;
				double frag_depth = interpolate_depth(screen_depths, weights);
				
				if (frag_depth <= zbuffer[zbufferInd]) {
					for(int i=0;i<3;i++) weights[i] = weights[i] * recip_w[i];
					weights = weights / (weights[0]+ weights[1] + weights[2]);
					bool discard = shader.fragment(weights, color);
					//std::cout << frag_depth << " : " << frag_depth * 255 << std::endl;
					//color = TGAColor(frag_depth * 255, frag_depth * 255, frag_depth * 255, 255);
					if (!discard) {
						zbuffer[zbufferInd] = frag_depth;
						SDLDrawPixel(gRenderer, gWindow, P[0], P[1], color);
					}
				}
			}

		}
	}

}


Matrix translate(double x, double y, double z) {
	Matrix t;
	t = Matrix::identity();
	t[0][3] = x;
	t[1][3] = y;
	t[2][3] = z;
	return t;
};

Matrix scale(double x, double y, double z) {
	Matrix t;
	t= Matrix::identity();
	t[0][0] = x;
	t[1][1] = y;
	t[2][2] = z;
	return t;
};

Matrix rotate(Vec3f &axis,double theta) {
	Matrix t;
	axis.normalize();

	theta = theta * PI / 180.0;
	double cosTheta = std::cos(theta);
	double sinTheta = std::sin(theta);

	//std::cout << "sinAngle:" << sinTheta << std::endl;
	//std::cout << "cosAngle:" << cosTheta << std::endl;
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



Matrix lookat(Vec3f &eye, Vec3f& center, Vec3f& up){
	Vec3f z = (eye - center).normalize();
	Vec3f x = cross(up, z).normalize();
	Vec3f y = cross(z, x).normalize();
	Matrix Minv;
	Minv= Matrix::identity();
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

Matrix setFrustum(double left, double right, double bottom, double top, double near, double far) {
	double x_range = right - left;
	double y_range = top - bottom;
	double z_range = far - near;

	Matrix matrix;
	matrix= Matrix::identity();
	matrix[0][0] = 2.0 * near / x_range;
	matrix[0][0] = 2.0 * near / y_range;
	matrix[0][0] = (left + right) / x_range;
	matrix[0][0] = (bottom + top) / y_range;
	matrix[0][0] = -(near + far) / z_range;
	matrix[0][0] = -2.0 * near * far / z_range;
	matrix[0][0] = -1.0;
	matrix[0][0] = 0.0;
	return matrix;
}

Matrix setFrustum(double fovy, double aspect, double near, double far)
{
	double z_range = far - near;
	Matrix matrix;
	matrix= Matrix::identity();
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


Matrix projection(double width, double height, double zNear, double zFar) {
	Matrix projection;
	double r = width / 2.0;
	double t = height / 2.0;
	projection[0][0] = zNear / r;
	projection[1][1] = zNear / t;
	projection[2][2] = -1*(zFar+zNear) /(zFar-zNear);
	projection[2][3] = -2*zFar*zNear / (zFar-zNear);
	projection[3][2] = -1;
	return projection;
}


static Vec3f viewport_transform(int width, int height, Vec3f ndc_coord) {
	double x = double((double)ndc_coord.x + 1.0f) * 0.5f * (double)width;   /* [-1, 1] -> [0, w] */
	double y = double((double)ndc_coord.y + 1.0f) * 0.5f * (double)height;  /* [-1, 1] -> [0, h] */
	double z = double((double)ndc_coord.z + 1.0f) * 0.5f;                  /* [-1, 1] -> [0, 1] */
	return Vec3f(x, y, z);
}





//void drawTriangle2D(std::vector<Vector<double>>& vertexBuffer, std::vector<Vector<int>>& colorBuffer, double* zbuffer, SDL_Renderer* gRenderer, SDL_Window* gWindow) {
//	int w = 0, h = 0;
//	SDL_GetWindowSize(gWindow, &w, &h);
//
//	std::vector<Vector<int>> screen_coords(3);
//	double screen_depths[3];
//	for (int i = 0; i < 3; i++) {
//		screen_coords[i] = Vector<int>((int)vertexBuffer[i][0], (int)vertexBuffer[i][1]);
//		screen_depths[i] = vertexBuffer[i][2];
//	}
//
//	Vector<int> bboxmin(w - 1, h - 1);
//	Vector<int> bboxmax(0, 0);
//	Vector<int> clamp(w - 1, h - 1);
//	for (int i = 0; i < 3; i++) {
//		bboxmin[0] = std::max(0, std::min(bboxmin[0], screen_coords[i][0]));
//		bboxmax[0] = std::min(clamp[0], std::max(bboxmax[0], screen_coords[i][0]));
//
//		bboxmin[1] = std::max(0, std::min(bboxmin[1], screen_coords[i][1]));
//		bboxmax[1] = std::min(clamp[1], std::max(bboxmax[1], screen_coords[i][1]));
//	}
//	//std::cout << "bboxmin[0]" << std::endl;
//	//bboxmin.print();
//	//bboxmax.print();
//	Vector<int> P(0, 0);
//	for (P[0] = bboxmin[0]; P[0] <= bboxmax[0]; P[0]++) {
//		for (P[1] = bboxmin[1]; P[1] <= bboxmax[1]; P[1]++) {
//			Vector<double> weights = calculate_weights(screen_coords, P);
//			int weight0_okay = weights[0] > -EPSILON;
//			int weight1_okay = weights[1] > -EPSILON;
//			int weight2_okay = weights[2] > -EPSILON;
//			if (weight0_okay && weight1_okay && weight2_okay) {
//				int zbufferInd = (int)P[0] + (int)P[1] * w;
//				double frag_depth = interpolate_depth(screen_depths, weights);
//				if (frag_depth <= zbuffer[zbufferInd]) {
//					zbuffer[zbufferInd] = frag_depth;
//
//					int depthColor = frag_depth * 255;
//					Vector<int> color = Vector<int>(depthColor, depthColor, depthColor, 255);
//					//std::cout << depthColor << std::endl;
//
//					SDLDrawPixel(gRenderer, gWindow, P[0], P[1], color);
//				}
//			};
//
//		}
//	}
//
//}