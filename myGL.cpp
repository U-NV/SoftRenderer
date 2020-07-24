#include"myGL.h"

IShader::~IShader() {}

Matrix ModelMatrix;
Matrix ViewMatrix;
Matrix ProjectionMatrix;
Matrix MVP;

int width = 0, height = 0;

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

inline bool is_back_facing(Vec3f&a, Vec3f& b, Vec3f& c ) {
	/* 建立向量 b-a， c-a
		由于在ndc坐标系中，实现方向为（0，0，1）
		故cross(b-a, c-a)*(0,0,1)只会保留cross(b-a, c-a).z
		令signed_area = cross(b-a, c-a).z 计算背面剔除
	*/

	float signed_area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
	return signed_area <= 0;
}
void draw2DFrame(VerInf* vertexs, TGAColor& color, SDL_Renderer* gRenderer, SDL_Window* gWindow) {
	SDL_GetWindowSize(gWindow, &width, &height);
	/* reciprocals of w */
	for (int i = 0; i < 3; i++) {
		vertexs[i].recip_w = 1 / vertexs[i].clip_coord[3];
	}
	/* perspective division */
	for (int i = 0; i < 3; i++) {
		vertexs[i].ndc_coord = proj<3>(vertexs[i].clip_coord) * vertexs[i].recip_w;
	}
	/* back-face culling */
	if (is_back_facing(vertexs[0].ndc_coord, vertexs[1].ndc_coord, vertexs[2].ndc_coord)) {
		//std::cout << "back_facing" << std::endl;
		return;
	}

	Vec2i screen_coords[3];
	/* viewport mapping */
	for (int i = 0; i < 3; i++) {
		Vec3f window_coord = viewport_transform(width, height, vertexs[i].ndc_coord);
		screen_coords[i] = Vec2i((int)window_coord[0], (int)window_coord[1]);
	}

	drawLine(screen_coords[0], screen_coords[1], color, gRenderer, gWindow);
	drawLine(screen_coords[1], screen_coords[2], color, gRenderer, gWindow);
	drawLine(screen_coords[2], screen_coords[0], color, gRenderer, gWindow);
}

inline Vec3f calculate_weights(Vec2i& A, Vec2i& B, Vec2i& C, Vec2i& P) {
	Vec2f ab = B - A;//  = b - a;
	Vec2f ac = C - A;//  = c - a;
	Vec2f ap = P - A;//  = p - a;

	double factor = 1 / (ab.x * ac.y - ab.y * ac.x);
	double s = ((double)ac.y * ap.x - (double)ac.x * ap.y) * factor;
	double t = ((double)ab.x * ap.y - (double)ab.y * ap.x) * factor;

	return Vec3f(1 - s - t, s, t);
}

inline double interpolate_depth(double* screen_depths, Vec3f& weights) {
	double depth0 = screen_depths[0] * weights.x;
	double depth1 = screen_depths[1] * weights.y;
	double depth2 = screen_depths[2] * weights.z;
	return depth0 + depth1 + depth2;
}

inline Vec2f interpolate(Vec2f& a,Vec2f& b,Vec2f& c, Vec3f& weights) {
	return Vec2f(
		a.x * weights.x + b.x * weights.y + c.x * weights.z,
		a.y * weights.x + b.y * weights.y + c.y * weights.z
	);
}

inline Vec3f interpolate(Vec3f& a, Vec3f& b, Vec3f& c, Vec3f& weights) {
	return Vec3f(
		a.x * weights.x + b.x * weights.y + c.x * weights.z,
		a.y * weights.x + b.y * weights.y + c.y * weights.z,
		a.z * weights.x + b.z * weights.y + c.z * weights.z
	);
}


inline bool AllVertexsInside(const std::vector<Vec2f> &ver) {
	for (int i = 0; i < 3; i++) {
		if (ver[i].x > 1 || ver[i].x < -1)
			return false;
		if (ver[i].y > 1 || ver[i].y < -1)
			return false;
	}
	return true;
}

inline bool ClipSpaceCull(const Vec4f& v1, const Vec4f& v2, const Vec4f& v3) {
	float near = defaultCamera.NEAR;
	float far = defaultCamera.FAR;
	if (v1[3] < near && v2[3] < near && v3[3] < near)
		return false;
	if (v1[3] > far && v2[3] > far && v3[3] > far)
		return false;
	if (fabs(v1[0]) <= fabs(v1[3]) || fabs(v1[1]) <= fabs(v1[3]) || fabs(v1[2]) <= fabs(v1[3]))
		return true;
	if (fabs(v2[0]) <= fabs(v2[3]) || fabs(v2[1]) <= fabs(v2[3]) || fabs(v2[2]) <= fabs(v2[3]))
		return true;
	if (fabs(v3[0]) <= fabs(v3[3]) || fabs(v3[1]) <= fabs(v3[3]) || fabs(v3[2]) <= fabs(v3[3]))
		return true;
	return false;
}



const Vec4f ViewLines[] = {
	//Near
	Vec4f(0,0,1,1),
	//far
	Vec4f(0,0,-1,1),
	//left
	Vec4f(1,0,0,1),
	//top
	Vec4f(0,1,0,1),
	//right
	Vec4f(-1,0,0,1),
	//bottom 
	Vec4f(0,-1,0,1)
};
bool Inside(const Vec4f& line, const Vec4f& p) {
	return line.x * p.x + line.y * p.y + line.z * p.z + line.w * p.w >= 0;
}
//交点，通过端点插值
//Vec2f Intersect(const V2F& v1, const V2F& v2, const Vec4f& line) {
//	float da = v1.windowPos.x * line.x + v1.windowPos.y * line.y + v1.windowPos.z * line.z + line.w * v1.windowPos.w;
//	float db = v2.windowPos.x * line.x + v2.windowPos.y * line.y + v2.windowPos.z * line.z + line.w * v2.windowPos.w;
//
//	float weight = da / (da - db);
//	return V2F::lerp(v1, v2, weight);
//}
//std::vector<Vec2f> SutherlandHodgeman(const Vec2f& v1, const Vec2f& v2, const Vec2f& v3) {
//	std::vector<Vec2f> output = { v1,v2,v3 };
//	if (AllVertexsInside(output)) {
//		return output;
//	}
//	for (int i = 0; i < 5; i++) {
//		std::vector<Vec2f> input(output);
//		output.clear();
//		for (int j = 0; j < input.size(); j++) {
//			Vec2f current = input[j];
//			Vec2f last = input[(j + input.size() - 1) % input.size()];
//			if (Inside(ViewLines[i], current.windowPos)) {
//				if (!Inside(ViewLines[i], last.windowPos)) {
//					V2F intersecting = Intersect(last, current, ViewLines[i]);
//					output.push_back(intersecting);
//				}
//				output.push_back(current);
//			}
//			else if (Inside(ViewLines[i], last.windowPos)) {
//				Vec2f intersecting = Intersect(last, current, ViewLines[i]);
//				output.push_back(intersecting);
//			}
//		}
//	}
//	return output;
//}

void drawTriangle2D(VerInf* verInf, IShader& shader, double* zbuffer, SDL_Renderer* gRenderer, SDL_Window* gWindow) {
	Vec2i screen_coords[3];
	double screen_depths[3];
	
	/* viewport mapping */
	for (int i = 0; i < 3; i++) {
		Vec3f window_coord = viewport_transform(width, height, verInf[i].ndc_coord);
		screen_coords[i] = Vec2i((int)window_coord.x, (int)window_coord.y);
		screen_depths[i] = window_coord.z;
	}

	Vec2i bboxmin(width - 1, height - 1);
	Vec2i bboxmax(0, 0);
	Vec2i bboxClamp(width - 1, height - 1);

	for (int i = 0; i < 3; i++) {
		bboxmin.x = std::max(0, std::min(bboxmin.x, screen_coords[i].x));
		bboxmin.y = std::max(0, std::min(bboxmin.y, screen_coords[i].y));

		bboxmax.x = std::min(bboxClamp.x, std::max(bboxmax.x, screen_coords[i].x));
		bboxmax.y = std::min(bboxClamp.y, std::max(bboxmax.y, screen_coords[i].y));
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
					for (int i = 0; i < 3; i++) weights[i] = weights[i] * verInf[i].recip_w;
					weights = weights / (weights[0] + weights[1] + weights[2]);
					
					VerInf temp;
					temp.uv = interpolate(verInf[0].uv, verInf[1].uv, verInf[2].uv,weights);
					temp.world_pos = interpolate(verInf[0].world_pos, verInf[1].world_pos, verInf[2].world_pos,weights);
					temp.normal = interpolate(verInf[0].normal, verInf[1].normal, verInf[2].normal,weights);

					bool discard = shader.fragment(temp, color);
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

void triangle(VerInf* vertexs, IShader& shader, double* zbuffer, SDL_Renderer* gRenderer, SDL_Window* gWindow) {
	SDL_GetWindowSize(gWindow, &width, &height);

	/* reciprocals of w */
	for (int i = 0; i < 3; i++) {
		vertexs[i].recip_w = 1 / vertexs[i].clip_coord[3];
	}
	/* perspective division */
	for (int i = 0; i < 3; i++) {
		vertexs[i].ndc_coord = proj<3>(vertexs[i].clip_coord) * vertexs[i].recip_w;
	}
	/* back-face culling */
	if (is_back_facing(vertexs[0].ndc_coord, vertexs[1].ndc_coord, vertexs[2].ndc_coord)) {
		//std::cout << "back_facing" << std::endl;
		return;
	}

	drawTriangle2D(vertexs, shader, zbuffer, gRenderer, gWindow);
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