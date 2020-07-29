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
Vec3f center(0, -1, 0);
Vec3f camDefaultPos(0, 0, 1);

//颜色
ColorVec backGroundColor(100, 30, 0x00, 0xFF);
ColorVec white(0xFF, 0xFF, 0xFF, 0xFF);


Model* model;
Vec3f lightPos(2, 2,2);
//Vec3f lightDir(-5, -5,-5);

clock_t lastFrame = 0;
float deltaTime = 0;

const float depth = 2000.f;

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
		faceVer.normal = proj<3>(MVP.invert_transpose() * embed<4>(model->normal(iface, nthvert), 0.f));
		//世界坐标
		faceVer.world_pos = proj<3>(ModelMatrix * embed<4>(model->vert(iface, nthvert)));
		//裁切坐标
		faceVer.clip_coord = MVP * embed<4>(model->vert(iface, nthvert));

		//光源空间裁切坐标
		//faceVer.clipPosLightSpace = *LightSpaceMatrix *embed<4>(faceVer.world_pos,1.0f);
	}

	float inShadow(Vec3f& world_pos, float& bias) {
		Vec4f clipPosLightSpace = *LightSpaceMatrix * embed<4>(world_pos);
		float light_recip_w = 1 / clipPosLightSpace.w;
		Vec3f ndcLightSpace = proj<3>(clipPosLightSpace * light_recip_w);
		ndcLightSpace.x = clamp(ndcLightSpace.x, -1.0f, 1.0f);
		ndcLightSpace.y = clamp(ndcLightSpace.y, -1.0f, 1.0f);
		ndcLightSpace.z = clamp(ndcLightSpace.z, -1.0f, 1.0f);
		Vec3f shadowTextureCoords = viewport_transform(SHADOW_WIDTH, SHADOW_HEIGHT, ndcLightSpace);

		// 计算顶点在光源坐标系的屏幕坐标
		//for (int i = 0; i < 3; i++) {
		//	double light_recip_w = 1 / verInf[i]->clipPosLightSpace.w;
		//	verInf[i]->ndcLightSpace = proj<3>(verInf[i]->clipPosLightSpace * light_recip_w);
		//	Vec3f window_coord = viewport_transform(1024, 1024, verInf[i]->ndcLightSpace);
		//	verInf[i]->screenPosLightSpace = Vec2i((int)window_coord.x, (int)window_coord.y);
		//	verInf[i]->depthLightSpace = window_coord.z;
		//}

		// 变换到阴影贴图的范围
		//std::cout << "ndcPosLightSpace:" << ndcPosLightSpace << std::endl;
		// 取得最近点的深度(使用[0,1]范围下的fragPosLight当坐标)
		//std::cout << "shadowPos:" << shadowTextureCoords << std::endl;
		float closestDepth = ShadowBuffer[(int)shadowTextureCoords.x + (int)shadowTextureCoords.y * SHADOW_WIDTH];
		// 取得当前片元在光源视角下的深度
		float currentDepth = LinearizeDepth(shadowTextureCoords.z);
		//float currentDepth = shadowTextureCoords.z;
		//std::cout << "currentDepth:" << currentDepth << std::endl;
		//std::cout << "closestDepth:" << closestDepth << std::endl;
		// 检查当前片元是否在阴影中
		//设置阴影偏移

		return currentDepth - bias > closestDepth ? 1.0 : 0.0;
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
		float lightPower = 10;
		//环境光
		float ambient = 0.2f;
		Vec3f vertPos = verInf.world_pos;
		
		//衰减
		Vec3f lightDir = (lightPos - vertPos);
		float dis = lightDir.norm();
		lightDir = lightDir / dis;

		//漫反射
		float diff = clamp<float>(std::abs(lightDir * normal), 0, 1);

		//镜面反射
		Vec3f viewDir = (defaultCamera.getPos() - vertPos).normalize();
		Vec3f hafe = (lightDir + viewDir).normalize();
		float spec = clamp<float>(std::abs(normal * hafe), 0, 1);

		

		//设置阴影偏移
		float bias = std::max(0.05 * (1.0 - (normal*lightDir)), 0.005);
		//计算该点在不在阴影区域
		float isInShadow = inShadow(verInf.world_pos, bias);

		TGAColor c = model->diffuse(uv);
		color = c;
		for (int i = 0; i < 3; i++) {
			float light = (1 - isInShadow) * (1 * diff + 1 * spec) * lightPower / dis/ dis;
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
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
		faceVer.clip_coord = *LightSpaceMatrix * ModelMatrix  * gl_Vertex;          // transform it to screen coordinates
		//varying_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
	}

	virtual bool fragment(VerInf verInf, TGAColor& color) {
		color = TGAColor(255, 255, 255,255) * verInf.depth;
		return false;
	}
};

void drawShadowMap(std::vector<Model>& models, double* shadowBuffer, ColorVec* shadowTexture) {
	//清空shadowMap数据，重新绘制
	std::fill(shadowBuffer, shadowBuffer + SHADOW_WIDTH * SHADOW_HEIGHT, 1);
	std::fill(shadowTexture, shadowTexture + SHADOW_WIDTH * SHADOW_WIDTH, white);

	//记录旧摄像机数据
	Camera oldCam = defaultCamera;
	//将摄像机摆放到光源位置
	defaultCamera.setCamera(lightPos, center, up);
	defaultCamera.setFov(100);

	//计算矩阵
	ViewMatrix = defaultCamera.getViewMatrix();
	ProjectionMatrix = defaultCamera.getProjMatrix();
	lightSpaceMatrix = ProjectionMatrix * ViewMatrix;
	MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

	//关闭面剔除
	enableFaceCulling = false;

	//计算阴影贴图
	for (int m = 0; m < models.size(); m++) {
		DepthShader depthshader(&lightSpaceMatrix);
		VerInf faceVer[3];
		model = &models[m];
		for (int i = 0; i < model->nfaces(); i++) {
			for (int j = 0; j < 3; j++) {
				depthshader.vertex(i, j, faceVer[j]);
			}
			triangle(false, faceVer, depthshader, SHADOW_WIDTH, SHADOW_HEIGHT, shadowBuffer, shadowTexture);
			//triangle(true,faceVer,shader, SCREEN_WIDTH, SCREEN_HEIGHT, zbuffer, drawBuffer);
		}
	}
	//恢复摄像机
	defaultCamera = oldCam;
	//启动背面剔除
	enableFaceCulling = true;
}

void draw(std::vector<Model>& models, ColorVec* drawBuffer,double* shadowBuffer, double* zbuffer) {
	//清空drawbuffer、zbuffer、shadowBuffer，绘制新的画面
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
			triangle(false, faceVer, shader, SCREEN_WIDTH, SCREEN_HEIGHT, zbuffer, drawBuffer);
			//triangle(true,faceVer,shader, SCREEN_WIDTH, SCREEN_HEIGHT, zbuffer, drawBuffer);
		}
	}
}

int main(int argc, char** argv) {
	//创建视窗
	SDLWindow window("SoftRenderer",SCREEN_WIDTH, SCREEN_HEIGHT);
	//SDLWindow shadow("shadow",SHADOW_WIDTH, SHADOW_HEIGHT);

	//监听键鼠事件
	KeyboardAndMouseHandle KMH(SCREEN_WIDTH, SCREEN_HEIGHT);

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
			//"obj/african_head/african_head.obj",
			//"obj/african_head/african_head_eye_inner.obj",
			"obj/diablo3_pose/diablo3_pose.obj",
			"obj/floor.obj",
			"obj/window.obj",
		};

		//载入需要渲染的模型
		std::vector<Model> models;
		for (int m = 0; m < modleName.size(); m++) {
			models.push_back(Model(modleName[m]));
		}

		//创建相机
		defaultCamera = Camera((float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.3, 7, 60);

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

		//While application is running
		while (KMH.SDL_Runing){
			//计算deltaTime与fps
			timer++;
			clock_t currentFrame = clock();
			deltaTime = float((float)currentFrame - lastFrame)/ CLOCKS_PER_SEC;
			lastFrame = currentFrame;
			if (timer >= 1 / deltaTime) {
				std::cout << "Current Frames Per Second:" << 1 / deltaTime << std::endl;
				timer = 0;
			}
			
			//处理键鼠事件
			KMH.getMouseKeyEven(NULL, deltaTime);

			//光源旋转
			angle += 60 * deltaTime;
			float radio = 2;
			float lightX = radio * cos(angle * DegToRad);
			float lightZ = radio * sin(angle * DegToRad);
			lightPos.x = lightX;
			lightPos.z = lightZ;

			//绘制shadow map
			drawShadowMap(models, shadowBuffer, shadowTexture);

			//根据shadow map绘制模型
			draw(models, drawBuffer, shadowBuffer, zbuffer);

			//交换缓存
			temp = drawBuffer;
			drawBuffer = showBuffer;
			showBuffer = temp;

			//更新屏幕显示内容
			window.refresh(showBuffer);

			//shadow.refresh(shadowTexture);
		}
		delete[] zbuffer;
		delete[] showBuffer;
		delete[] drawBuffer;
		delete[] shadowTexture;
		delete[] shadowBuffer;
	}
	//Free resources and close SDL
	window.close();
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


