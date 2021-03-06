#include <vector>
#include <limits>
#include <cmath>
#include <iostream>
#include <time.h>
#include <Thread>
#include "myGL.h"
#include "model.h"
#include "KeyboardAndMouseHandle.h"
#include "SDLWindow.h"
#include "Light.h"

//#pragma comment(lib,"SDL2_ttf.dll")
//#include "SDL_ttf.h"
//Screen dimension constants
const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 400;
const int SHADOW_WIDTH = 1000, SHADOW_HEIGHT = 1000;


//位置相关参数
Vec3f up(0, 1, 0);
Vec3f right(1, 0, 0);
Vec3f forword(0, 0, 1);
Vec3f center(0, 0, 0);
Vec3f camDefaultPos(0, 0, 1);

//颜色
const Vec4f ambientColor(1, 1, 1, 1);
const Vec4f white(1, 1, 1, 1);
const Vec4f black(0, 0, 0, 1);

float screenGamma = 0.8f;
float exposure = 0.9f;
//光源位置
//Vec3f lightPos(2, 1, -1);//暗黑破坏神光源
//Vec3f lightPos(2.2, 3.5, 2);//黑人头

//模型指针
Model* modelNow;

//计数器
clock_t lastFrame = 0;
float deltaTime = 0;

//摄像机
Camera defaultCamera;
//Camera lightCamera;


struct diffuseTextureShader : public IShader {
	virtual void vertex(int iface, int nthvert, VerInf& faceVer) {//对第iface的第nthvert个顶点进行变换
		//获得模型uv
		faceVer.uv = modelNow->uv(iface, nthvert);
		//裁切坐标
		faceVer.clip_coord = MVP * embed<4>(modelNow->vert(iface, nthvert));
	}


	virtual bool fragment(VerInf& verInf, Vec4f& color) {
		//插值uv
		Vec2f uv = verInf.uv;
		TGAColor tgaDiff = modelNow->diffuse(uv);
		color = Vec4f(tgaDiff.bgra[2] / 255.f, tgaDiff.bgra[1] / 255.f, tgaDiff.bgra[0] / 255.f, tgaDiff.bgra[3] / 255.f);
		return false;
	}
};

struct reflectSkyboxShader : public IShader {
	Material* Mat;
	bool nullMat;
	TGAImage* skyboxFaces;
	reflectSkyboxShader(Material* mat, TGAImage* skyboxFaces) {
		this->skyboxFaces = skyboxFaces;
		if (mat == NULL) {
			Mat = new Material();
			nullMat = true;
		}
		else {
			Mat = mat;
			nullMat = false;
		}
	}
	~reflectSkyboxShader() {
		if (nullMat) {
			delete Mat;
		}
	}
	virtual void vertex(int iface, int nthvert, VerInf& faceVer) {//对第iface的第nthvert个顶点进行变换
		//获得模型uv
		faceVer.uv = modelNow->uv(iface, nthvert);
		Vec4f gl_Vertex = embed<4>(modelNow->vert(iface, nthvert));
		//法线变换
		faceVer.normal = proj<3>(normalMatrix * embed<4>(modelNow->normal(iface, nthvert), 0.0f));
		//世界坐标
		faceVer.world_pos = proj<3>(ModelMatrix * gl_Vertex);
		//裁切坐标
		faceVer.clip_coord = MVP * gl_Vertex;
	}


	virtual bool fragment(VerInf& verInf, Vec4f& color) {
		//插值uv
		Vec2f uv = verInf.uv;
		TGAColor tgaDiff = modelNow->diffuse(uv);
		//插值法向量
		Vec3f normal = verInf.normal;
		//Vec3f normal = modelNow->normal(uv);
		normal.normalize();

		//设定材质
		Mat->diffuse = Vec3f(tgaDiff.bgra[2] / 255.f, tgaDiff.bgra[1] / 255.f, tgaDiff.bgra[0] / 255.f);
		Mat->ambient = Mat->diffuse;
		Mat->specular = Mat->diffuse;
		if (nullMat) {
			Mat->shininess = 50;
		}

		//计算光源反射颜色
		Vec3f worldPos = verInf.world_pos;
		Vec3f viewDir = (defaultCamera.getPos() - worldPos).normalize();
		Vec3f I = viewDir * (-1);
		Vec3f R = reflect(I, normal);
		Vec3f skyColor = proj<3>(CubeMap(skyboxFaces, R));


		float Rrate = Mat->shininess / 255;
		Vec3f hdrColor = skyColor * Rrate + (Mat->diffuse) * (1 - Rrate);

		Vec3f mapped(0, 0, 0);
		for (int i = 0; i < 3; i++) {
			// Reinhard色调映射
			//	mapped[i] = hdrColor[i] / (hdrColor[i] + 1);
			// 曝光色调映射
			mapped[i] = float(1.0f - exp(float (-hdrColor[i]) * exposure));
		}

		color = embed<4>(mapped, tgaDiff.bgra[3] / 255.f);
		return false;
	}
};

struct LightAndShadowShader : public IShader {
	Frame* SAAOTexture; 
	std::vector<PointLight>* Lights;
	Material * Mat;
	bool nullMat; 
	LightAndShadowShader(Frame* ssao,
		std::vector<PointLight>* lights, Material* mat) {
		SAAOTexture = ssao;
		Lights = lights;
		if (mat == NULL) {
			Mat = new Material();
			nullMat = true;
		}
		else {
			Mat = mat;
			nullMat = false;
		}
	}
	~LightAndShadowShader(){
		if (nullMat) {
			delete Mat;
		}
	}
	virtual void vertex(int iface, int nthvert, VerInf& faceVer) {//对第iface的第nthvert个顶点进行变换
		//获得模型uv
		faceVer.uv = modelNow->uv(iface, nthvert);
		Vec4f gl_Vertex = embed<4>(modelNow->vert(iface, nthvert));
		//法线变换
		faceVer.normal = proj<3>(normalMatrix * embed<4>(modelNow->normal(iface, nthvert), 0.0f));
		//世界坐标
		faceVer.world_pos = proj<3>(ModelMatrix * gl_Vertex);
		//裁切坐标
		faceVer.clip_coord = MVP * gl_Vertex;
	}

	
	virtual bool fragment(VerInf& verInf, Vec4f& color) {
		//插值uv
		Vec2f uv = verInf.uv;
		TGAColor tgaDiff = modelNow->diffuse(uv);
		//插值法向量
		//Vec3f normal = verInf.normal;
		//使用法线贴图
		Vec3f normal = modelNow->normal(uv);
		normal.normalize();

		//设定材质
		Mat->diffuse = Vec3f(tgaDiff.bgra[2] / 255.f, tgaDiff.bgra[1] / 255.f, tgaDiff.bgra[0] / 255.f);
		Mat->ambient = Mat->diffuse;
		Mat->specular = Mat->diffuse;
		if (nullMat) {
			Mat->shininess = 32;//modelNow->specular(uv);
		}
		//计算光源反射颜色
		Vec3f worldPos = verInf.world_pos;
		Vec3f viewDir = (defaultCamera.getPos() - worldPos).normalize();
		Vec3f hdrColor(0, 0, 0);
		for (int i = 0; i < (*Lights).size(); i++) {
			hdrColor = hdrColor + (*Lights)[i].calcLightColor(normal, worldPos, viewDir, *Mat);
		}
		
		//计算环境光
		float ambient = (*SAAOTexture->getPixel(verInf.screen_coord.x, verInf.screen_coord.y)).x;
		//ambient = clamp(ambient, 0.2f, f);
		Vec3f ambientColor(1, 1, 1);
		ambientColor = colorMulit(ambientColor, Mat->diffuse) * ambient;

		hdrColor = hdrColor + ambientColor;

		Vec3f mapped(0, 0, 0);
		for (int i = 0; i < 3; i++) {
		// Reinhard色调映射
		//	mapped[i] = hdrColor[i] / (hdrColor[i] + 1);
		// 曝光色调映射
			mapped[i] = 1.0f - exp(-hdrColor[i] * exposure);
		}

		color = embed<4>(mapped, tgaDiff.bgra[3] / 255.f);
		return false;
	}
};

struct PointLightShader : public IShader {
	PointLight* pointLight;
	PointLightShader(PointLight* light) {
		pointLight = light;
	}
	virtual void vertex(int iface, int nthvert, VerInf& faceVer) {
		faceVer.clip_coord = MVP * embed<4>(modelNow->vert(iface, nthvert));
	}

	virtual bool fragment(VerInf& verInf, Vec4f& color) {
		color = embed<4>(pointLight->lightColor,1.0f);
		return false;
	}
};

struct DepthShader : public IShader {
	Matrix* LightSpaceMatrix;
	Camera* DepthCamera;
	DepthShader(Matrix* lightSpaceMatrix, Camera* depthCamera) {
		LightSpaceMatrix = lightSpaceMatrix;
		DepthCamera = depthCamera;
	}
	virtual void vertex(int iface, int nthvert, VerInf& faceVer) {
		Vec4f gl_Vertex = embed<4>(modelNow->vert(iface, nthvert));
		//faceVer.world_pos = proj<3>(ModelMatrix * gl_Vertex);
		faceVer.clip_coord = *LightSpaceMatrix * ModelMatrix  * gl_Vertex; 
	}

	virtual bool fragment(VerInf& verInf, Vec4f& color) {
		if (DepthCamera->getProjectMode()) {
			float far = DepthCamera->getFar();
			float near = DepthCamera->getNear();
			verInf.depth = LinearizeDepth(verInf.depth,near, far)/ far;
		}
		color = Vec4f(1, 1, 1,1) * verInf.depth;
		color.w = 1;
		return false;
	}
};

struct SSAOShader : public IShader {
	virtual void vertex(int iface, int nthvert, VerInf& faceVer) {
		Vec4f gl_Vertex = embed<4>(modelNow->vert(iface, nthvert));
		faceVer.clip_coord = MVP * gl_Vertex;
	}

	virtual bool fragment(VerInf& verInf, Vec4f& color) {
		color = Vec4f(1, 1, 1,1);
		return false;
	}
};

struct skyboxShader : public IShader {
	TGAImage* skyboxFaces;
	skyboxShader(TGAImage* skyboxFaces) {
		this->skyboxFaces = skyboxFaces;
	}
	virtual void vertex(int iface, int nthvert, VerInf& faceVer) {
		Vec4f gl_Vertex = embed<4>(modelNow->vert(iface, nthvert));
		faceVer.world_pos = modelNow->vert(iface, nthvert);
		ViewMatrix[0][3] = 0;
		ViewMatrix[1][3] = 0;
		ViewMatrix[2][3] = 0;
		faceVer.clip_coord =ProjectionMatrix*ViewMatrix * gl_Vertex;
		//faceVer.clip_coord.w = 1.0f;
		//faceVer.clip_coord.z = faceVer.clip_coord.w;
	}

	virtual bool fragment(VerInf& verInf, Vec4f& color) {
		color = CubeMap(skyboxFaces, verInf.world_pos);
		return false;
	}
};

inline void drawSingleModle(Model& modle, IShader& shader, const ViewPort& port, Frame* drawBuffer, double* zbuffer, RendererMode mode = RendererMode::fragment, bool fog = false) {
	VerInf faceVer[3];
	modelNow = &modle;
	for (int i = 0; i < modelNow->nfaces(); i++) {
		for (int j = 0; j < 3; j++) {
			shader.vertex(i, j, faceVer[j]);
		}
		triangle(faceVer, shader, //传入顶点数据和shader
			port,//传入屏幕大小用于视窗变换
			defaultCamera.getNear(), defaultCamera.getFar(), //传入透视远近平面用于裁切和线性zbuffer
			zbuffer, drawBuffer,//传入绘制buffer
			mode,//是否绘制线框模型
			fog//是否绘雾
		);
	}
}
void drawPointLightPos(Model& cube, std::vector<PointLight>& lights, const ViewPort& port, Frame* drawBuffer, double* zbuffer) {
	//启动正面剔除
	enableFaceCulling = true;
	//绘制zbuffer
	modelNow = &cube;
	for (int m = 0; m < lights.size(); m++) {
		ModelMatrix = translate(lights[m].lightPos.x, lights[m].lightPos.y, lights[m].lightPos.z) * scale(0.1f, 0.1f, 0.1f);
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		PointLightShader shader(&lights[m]);
		drawSingleModle(cube, shader, port, drawBuffer, zbuffer, RendererMode::fragment, false);
	}
	ModelMatrix = Matrix::identity();
	MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
}
float max_elevation_angle(double* zbuffer, float radio, float near, float far, Vec2f& p, Vec2f& dir) {
	float maxangle = 0;
	//double orgZ = LinearizeDepth(zbuffer[int(p.x) + int(p.y) * SCREEN_WIDTH], near, far);
	double orgZ = zbuffer[int(p.x) + int(p.y) * SCREEN_WIDTH];
	for (float t = 0.; t < radio; t += 1.) {
		Vec2f cur = p + dir * t;
		if (cur.x >= SCREEN_WIDTH || cur.y >= SCREEN_HEIGHT || cur.x < 0 || cur.y < 0) return maxangle;
		float distance = (p - cur).norm();
		//if (distance < 1.f) continue;
		double nearZ = zbuffer[int(cur.x) + int(cur.y) * SCREEN_WIDTH];
		//double nearZ =  LinearizeDepth(zbuffer[int(cur.x) + int(cur.y) * SCREEN_WIDTH], near, far);
		//if (abs(orgZ - nearZ) >= 0.2)continue;
		//float elevation = nearZ/ far - orgZ / far;
		float elevation = float(nearZ - orgZ);
		//if (elevation >= 0.2) continue;
		maxangle = std::max(maxangle, atanf(elevation / distance));
	}
	return maxangle;
}
void drawSSAOTexture(std::vector<Model>& models,const ViewPort& port, double* zbuffer, Frame* SSAOTexture) {
	//清空drawbuffer，绘制新的画面
	SSAOTexture->fill(white);
	float far = defaultCamera.getFar();
	float near = defaultCamera.getNear();

	//启动正面剔除
	enableFaceCulling = true;
	
	//绘制zbuffer
	for (int m = 0; m < models.size(); m++) {
		SSAOShader shader;
		drawSingleModle(models[m], shader, port, SSAOTexture, zbuffer, RendererMode::fragment, false);
	}

	////计算SAAO
	float halfPI = (float)M_PI / 2.0f;
	int samplingDir = 4;
	float dirChangeAlmont = (float)M_PI * 2.0f/samplingDir;
	
	Vec2f dirList[4];
	for (int i = 0; i < samplingDir; i++) {
		float a = float (dirChangeAlmont * i);
		dirList[i] = Vec2f(cos(a), sin(a));
	}
	Vec2f posNow(0, 0);
	Vec4f temp;
	for (posNow.x = 0; posNow.x < SCREEN_WIDTH; posNow.x++) {
		for (posNow.y = 0; posNow.y < SCREEN_HEIGHT; posNow.y++) {
			int id = (int)posNow.x + (int)posNow.y * SCREEN_WIDTH;
			//if (zbuffer[id] >=1) continue;
			float total = 0;
			
			for (int i = 0; i < samplingDir;i++ ) {
				
				total += halfPI - max_elevation_angle(zbuffer, 2, near, far, posNow, dirList[i]);
			}
			//std::cout << total << std::endl;
			total /= halfPI * samplingDir;
			total = pow(total, 1000.f);
			
			temp.x = total;
			temp.y = total;
			temp.z = total;
			temp.w = 1;

			int x = (int)posNow.x;
			int y = (int)posNow.y;
			SSAOTexture->setPixel(x,y, temp);

		}
	}

}

Frame* shadowTexture = new Frame(SHADOW_WIDTH, SHADOW_HEIGHT);
void drawShadowMap(std::vector<Model>& models,PointLight& light) {
	if (light.enableShadow == false)return;
	if (light.ShadowPort == NULL) {
		light.ShadowPort = new ViewPort(Vec2i(0, 0), SHADOW_WIDTH, SHADOW_HEIGHT);
	}
	//创建阴影贴图
	if (light.depthBuffer == NULL) {
		light.depthBuffer = new double[SHADOW_WIDTH * SHADOW_HEIGHT];
	}
	//清空shadowMap数据，重新绘制
	std::fill(light.depthBuffer, light.depthBuffer + SHADOW_WIDTH * SHADOW_HEIGHT, 1);
	shadowTexture->fill(white);

	//启动深度测试
	enableZTest = true;
	enableZWrite = true;

	//将摄像机摆放到光源位置
	light.lightCamera.changeViewPort(light.ShadowPort);
	light.lightCamera.setCamera(light.lightPos, center, up);
	light.lightCamera.setClipPlane(0.5,8);
	light.lightCamera.setFov(90);

	//计算矩阵
	ModelMatrix = Matrix::identity();
	light.lightMatrix = light.lightCamera.getProjMatrix() * light.lightCamera.getViewMatrix();
	//MVP = *light.LightSpaceMatrix * ModelMatrix;

	//关闭面剔除
	enableFaceCulling = false;

	//计算阴影贴图
	DepthShader depthshader(&light.lightMatrix, &light.lightCamera);
	for (int m = 0; m < models.size(); m++) {
		drawSingleModle(models[m], depthshader, *light.ShadowPort, shadowTexture, light.depthBuffer, RendererMode::fragment, false);
	}
	//启动背面剔除
	enableFaceCulling = true;
	enableFrontFaceCulling = false;
}

void drawSkybox(Model& skyboxModle, const ViewPort& port, TGAImage* skyboxFaces,Frame* drawBuffer, double* zbuffer) {
	enableFaceCulling = false;
	enableZWrite = false;
	enableZTest = false;
	//绘制模型的三角面片
	skyboxShader skyboxShader(skyboxFaces);
	drawSingleModle(skyboxModle, skyboxShader, port, drawBuffer, zbuffer, RendererMode::fragment, false);
	enableZTest = true;
	enableZWrite = true;
	enableFaceCulling = true;
}

//void draw(std::vector<Model>& models, IShader& shader, const ViewPort& port, Frame* drawBuffer, double* zbuffer) {
void draw(Model& model, RendererMode mode, IShader& shader, const ViewPort& port, Frame* drawBuffer, double* zbuffer) {

	//绘制模型的三角面片
	drawSingleModle(model, shader, port, drawBuffer, zbuffer, mode, false);
	
}

void draw(std::vector<Model>& models, RendererMode mode, IShader& shader, const ViewPort& port, Frame* drawBuffer, double* zbuffer) {

	//绘制模型的三角面片
	for (int m = 0; m < models.size(); m++) {
		drawSingleModle(models[m], shader, port, drawBuffer, zbuffer, mode, false);
	}
}

//#define showShadow
//#define showSSAO
int main(int argc, char** argv) {
	//创建视窗
	SDLWindow mainWindow("SoftRenderer",SCREEN_WIDTH, SCREEN_HEIGHT, screenGamma);
#ifdef showShadow
	SDLWindow shadow("shadow", SHADOW_WIDTH, SHADOW_HEIGHT, 1);
#endif // showShadow
#ifdef showSSAO
	SDLWindow SSAO("SSAO", SCREEN_WIDTH, SCREEN_HEIGHT,1);
#endif // showSSAO

	//监听键鼠事件
	KeyboardAndMouseHandle KMH(SCREEN_WIDTH, SCREEN_HEIGHT,&defaultCamera,&screenGamma,&exposure);

	if (!mainWindow.initSuccess)
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Main loop flag
		KMH.SDL_Runing = true;

		Model windowModel = Model("obj/window.obj");
		Model floor = Model("obj/floor.obj");
		Model diablo = Model("obj/diablo3_pose/diablo3_pose.obj");
		std::vector<Model> african_head;
		african_head.push_back(Model("obj/african_head/african_head.obj"));
		african_head.push_back(Model("obj/african_head/african_head_eye_inner.obj"));

		std::vector<Model> scene;
		//scene.push_back(diablo);
		scene.push_back(floor);
		//scene.push_back(african_head[0]);
		//scene.push_back(african_head[1]);
		scene.push_back(diablo);
		//创建相机
		ViewPort defaultViewPort(Vec2i(0, 0), SCREEN_WIDTH, SCREEN_HEIGHT);
		defaultCamera = Camera(&defaultViewPort, 0.3f, 13.0f, 60.0f);
		//初始化摄像机
		Vec3f camPos(1.5f, 0.8f, 2.0f);
		Vec3f camCenter = center + Vec3f(-1.0f, -0.2f, 0.0f);
		camPos = camPos * 0.8;
		defaultCamera.setCamera(camPos, camCenter, up);

		//初始化矩阵
		ViewMatrix = Matrix::identity();
		ModelMatrix = Matrix::identity();
		ProjectionMatrix = Matrix::identity();

		//创建zbuffer
		double* zbuffer = new double[SCREEN_HEIGHT * SCREEN_WIDTH];
		std::fill(zbuffer, zbuffer + SCREEN_WIDTH * SCREEN_HEIGHT, 1);

		//创建双缓存
		Frame* drawBuffer = new Frame(SCREEN_WIDTH, SCREEN_HEIGHT);
		Frame* showBuffer = new Frame(SCREEN_WIDTH, SCREEN_HEIGHT);
		Frame* temp = NULL;

		//创建屏幕环境遮罩贴图
		Frame* SSAOTexture = new Frame(SCREEN_WIDTH, SCREEN_HEIGHT);
		SSAOTexture->fill(white);

		//载入天空盒
		Model cube("obj/skybox.obj");
		std::vector<const char*> skyboxFaceName = {
			"skybox/right.tga",
			"skybox/left.tga",
			
			
			"skybox/top.tga",
			"skybox/bottom.tga",

			"skybox/back.tga",
			"skybox/front.tga",
			
		};
		TGAImage* skyboxFaces = new TGAImage[6];
		for (int i = 0; i < 6; i++) {
			skyboxFaces[i].read_tga_file(skyboxFaceName[i]);
		}

		// 启动背面剔除
		enableFaceCulling = true;
		enableFrontFaceCulling = false;

		//帧计数器
		int timer = 0;
		float angle = 0;

		
		
		//设置光源
		std::vector<PointLight> pointlights;
		pointlights.push_back(PointLight(Vec3f(1.3, 2.0f, 1), Vec3f(1, 1, 1), 10.f, true));
		//pointlights.push_back(PointLight(Vec3f(-1, 1, 1),Vec3f(0.3, 1,0.3), 5.f, false));
		//pointlights.push_back(PointLight(Vec3f(1, 1, 1),Vec3f(0.3, 0.3, 1),5.f, false));
	
		for (int i = 0; i < pointlights.size(); i++) {\
			////光源旋转
			//angle += 10 * deltaTime;
			//float angleRad = angle * DegToRad;
			//float radio = 2;
			//float lightX = radio * cos((angle + i * 90) * DegToRad);
			//float lightZ = radio * sin((angle + i * 90) * DegToRad);
			//pointlights[i].lightPos.x = lightX;
			//pointlights[i].lightPos.z = lightZ;
			//pointlights[i].lightPos.y = 2;

			//绘制shadow map
			drawShadowMap(scene, pointlights[i]);
		}
		//While application is running
		while (KMH.SDL_Runing){
			//计算deltaTime与fps
			timer++;
			clock_t currentFrame = clock();
			deltaTime = float((float)currentFrame - lastFrame) / CLOCKS_PER_SEC;
			lastFrame = currentFrame;
			if (timer >= 1 / deltaTime) {
				std::cout << "Current Frames Per Second:" << 1 / deltaTime << std::endl;
				timer = 0;
			}

			//处理键鼠事件
			KMH.getMouseKeyEven(NULL, deltaTime);
			//计算MVP矩阵
			//首先执行缩放，接着旋转，最后才是平移
			//ModelMatrix = translate(0, 1, 0) * rotate(up, timer) * scale(1, 1, 1);
			ModelMatrix = Matrix::identity();
			ViewMatrix = defaultCamera.getViewMatrix();
			ProjectionMatrix = defaultCamera.getProjMatrix();
			Matrix cameraMat = ProjectionMatrix * ViewMatrix;
			MVP = cameraMat * ModelMatrix;

			normalMatrix = ModelMatrix.invert_transpose();
			//清空绘图区域
			drawBuffer->fill(black);
			////清空zbuffer
			std::fill(zbuffer, zbuffer + SCREEN_WIDTH * SCREEN_HEIGHT, 1);

			//绘制光源位置
			drawPointLightPos(cube, pointlights, defaultViewPort, drawBuffer, zbuffer);

			//绘制屏幕全局光照贴图
			//drawSSAOTexture(scene, defaultViewPort, zbuffer, SSAOTexture);
			
			//根据shadow map和SSAO绘制模型
			enableFaceCulling = true;
			enableFrontFaceCulling = false;
			enableZTest = true;
			enableZWrite = true;

			Material m;
			m.shininess = 250;
			diffuseTextureShader diffuseTexture;
			reflectSkyboxShader reflectShaderWithTextrue(NULL, skyboxFaces);
			reflectSkyboxShader reflectShaderWithoutTextrue(&m, skyboxFaces);
			LightAndShadowShader shaderWithTextrue(SSAOTexture, &pointlights, NULL);
			LightAndShadowShader shaderWithoutTextrue(SSAOTexture, &pointlights, &m);
			
			DepthShader depthshader(&cameraMat, &defaultCamera);

			RendererMode mode = RendererMode::fragment;
			draw(diablo, mode, shaderWithTextrue, defaultViewPort, drawBuffer,  zbuffer);
			draw(floor, mode, shaderWithoutTextrue, defaultViewPort, drawBuffer, zbuffer);
		/*	draw(diablo, shaderWithTextrue, defaultViewPort, drawBuffer,  zbuffer);
			draw(floor, shaderWithoutTextrue, defaultViewPort, drawBuffer, zbuffer);
		*/	
		
			//draw(windowModel, diffuseTexture, defaultViewPort, drawBuffer, zbuffer);
			
			//绘制天空盒
			drawSkybox(cube, defaultViewPort, skyboxFaces, drawBuffer, zbuffer);

			//交换缓存
			temp = drawBuffer;
			drawBuffer = showBuffer;
			showBuffer = temp;

			//更新屏幕显示内容
			mainWindow.screenGamma = screenGamma;
			mainWindow.refresh(drawBuffer);

#ifdef showShadow
			shadow.refresh(shadowTexture);
#endif
#ifdef showSSAO
			SSAO.refresh(SSAOTexture);
#endif
		}
		delete[] zbuffer;


		delete showBuffer;
		delete drawBuffer;
		delete shadowTexture;
		
		delete SSAOTexture;
	}
	//Free resources and close SDL
	mainWindow.close();
#ifdef showShadow
	shadow.close();
#endif

#ifdef showSSAO
	SSAO.close();
#endif
	return 0;
}
