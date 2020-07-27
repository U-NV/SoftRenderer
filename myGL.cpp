#include"myGL.h"

IShader::~IShader() {}

Matrix ModelMatrix;
Matrix ViewMatrix;
Matrix ProjectionMatrix;
Matrix MVP;

TGAColor white(255, 255, 255, 255);
TGAColor red = TGAColor(255, 0, 0, 255);

inline VerInf VerInf::verLerp(const VerInf& v1, const VerInf& v2, const float& factor)
{
	VerInf result;
	result.clip_coord = lerp(v1.clip_coord, v2.clip_coord, factor);
	result.world_pos = lerp(v1.world_pos, v2.world_pos, factor);
	result.uv = lerp(v1.uv, v2.uv, factor);
	result.normal = lerp(v1.normal, v2.normal, factor);
	result.ndc_coord = lerp(v1.ndc_coord, v2.ndc_coord, factor);

	return result;
}

inline void setPixelBuffer(int& x, int& y, int& width, int& height, TGAColor& color, ColorVec* drawBuffer) {
	int bufferInd = x + y * width;
	drawBuffer[bufferInd].x = color[2];
	drawBuffer[bufferInd].y = color[1];
	drawBuffer[bufferInd].z = color[0];
	drawBuffer[bufferInd].w = color[3];
}

inline TGAColor blendColor(TGAColor& newColor, TGAColor& oldColor) {
	float alpha = newColor[3] / 255.0f;
	newColor = newColor * alpha + oldColor * (1 - alpha);
	return newColor;
}

void drawLine(int x0, int y0, int x1, int y1, TGAColor& color, int width, int height, ColorVec* drawBuffer) {
	x0 = clamp(x0, 0, width-1);
	x1 = clamp(x1, 0, width-1);

	y0 = clamp(y0, 0, height-1);
	y1 = clamp(y1, 0, height-1);
	
	
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
			setPixelBuffer(y, x, width, height, color, drawBuffer);
			//SDLDrawPixel(gRenderer, gWindow, y, x, color);
        }
        else {
			setPixelBuffer(x, y, width, height, color, drawBuffer);
			//SDLDrawPixel(gRenderer, gWindow, x, y, color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}
void drawLine(Vec2i& a, Vec2i& b, TGAColor& color, int width, int height, ColorVec* drawBuffer) {
    drawLine(a.x, a.y, b.x, b.y, color, width,height, drawBuffer);
}
void drawLine(Vec3f& a, Vec3f& b, TGAColor& color, int width, int height, ColorVec* drawBuffer) {
	drawLine(a.x, a.y, b.x, b.y, color, width, height, drawBuffer);
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
void draw2DFrame(VerInf** vertexs, TGAColor& color, int width, int height, ColorVec* drawBuffer) {
	Vec2i screen_coords[3];
	/* viewport mapping */
	for (int i = 0; i < 3; i++) {
		Vec3f window_coord = viewport_transform(width, height, vertexs[i]->ndc_coord);
		screen_coords[i] = Vec2i((int)window_coord[0], (int)window_coord[1]);
	}

	drawLine(screen_coords[0], screen_coords[1], color, width, height, drawBuffer);
	drawLine(screen_coords[1], screen_coords[2], color, width, height, drawBuffer);
	drawLine(screen_coords[2], screen_coords[0], color, width, height, drawBuffer);
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

inline bool AllVertexsInside(const Vec4f& v1, const Vec4f& v2, const Vec4f& v3) {
	if (v1.x > 1 || v1.x < -1)
		return false;
	if (v1.y > 1 || v1.y < -1)
		return false;
	if (v2.x > 1 || v2.x < -1)
		return false;
	if (v2.y > 1 || v2.y < -1)
		return false;
	if (v3.x > 1 || v3.x < -1)
		return false;
	if (v3.y > 1 || v3.y < -1)
		return false;
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

// Ax + By + C < 0 就在内侧
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
inline bool Inside(const Vec4f& line, const Vec4f& p) {
	return line.x * p.x + line.y * p.y + line.z * p.z + line.w * p.w >= 0;
}
inline VerInf Intersect(const VerInf& v1, const VerInf& v2, const Vec4f& line) {
	float da = v1.clip_coord.x * line.x + v1.clip_coord.y * line.y + v1.clip_coord.z * line.z + line.w * v1.clip_coord.w;
	float db = v2.clip_coord.x * line.x + v2.clip_coord.y * line.y + v2.clip_coord.z * line.z + line.w * v2.clip_coord.w;

	float weight = da / (da - db);
	return VerInf::verLerp(v1, v2, weight);
}
std::vector<VerInf> SutherlandHodgeman(const VerInf& v1, const VerInf& v2, const VerInf& v3) {
	std::vector<VerInf> output = { v1,v2,v3 };
	if (AllVertexsInside(v1.clip_coord, v2.clip_coord, v3.clip_coord)) {
		return output;
	}
	for (int i = 0; i <5; i++) {
		std::vector<VerInf> input(output);
		output.clear();
		for (int j = 0; j < input.size(); j++) {
			VerInf current = input[j];
			VerInf last = input[(j + input.size() - 1) % input.size()];
			if (Inside(ViewLines[i], current.clip_coord)) {
				if (!Inside(ViewLines[i], last.clip_coord)) {
					VerInf intersecting = Intersect(last, current, ViewLines[i]);
					output.push_back(intersecting);
				}
				output.push_back(current);
			}
			else if (Inside(ViewLines[i], last.clip_coord)) {
				VerInf intersecting = Intersect(last, current, ViewLines[i]);
				output.push_back(intersecting);
			}
		}
	}
	return output;
}

void drawTriangle2D(VerInf** verInf, IShader& shader, int &width, int &height, double* zbuffer, ColorVec* drawBuffer) {
	Vec2i screen_coords[3];
	double screen_depths[3];
	
	/* viewport mapping */
	for (int i = 0; i < 3; i++) {
		Vec3f window_coord = viewport_transform(width, height, verInf[i]->ndc_coord);
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
					//透视投影纠正
					for (int i = 0; i < 3; i++) weights[i] = weights[i] * verInf[i]->recip_w;
					weights = weights / (weights[0] + weights[1] + weights[2]);
					
					//为面元着色器准备参数
					VerInf temp;
					temp.uv = interpolate(verInf[0]->uv, verInf[1]->uv, verInf[2]->uv,weights);
					temp.world_pos = interpolate(verInf[0]->world_pos, verInf[1]->world_pos, verInf[2]->world_pos,weights);
					temp.normal = interpolate(verInf[0]->normal, verInf[1]->normal, verInf[2]->normal,weights);

					//调用面元着色器
					bool discard = shader.fragment(temp, color);
					//std::cout << frag_depth << " : " << frag_depth * 255 << std::endl;
					//color = TGAColor(frag_depth * 255, frag_depth * 255, frag_depth * 255, 255);
					if (!discard) {
						//更新zbuffer
						zbuffer[zbufferInd] = frag_depth;
						//进行alpha混合
						TGAColor colorNow(drawBuffer[zbufferInd].x, drawBuffer[zbufferInd].y, drawBuffer[zbufferInd].z, drawBuffer[zbufferInd].w);
						color = blendColor(color, colorNow);
						//写入绘制buffer
						setPixelBuffer(P[0], P[1], width, height, color, drawBuffer);
					}
				}
			}

		}
	}
}

void triangle(bool farme,VerInf* vertexs, IShader& shader, int width, int height, double* zbuffer, ColorVec* drawBuffer) {
	if (!ClipSpaceCull(vertexs[0].clip_coord, vertexs[1].clip_coord, vertexs[2].clip_coord)) {
		return;
	}
	
	VerInf *verList[3];
	std::vector<VerInf> clipingVertexs = SutherlandHodgeman(vertexs[0], vertexs[1], vertexs[2]);

	/* perspective division */
	for (int i = 0; i < clipingVertexs.size(); i++) {
		clipingVertexs[i].recip_w = 1 / clipingVertexs[i].clip_coord[3];
		clipingVertexs[i].ndc_coord = proj<3>(clipingVertexs[i].clip_coord) * clipingVertexs[i].recip_w;
	}

	int n = clipingVertexs.size() - 3 + 1;
	for (int i = 0; i < n; i++) {
		verList[0] = &clipingVertexs[0];
		verList[1] = &clipingVertexs[i + 1];
		verList[2] = &clipingVertexs[i + 2];

		/* back-face culling */
		if (is_back_facing(verList[0]->ndc_coord, verList[1]->ndc_coord, verList[2]->ndc_coord)) {
			//std::cout << "back_facing" << std::endl;
			return;
		}
		if (farme)
			draw2DFrame(verList, white, width,height,drawBuffer);
		else
			drawTriangle2D(verList, shader,  width, height, zbuffer, drawBuffer);
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

Vec3f viewport_transform(int width, int height, Vec3f ndc_coord) {
	double x = double((double)ndc_coord.x + 1.0f) * 0.5f * (double)width;   /* [-1, 1] -> [0, w] */
	double y = double((double)ndc_coord.y + 1.0f) * 0.5f * (double)height;  /* [-1, 1] -> [0, h] */
	double z = double((double)ndc_coord.z + 1.0f) * 0.5f;                  /* [-1, 1] -> [0, 1] */
	return Vec3f(x, y, z);
}


