#include"myGL.h"

IShader::~IShader() {}

Matrix ModelMatrix;
Matrix ViewMatrix;
Matrix ProjectionMatrix;
Matrix MVP;
Matrix normalMatrix;
bool enableFaceCulling;
bool enableFrontFaceCulling;


bool enableZTest;
bool enableZWrite;

TGAColor white(255, 255, 255, 255);
TGAColor red = TGAColor(255, 0, 0, 255);

double LinearizeDepth(double depth, float near_plane, float far_plane)
{
	double z = depth * 2.0 - 1.0; // Back to NDC 
	return (2.0 * (double)near_plane * (double)far_plane) / ((double)far_plane + near_plane - z * ((double)far_plane - near_plane));
}

inline VerInf VerInf::verLerp(const VerInf& v1, const VerInf& v2, const float& factor)
{
	VerInf result;
	result.clip_coord = lerp(v1.clip_coord, v2.clip_coord, factor);
	result.world_pos = lerp(v1.world_pos, v2.world_pos, factor);
	result.uv = lerp(v1.uv, v2.uv, factor);
	result.normal = lerp(v1.normal, v2.normal, factor);
	result.ndc_coord = lerp(v1.ndc_coord, v2.ndc_coord, factor);
	//result.clipPosLightSpace = lerp(v1.clipPosLightSpace, v2.clipPosLightSpace, factor);
	return result;
}

//inline void setPixelBuffer(const int& x, const  int& y, const int& width, const int& height, TGAColor& color, Vec4f* drawBuffer) {
//	int bufferInd = x + y * 600;
//	drawBuffer[bufferInd].x = color[2];
//	drawBuffer[bufferInd].y = color[1];
//	drawBuffer[bufferInd].z = color[0];
//	drawBuffer[bufferInd].w = color[3];
//}

inline Vec4f blendColor(Vec4f& newColor, Vec4f& oldColor) {
	float alpha = newColor.w;
	newColor = newColor * alpha + oldColor * (1 - alpha);
	newColor.w = 1;
	return newColor;
}

void drawLine(int x0, int y0, int x1, int y1, TGAColor& color, int width, int height, Frame* drawBuffer) {
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
			drawBuffer->setPixel(y, x, color);
        }
        else {
			drawBuffer->setPixel(x, y, color);
			//setPixelBuffer(x, y, width, height, color, drawBuffer);
			//SDLDrawPixel(gRenderer, gWindow, x, y, color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}
void drawLine(Vec2i& a, Vec2i& b, TGAColor& color, int width, int height, Frame* drawBuffer) {
    drawLine(a.x, a.y, b.x, b.y, color, width,height, drawBuffer);
}
void drawLine(Vec3f& a, Vec3f& b, TGAColor& color, int width, int height, Frame* drawBuffer) {
	drawLine((int)a.x, (int)a.y, (int)b.x, (int)b.y, color, width, height, drawBuffer);
}

inline bool is_back_facing(Vec3f&a, Vec3f& b, Vec3f& c ) {
	/* 建立向量 b-a， a-c
		由于在ndc坐标系中，视线方向为（0，0，1）
		故cross(b-a, a-c)*(0,0,1)只会保留cross(b-a, a-c).z
		令signed_area = cross(b-a, a-c).z 计算背面剔除
	*/
	//Vec3f A = a - c;
	//Vec3f B = b - c;
	//float signed_area = cross(B,A).z;
	float signed_area = (b.x - c.x) * (a.y - c.y) - (b.y - c.y) * (a.x - c.x);
	if(enableFrontFaceCulling)
		return signed_area < 0;
	else
		return signed_area >= 0;
}
void draw2DFrame(VerInf** vertexs, TGAColor& color, const ViewPort& port, Frame* drawBuffer) {
	Vec2i screen_coords[3];
	/* viewport mapping */
	for (int i = 0; i < 3; i++) {
		Vec3f window_coord = port.transform(vertexs[i]->ndc_coord);
		screen_coords[i] = Vec2i((int)window_coord[0], (int)window_coord[1]);
	}

	drawLine(screen_coords[0], screen_coords[1], color, port.v_width, port.v_height, drawBuffer);
	drawLine(screen_coords[1], screen_coords[2], color, port.v_width, port.v_height, drawBuffer);
	drawLine(screen_coords[2], screen_coords[0], color, port.v_width, port.v_height, drawBuffer);
}

inline Vec3f calculate_weights(Vec2f& A, Vec2f& B, Vec2f& C, Vec2i& P) {
	Vec2f ab = B - A;//  = b - a;
	Vec2f ac = C - A;//  = c - a;
	Vec2f ap(P.x - A.x, P.y - A.y);//  = p - a;

	float factor = 1 / (ab.x * ac.y - ab.y * ac.x);
	float s = ((float)ac.y * ap.x - (float)ac.x * ap.y) * factor;
	float t = ((float)ab.x * ap.y - (float)ab.y * ap.x) * factor;

	return Vec3f(1.0f - s - t, s, t);
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
	Vec4f temp = v1 / v1.w;
	if (temp.x > 1 || temp.x < -1)
		return false;
	if (temp.y > 1 || temp.y < -1)
		return false;

	temp = v2 / v2.w;
	if (temp.x > 1 || temp.x < -1)
		return false;
	if (temp.y > 1 || temp.y < -1)
		return false;

	temp = v3 / v3.w;
	if (temp.x > 1 || temp.x < -1)
		return false;
	if (temp.y > 1 || temp.y < -1)
		return false;

	//if (v1.x > 1 || v1.x < -1)
	//	return false;
	//if (v1.y > 1 || v1.y < -1)
	//	return false;


	//if (v2.x > 1 || v2.x < -1)
	//	return false;
	//if (v2.y > 1 || v2.y < -1)
	//	return false;

	//if (v3.x > 1 || v3.x < -1)
	//	return false;
	//if (v3.y > 1 || v3.y < -1)
	//	return false;
	return true;
}

inline bool ClipSpaceCull(const Vec4f& v1, const Vec4f& v2, const Vec4f& v3, float near, float far) {
	if (v1.w < near && v2.w < near && v3.w < near)
		return false;
	if (v1.w > far && v2.w > far && v3.w > far)
		return false;
	if (fabs(v1[0]) <= fabs(v1.w) || fabs(v1[1]) <= fabs(v1.w) || fabs(v1[2]) <= fabs(v1.w))
		return true;
	if (fabs(v2[0]) <= fabs(v2.w) || fabs(v2[1]) <= fabs(v2.w) || fabs(v2[2]) <= fabs(v2.w))
		return true;
	if (fabs(v3[0]) <= fabs(v3.w) || fabs(v3[1]) <= fabs(v3.w) || fabs(v3[2]) <= fabs(v3.w))
		return true;
	return false;
}

//https://zhuanlan.zhihu.com/p/102758967
//在NDC中，一点 P [x/w, y/w, z/w] 与平面 Ax+By+Cz+D=0（法向量n=[A,B,C]） 
//的距离d = Ax/w + By/w + Cz/w + D
//若 d>0则在平面法向量所指区域内，
//d=0在平面上，d<0在另一侧区域。
//所有等式两次同乘w，d * w = Ax + By + Cz + Dw，依然是 d * w > 0就在我们要的区域内

// Ax + By + Cz + D > 0 就与法向量同侧
const Vec4f ViewLines[] = {
	//left
	Vec4f(-1,0,0,1),
	//right
	Vec4f(1,0,0,1),

	//top
	Vec4f(0,1,0,1),
	//bottom 
	Vec4f(0,-1,0,1),

	//Near
	Vec4f(0,0,1,1),
	//far
	Vec4f(0,0,-1,1),
};
inline bool Inside(const Vec4f& line, const Vec4f& p) {
	/*float rw = 1 / p.w;
	return (line.x * p.x * rw + line.y * p.y * rw + line.z * p.z * rw + line.w) > 0;*/
	return (line.x * p.x + line.y * p.y + line.z * p.z + line.w * p.w) >= 0;
}

//da*aw / (da*aw - db*bw)
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
	for (int i = 0; i <6; i++) {
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

void drawTriangle2D(VerInf** verInf, IShader& shader,const ViewPort& port, 
	const float& near, const float& far,
	double* zbuffer, Frame* drawBuffer,
	bool fog) {

	Vec2f screen_coords[3];
	double screen_depths[3];
	/* viewport mapping */
	for (int i = 0; i < 3; i++) {
		Vec3f window_coord = port.transform(verInf[i]->ndc_coord);
		screen_coords[i] = Vec2i(int(window_coord.x+0.5), int(window_coord.y+0.5));
		screen_depths[i] = window_coord.z;
	}

	Vec2i bboxmin(drawBuffer->f_width - 1, drawBuffer->f_height - 1);
	Vec2i bboxmax(0, 0);
	Vec2i bboxClamp(drawBuffer->f_width - 1, drawBuffer->f_height - 1);

	for (int i = 0; i < 3; i++) {
		bboxmin.x = std::max(0, std::min<int>(bboxmin.x, (int)screen_coords[i].x));
		bboxmin.y = std::max(0, std::min<int>(bboxmin.y, (int)screen_coords[i].y));

		bboxmax.x = std::min(bboxClamp.x, std::max<int>(bboxmax.x, (int)screen_coords[i].x));
		bboxmax.y = std::min(bboxClamp.y, std::max<int>(bboxmax.y, (int)screen_coords[i].y));
	}

	Vec2i P(0, 0);
	Vec4f color;
	VerInf temp;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f weights = calculate_weights(screen_coords[0], screen_coords[1], screen_coords[2], P);
			//weights = Vec3f(0.333, 0.3333, 0.33333);

			int weight0_okay = weights.x > -EPSILON;
			int weight1_okay = weights.y > -EPSILON;
			int weight2_okay = weights.z > -EPSILON;
			if (weight0_okay && weight1_okay && weight2_okay) {
				int zbufferInd = P.x + P.y * port.v_width;
				double frag_depth = 1.0;
				if (enableZTest)
					frag_depth = interpolate_depth(screen_depths, weights);
				temp.depth = frag_depth;


				if (frag_depth <= zbuffer[zbufferInd]) {
					//透视投影纠正
					for (int i = 0; i < 3; i++) weights[i] = weights[i] * (float)verInf[i]->recip_w;
					weights = weights / (weights[0] + weights[1] + weights[2]);
					
					//为面元着色器准备参数
					temp.uv = interpolate(verInf[0]->uv, verInf[1]->uv, verInf[2]->uv,weights);
					temp.world_pos = interpolate(verInf[0]->world_pos, verInf[1]->world_pos, verInf[2]->world_pos, weights);
					temp.normal = interpolate(verInf[0]->normal, verInf[1]->normal, verInf[2]->normal, weights);
					//temp.ndc_coord = interpolate(verInf[0]->ndc_coord, verInf[1]->ndc_coord, verInf[2]->ndc_coord,weights);
					
					temp.screen_coord.x = P.x;
					temp.screen_coord.y = P.y;

					//调用面元着色器
					bool discard = shader.fragment(temp, color);
					if (!discard) {
						//if (fog) {
						//	frag_depth = LinearizeDepth(frag_depth, near, far) / far;
						//	float fogRange = 0.3;
						//	float fogStartPos = 1 - fogRange;
						//	float rate = (frag_depth - fogStartPos) / fogRange;
						//	rate = clamp(rate, 0.0f, 1.0f);

						//	
						//	color[3] = (1 - rate) * 255;
						//}
						//进行alpha混合
						Vec4f* colorNow = drawBuffer->getPixel(P[0], P[1]);
						color = blendColor(color, *colorNow);
						//写入绘制buffer
						drawBuffer->setPixel(P[0], P[1], color);
						//setPixelBuffer(P[0], P[1], port.v_width, port.v_height, color, drawBuffer);
						//更新zbuffer
						if(enableZWrite)
							zbuffer[zbufferInd] = frag_depth;
					}
				}
			}

		}
	}
}

void triangle(VerInf* vertexs, IShader& shader,
	const ViewPort& port, const float& near, const float& far,
	double* zbuffer, Frame* drawBuffer,
	bool farme, bool fog) {
	if (!ClipSpaceCull(vertexs[0].clip_coord, vertexs[1].clip_coord, vertexs[2].clip_coord, near,far)) {
		return;
	}

	VerInf *verList[3];
	std::vector<VerInf> clipingVertexs = SutherlandHodgeman(vertexs[0], vertexs[1], vertexs[2]);

	/* perspective division */
	for (int i = 0; i < clipingVertexs.size(); i++) {
		clipingVertexs[i].recip_w = 1.0f / clipingVertexs[i].clip_coord.w;
		clipingVertexs[i].ndc_coord = proj<3>(clipingVertexs[i].clip_coord) * clipingVertexs[i].recip_w;
	}

	int n = int(clipingVertexs.size() - 3 + 1);
	for (int i = 0; i < n; i++) {
		verList[0] = &clipingVertexs[0];
		verList[1] = &clipingVertexs[__int64(i) + 1];
		verList[2] = &clipingVertexs[__int64(i) + 2];

		if (enableFaceCulling && is_back_facing(verList[0]->ndc_coord, verList[1]->ndc_coord, verList[2]->ndc_coord)) {
			return;
		}
		if (farme)
			draw2DFrame(verList, white, port, drawBuffer);
		else
			drawTriangle2D(verList, shader, port, near,far, zbuffer, drawBuffer, fog);
	}

}

Matrix translate(float x, float y, float z) {
	Matrix t;
	t = Matrix::identity();
	t[0][3] = x;
	t[1][3] = y;
	t[2][3] = z;
	return t;
};

Matrix scale(float x, float y, float z) {
	Matrix t;
	t= Matrix::identity();
	t[0][0] = x;
	t[1][1] = y;
	t[2][2] = z;
	return t;
};

Matrix rotate(Vec3f &axis, float theta) {
	Matrix t;
	axis.normalize();

	theta = theta * PI / 180.0f;
	float cosTheta = std::cos(theta);
	float sinTheta = std::sin(theta);

	float u = axis[0];
	float v = axis[1];
	float w = axis[2];

	t[0][0] = cosTheta + u * u * (1.0f - cosTheta);
	t[0][1] = u * v * (1.0f - cosTheta) + w * sinTheta;
	t[0][2] = u * w * (1.0f - cosTheta) - v * sinTheta;
	t[0][3] = 0.0f;

	t[1][0] = u * v * (1.0f - cosTheta) - w * sinTheta;
	t[1][1] = cosTheta + v * v * (1 - cosTheta);
	t[1][2] = w * v * (1.0f - cosTheta) + u * sinTheta;
	t[1][3] = 0.0f;

	t[2][0] = u * w * (1.0f - cosTheta) + v * sinTheta;
	t[2][1] = v * w * (1.0f - cosTheta) - u * sinTheta;
	t[2][2] = cosTheta + w * w * (1.0f - cosTheta);
	t[2][3] = 0.0f;

	t[3][0] = 0.0f;
	t[3][1] = 0.0f;
	t[3][2] = 0.0f;
	t[3][3] = 1.0f;

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



//Matrix setFrustum(double left, double right, double bottom, double top, double near, double far) {
//	double x_range = right - left;
//	double y_range = top - bottom;
//	double z_range = far - near;
//
//	Matrix matrix;
//	matrix= Matrix::identity();
//	matrix[0][0] = 2.0 * near / x_range;
//	matrix[0][0] = 2.0 * near / y_range;
//	matrix[0][0] = (left + right) / x_range;
//	matrix[0][0] = (bottom + top) / y_range;
//	matrix[0][0] = -(near + far) / z_range;
//	matrix[0][0] = -2.0 * near * far / z_range;
//	matrix[0][0] = -1.0;
//	matrix[0][0] = 0.0;
//	return matrix;
//}

Matrix setFrustum(float fovy, float aspect, float near, float far)
{
	float z_range = far - near;
	Matrix matrix;
	matrix= Matrix::identity();
	assert(fovy > 0 && aspect > 0);
	assert(near > 0 && far > 0 && z_range > 0);
	matrix[1][1] = 1 / (float)tan(fovy / 2);
	matrix[0][0] = matrix[1][1] / aspect;
	matrix[2][2] = -(near + far) / z_range;
	matrix[2][3] = -2.0f * near * far / z_range;
	matrix[3][2] = -1;
	matrix[3][3] = 0;

	return matrix;
}

Matrix mat4_orthographic(float right, float top, float near, float far) {
	float z_range = far - near;
	Matrix matrix = Matrix::identity();
	assert(right > 0 && top > 0 && z_range > 0);
	matrix[0][0] = 1 / right;
	matrix[1][1] = 1 / top;
	matrix[2][2] = -2 / z_range;
	matrix[2][3] = -(near + far) / z_range;
	return matrix;
}


Matrix projection(float width, float height, float zNear, float zFar) {
	Matrix projection;
	float r = width / 2.0f;
	float t = height / 2.0f;
	projection[0][0] = zNear / r;
	projection[1][1] = zNear / t;
	projection[2][2] = -1.0f *(zFar+zNear) /(zFar-zNear);
	projection[2][3] = -2.0f*zFar*zNear / (zFar-zNear);
	projection[3][2] = -1.0f;
	return projection;
}

Vec4f CubeMap(TGAImage* skyboxFaces, Vec3f dir) {
	int maxDir = fabs(dir.x) > fabs(dir.y) ? 0 : 1;
	maxDir = fabs(dir[maxDir]) > fabs(dir.z) ? maxDir : 2;
	dir = dir * abs(1 / dir[maxDir]);
	if (dir[maxDir] < 0)maxDir = maxDir * 2 + 1;
	else maxDir *= 2;
	float screen_coord[2];
	for (int i = 0; i < 3; i++) {
		dir[i] = (dir[i] + 1) * 0.5f;
	}
	switch (maxDir) {
	case 0://+x
		screen_coord[0] = 1 - dir.z;
		screen_coord[1] = 1 - dir.y;
		break;
	case 1://-x
		screen_coord[0] = dir.z;
		screen_coord[1] = 1 - dir.y;
		break;
	case 2://+y
		screen_coord[0] = dir.x;
		screen_coord[1] = dir.z;
		break;
	case 3://-y
		screen_coord[0] = dir.x;
		screen_coord[1] = 1 - dir.z;
		break;
	case 4://+z
		screen_coord[0] = dir.x;
		screen_coord[1] = 1 - dir.y;
		break;
	case 5://-z
		screen_coord[0] = 1 - dir.x;
		screen_coord[1] = 1 - dir.y;
		break;
	}
	screen_coord[0] = screen_coord[0] - (float)floor(screen_coord[0]);
	screen_coord[1] = screen_coord[1] - (float)floor(screen_coord[1]);

	screen_coord[0] = screen_coord[0] * skyboxFaces[maxDir].get_width();
	screen_coord[1] = screen_coord[1] * skyboxFaces[maxDir].get_height();


	TGAColor cubeColor = skyboxFaces[maxDir].get(int(screen_coord[0]), int(screen_coord[1]));
	//std::cout << "dir:" << pos << " maxDir:" << maxDir <<" uv:"<< screen_coord[0]<<","<< screen_coord[1]
	//	<< " Color " << cubeColor[2]<<","<< cubeColor[1] << "," << cubeColor[0] << std::endl;
	return Vec4f(cubeColor.bgra[2]/255.f, cubeColor.bgra[1] / 255.f, cubeColor.bgra[0] / 255.f, cubeColor.bgra[3] / 255.f);
}

Vec3f ViewPort::transform(Vec3f& ndc_coord)const
{
	float x = v_pos.x + (ndc_coord.x + 1.0f) * 0.5f * (float)v_width;   /* [-1, 1] -> [0, w] */
	float y = v_pos.y + (ndc_coord.y + 1.0f) * 0.5f * (float)v_height;  /* [-1, 1] -> [0, h] */
	float z = (ndc_coord.z + 1.0f) * 0.5f;                  /* [-1, 1] -> [0, 1] */
	return Vec3f(x, y, z);
}



Frame::Frame(int w, int h)
{
	f_width = w;
	f_height = h;
	buffer = new Vec4f[f_width * f_height];
}

Frame::~Frame()
{
	delete[] buffer;
}
void Frame::setPixel(int& x, int& y, TGAColor& color)
{
	int bufferInd = x + y * f_width;
	buffer[bufferInd].x = color[2];
	buffer[bufferInd].y = color[1];
	buffer[bufferInd].z = color[0];
	buffer[bufferInd].w = color[3];
}

void Frame::setPixel(int& x, int& y, Vec4f& color)
{
	int bufferInd = x + y * f_width;
	buffer[bufferInd].x = color.x;
	buffer[bufferInd].y = color.y;
	buffer[bufferInd].z = color.z;
	buffer[bufferInd].w = color.w;
}

Vec4f* Frame::getPixel(int& x, int& y) {
	int bufferId = x + y * f_width;
	return &buffer[bufferId];
}

void Frame::fill(const Vec4f& defaultColor)
{
	std::fill(buffer, buffer + (__int64(f_width) * __int64(f_height)), defaultColor);
}

//https://www.cnblogs.com/bluebean/p/5299358.html
Vec3f reflect(Vec3f& I, Vec3f& N) {
	Vec3f V =  N * 2 * (N * (-1) * I);
	return (I + V * 1.45);
}
