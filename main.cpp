#include <vector>
#include <limits>
#include <cmath>
#include <iostream>
#include <time.h>
#include "myGL.h"
#include "model.h"
#include "KeyboardAndMouseHandle.h"
#include "SDLWindow.h"

//Screen dimension constants
const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 400;

const int SHADOW_WIDTH = 1000, SHADOW_HEIGHT = 1000;
//位置相关参数
Vec3f up(0, 1, 0);
Vec3f right(1, 0, 0);
Vec3f forword(0, 0, 1);
Vec3f center(0, 0, 0);
Vec3f camDefaultPos(0, 0, 1);

//颜色
const ColorVec backGroundColor(100, 30, 0x00, 0xFF);
const ColorVec white(0xFF, 0xFF, 0xFF, 0xFF);

//光源位置
Vec3f lightPos(1, 3, 1);

//模型指针
Model* model;

//计数器
clock_t lastFrame = 0;
float deltaTime = 0;

//摄像机
Camera defaultCamera;
Camera lightCamera;

struct Shader : public IShader {
	Matrix *LightSpaceMatrix;
	double* ShadowBuffer;
	Shader(Matrix *lightSpaceMatrix, double* shadowBuffer) {
		LightSpaceMatrix = lightSpaceMatrix;
		ShadowBuffer = shadowBuffer;
	}
	virtual void vertex(int iface, int nthvert, VerInf& faceVer) {//对第iface的第nthvert个顶点进行变换
		//获得模型uv
		faceVer.uv = model->uv(iface, nthvert);
		//法线变换
		faceVer.normal = proj<3>(MVP.invert_transpose() * embed<4>(model->normal(iface, nthvert), 0.0));
		//世界坐标
		faceVer.world_pos = proj<3>(ModelMatrix * embed<4>(model->vert(iface, nthvert)));
		//裁切坐标
		faceVer.clip_coord = MVP * embed<4>(model->vert(iface, nthvert));
	}

	float inShadow(Vec3f& world_pos, float& bias) {
		//计算顶点在光照坐标系的坐标
		Vec4f clipPosLightSpace = *LightSpaceMatrix * embed<4>(world_pos);
		//进行透视除法
		clipPosLightSpace.w = std::max(0.000000001, clipPosLightSpace.w);
		float light_recip_w = 1 / clipPosLightSpace.w;
		Vec3f ndcLightSpace = proj<3>(clipPosLightSpace * light_recip_w);
		//因为没有进行各种裁切，故这里ndc坐标会溢出
		//这里默认超出贴图的地方都可以受到光照，直接返回0
		if (ndcLightSpace.x > 1.0 || ndcLightSpace.x < -1.0)return 0;
		if (ndcLightSpace.y > 1.0 || ndcLightSpace.y < -1.0)return 0;
		if (ndcLightSpace.z > 1.0 || ndcLightSpace.z < -1.0)return 0;
		//ndcLightSpace.x = clamp(ndcLightSpace.x, -1.0, 1.0);
		//ndcLightSpace.y = clamp(ndcLightSpace.y, -1.0, 1.0);
		//ndcLightSpace.z = clamp(ndcLightSpace.z, -1.0, 1.0);

		//变换到阴影贴图的范围
		Vec3f shadowTextureCoords = viewport_transform(SHADOW_WIDTH, SHADOW_HEIGHT, ndcLightSpace);
		//从缓存中得到最近的深度
		int x = clamp((int)shadowTextureCoords.x, 0, SHADOW_WIDTH - 1);
		int y = clamp((int)shadowTextureCoords.y, 0, SHADOW_HEIGHT - 1);
		int id = x + y* SHADOW_WIDTH;
		float closestDepth = ShadowBuffer[id];
		//取得当前片元在光源视角下的深度
		float currentDepth = shadowTextureCoords.z;
		if (defaultCamera.getProjectMode()) {
			//透视投影中需要对z值线性化，避免冲突
			float near = defaultCamera.getNear();
			float far = defaultCamera.getFar();
			closestDepth = LinearizeDepth(closestDepth, near, far);
			currentDepth = LinearizeDepth(currentDepth, near, far);
			//std::cout << "ProjectionMode" << defaultCamera.ProjectionMode << std::endl;
		}
		//std::cout << "closestDepth"<< closestDepth <<std::endl;
		//std::cout << "currentDepth"<< currentDepth <<std::endl;
		//// 检查当前片元是否在阴影中
		return currentDepth - bias > closestDepth ? 1.0 : 0.0;
		//return 0;
	}
	
	virtual bool fragment(VerInf verInf, TGAColor& color) {
		//插值uv
		Vec2f uv = verInf.uv;

		//插值法向量
		//Vec3f normal = verInf.normal;

		//使用法线贴图
		Vec3f normal = model->normal(uv);
		{
		//Vec3f bn = (varying_nrm * weights).normalize();
		//mat<3, 3, float> A;
		//A[0] = ndc_tri.col(1) - ndc_tri.col(0);
		//A[1] = ndc_tri.col(2) - ndc_tri.col(0);
		//A[2] = bn;

		//mat<3, 3, float> AI = A.invert();
		//Vec3f i = AI * Vec3f(varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0);
		//Vec3f j = AI * Vec3f(varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0);

		//mat<3, 3, float> B;
		//B.set_col(0, i.normalize());
		//B.set_col(1, j.normalize());
		//B.set_col(2, bn);
		}

		//设置光照
		float lightPower = 6;
		//环境光
		float ambient = 0.5f;
		Vec3f vertPos = verInf.world_pos;
		
		//衰减
		Vec3f lightDir = (lightPos - vertPos);
		float dis = lightDir.norm();
		float recip_dis = 1.0f / dis;
		lightDir = lightDir * recip_dis;

		//漫反射
		float diff = clamp<float>(std::abs(lightDir * (-1) * normal), 0, 1);

		//镜面反射
		Vec3f viewDir = (defaultCamera.getPos() - vertPos).normalize();
		Vec3f hafe = (lightDir + viewDir).normalize();
		float spec = clamp<float>(std::abs(normal * hafe), 0, 1);

		
		//设置阴影偏移
		float bias = std::max(0.1 * (1.0 - diff), 0.005);
		//计算该点在不在阴影区域
		float isInShadow = inShadow(verInf.world_pos, bias);

		TGAColor c = model->diffuse(uv);
		color = c;
		for (int i = 0; i < 3; i++) {
			float light = (1 - isInShadow) * (1 * diff + 1* spec ) * lightPower * recip_dis * recip_dis;
			//float light = (1 * diff + 1 * spec) * lightPower;
			float rate = clamp<float>(ambient + light,0,10);
			float tempColor = c[i] * rate;
			color[i] = clamp<int>(tempColor,0,255);
		}

		return false;
	}
};

struct DepthShader : public IShader {
	Matrix* LightSpaceMatrix;
	DepthShader(Matrix* lightSpaceMatrix) {
		LightSpaceMatrix = lightSpaceMatrix;
	}
	virtual void vertex(int iface, int nthvert, VerInf& faceVer) {
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
		faceVer.clip_coord = *LightSpaceMatrix * ModelMatrix  * gl_Vertex; 
	}

	virtual bool fragment(VerInf verInf, TGAColor& color) {
		if (lightCamera.getProjectMode()) {
			float far = lightCamera.getFar();
			float near = lightCamera.getNear();
			verInf.depth = LinearizeDepth(verInf.depth,near, far)/ far;
		}
		color = TGAColor(255, 255, 255,255) * verInf.depth;
		return false;
	}
};

void drawShadowMap(std::vector<Model>& models, double* shadowBuffer, ColorVec* shadowTexture) {
	//清空shadowMap数据，重新绘制
	std::fill(shadowBuffer, shadowBuffer + SHADOW_WIDTH * SHADOW_HEIGHT, 1);
	std::fill(shadowTexture, shadowTexture + SHADOW_WIDTH * SHADOW_WIDTH, white);

	//将摄像机摆放到光源位置
	//lightCamera.enableProjectMode(false);
	Vec3f lightPos2 = lightPos;
	lightCamera.setCamera(lightPos2, center, up);
	lightCamera.setClipPlane(2,6);
	lightCamera.setFov(50);

	//计算矩阵
	ViewMatrix = lightCamera.getViewMatrix();
	ProjectionMatrix = lightCamera.getProjMatrix();
	lightSpaceMatrix = ProjectionMatrix * ViewMatrix;
	MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

	//关闭面剔除
	enableFaceCulling = false;
	//enableFrontFaceCulling = true;

	//计算阴影贴图
	for (int m = 0; m < models.size(); m++) {
		DepthShader depthshader(&lightSpaceMatrix);
		VerInf faceVer[3];
		model = &models[m];
		for (int i = 0; i < model->nfaces(); i++) {
			for (int j = 0; j < 3; j++) {
				depthshader.vertex(i, j, faceVer[j]);
			}
			triangle(faceVer, depthshader,//传入顶点数据和shader
					SHADOW_WIDTH, SHADOW_HEIGHT,//传入屏幕大小用于视窗变换
					lightCamera.getNear(),lightCamera.getFar(),//传入透视远近平面用于裁切和线性zbuffer
					shadowBuffer, shadowTexture,//传入绘制buffer
					false,//是否绘制线框模型
					false//是否绘雾
					);
		}
	}
	//启动背面剔除
	enableFaceCulling = true;
	enableFrontFaceCulling = false;
}

void draw(std::vector<Model>& models, ColorVec* drawBuffer,double* shadowBuffer, double* zbuffer) {
	//清空drawbuffer和zbuffer，绘制新的画面
	std::fill(zbuffer, zbuffer + SCREEN_WIDTH * SCREEN_HEIGHT, 1);
	std::fill(drawBuffer, drawBuffer + SCREEN_WIDTH * SCREEN_HEIGHT, backGroundColor);

	//计算MVP矩阵
	//首先执行缩放，接着旋转，最后才是平移
	//ModelMatrix = translate(0, 1, 0) * rotate(up, timer) * scale(1, 1, 1);
	ViewMatrix = defaultCamera.getViewMatrix();
	ProjectionMatrix = defaultCamera.getProjMatrix();
	MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

	//绘制模型的三角面片
	for (int m = 0; m < models.size(); m++) {
		Shader shader(&lightSpaceMatrix, shadowBuffer);
		VerInf faceVer[3];
		model = &models[m];
		for (int i = 0; i < model->nfaces(); i++) {
			for (int j = 0; j < 3; j++) {
				shader.vertex(i, j, faceVer[j]);
			}
			triangle(faceVer, shader, //传入顶点数据和shader
					SCREEN_WIDTH, SCREEN_HEIGHT,//传入屏幕大小用于视窗变换
					defaultCamera.getNear(), defaultCamera.getFar(), //传入透视远近平面用于裁切和线性zbuffer
					zbuffer, drawBuffer,//传入绘制buffer
					false,//是否绘制线框模型
					true//是否绘雾
				);
		}
	}
}

//#define showShadow
int main(int argc, char** argv) {
	//创建视窗
	SDLWindow window("SoftRenderer",SCREEN_WIDTH, SCREEN_HEIGHT);
#ifdef showShadow
	SDLWindow shadow("shadow", SHADOW_WIDTH, SHADOW_HEIGHT);
#endif // showShadow
	//监听键鼠事件
	KeyboardAndMouseHandle KMH(SCREEN_WIDTH, SCREEN_HEIGHT,&defaultCamera);

	if (!window.initSuccess)
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Main loop flag
		KMH.SDL_Runing = true;

		//模型信息
		std::vector<std::string> modleName = {
			"obj/african_head/african_head.obj",
			"obj/african_head/african_head_eye_inner.obj",
			//"obj/diablo3_pose/diablo3_pose.obj",
			"obj/floor.obj",
			//"obj/floor1.obj",
			//"obj/window.obj",
		};

		//载入需要渲染的模型
		std::vector<Model> models;
		for (int m = 0; m < modleName.size(); m++) {
			models.push_back(Model(modleName[m]));
		}

		//创建相机
		defaultCamera = Camera((float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.3, 10, 60);
		lightCamera = Camera((float)SHADOW_WIDTH / SHADOW_HEIGHT, 0.3, 10, 60);

		//初始化矩阵
		ViewMatrix = Matrix::identity();
		ModelMatrix = Matrix::identity();
		ProjectionMatrix = Matrix::identity();
		lightSpaceMatrix = Matrix::identity();

		//创建zbuffer
		double* zbuffer = new double[SCREEN_HEIGHT * SCREEN_WIDTH];
		std::fill(zbuffer, zbuffer + SCREEN_WIDTH * SCREEN_HEIGHT, 1);

		//创建双缓存
		ColorVec* drawBuffer = new ColorVec[SCREEN_HEIGHT * SCREEN_WIDTH];
		ColorVec* showBuffer = new ColorVec[SCREEN_HEIGHT * SCREEN_WIDTH];
		ColorVec* temp = NULL;

		//创建阴影贴图
		double* shadowBuffer = new double[SHADOW_WIDTH * SHADOW_HEIGHT];
		std::fill(shadowBuffer, shadowBuffer + SHADOW_WIDTH * SHADOW_HEIGHT, 1);
		ColorVec* shadowTexture = new ColorVec[SHADOW_WIDTH * SHADOW_HEIGHT];
		std::fill(shadowTexture, shadowTexture + SHADOW_WIDTH * SHADOW_HEIGHT, white);

		// 启动背面剔除
		enableFaceCulling = true;
		enableFrontFaceCulling = false;

		//帧计数器
		int timer = 0;
		float angle = 0;
		
		//光源旋转
		//angle += 10 * deltaTime;
		//float radio = 2;
		//float lightX = radio * cos(angle * DegToRad);
		//float lightZ = radio * sin(angle * DegToRad);
		//lightPos.x = lightX;
		//lightPos.z = lightZ;
		//绘制shadow map
		drawShadowMap(models, shadowBuffer, shadowTexture);

		//While application is running
		while (KMH.SDL_Runing){
			//计算deltaTime与fps
			timer++;
			clock_t currentFrame = clock();
			deltaTime = float((float)currentFrame - lastFrame)/ CLOCKS_PER_SEC;
			lastFrame = currentFrame;
			if (timer >= 1 / deltaTime) {
				std::cout << "Current Frames Per Second:" << int(1 / deltaTime + 0.5f) << std::endl;
				timer = 0;
			}
			
			//处理键鼠事件
			KMH.getMouseKeyEven(NULL, deltaTime);

			

			
			
			//根据shadow map绘制模型
			draw(models, drawBuffer, shadowBuffer, zbuffer);

			//交换缓存
			temp = drawBuffer;
			drawBuffer = showBuffer;
			showBuffer = temp;

			//更新屏幕显示内容
			window.refresh(showBuffer);
#ifdef showShadow
			shadow.refresh(shadowTexture);
#endif
		}
		delete[] zbuffer;
		delete[] showBuffer;
		delete[] drawBuffer;
		delete[] shadowTexture;
		delete[] shadowBuffer;
	}
	//Free resources and close SDL
	window.close();
#ifdef showShadow
	shadow.close();
#endif
	return 0;
}

void drawWindowTGA() {
	int imageH = 256;
	int imageW = 256;
	int midH = imageH / 2;
	int midW = imageW / 2;
	int lineW = 10;
	int halfLineW = lineW/2;
	TGAImage frame(imageW, imageH, TGAImage::RGBA);

	TGAColor frameColor(100, 80, 100, 255);
	TGAColor windowColor(0, 200, 255, 128);
	
	for (int i = 0; i < imageH; i++) {
		for (int j = 0; j < imageW; j++) {
			if ((i < lineW || i >= imageH - lineW)
				|| (j < lineW || j >= imageW - lineW)
				|| (i > midH - halfLineW && i <= midH + halfLineW)
				|| (j > midW - halfLineW && j <= midW + halfLineW)) 
			{
				frame.set(i, j, frameColor);
			}
			else {
				frame.set(i, j, windowColor);
			}
			
		}
	}
	frame.flip_vertically(); // to place the origin in the bottom left corner of the image
	frame.write_tga_file("window_diffuse.tga");
}


