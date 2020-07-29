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
//λ����ز���
Vec3f up(0, 1, 0);
Vec3f right(1, 0, 0);
Vec3f forword(0, 0, 1);
Vec3f center(0, -1, 0);
Vec3f camDefaultPos(0, 0, 1);

//��ɫ
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
	virtual void vertex(int iface, int nthvert, VerInf& faceVer) {//�Ե�iface�ĵ�nthvert��������б任
		//���ģ��uv
		faceVer.uv = model->uv(iface, nthvert);
		//���߱任
		faceVer.normal = proj<3>(MVP.invert_transpose() * embed<4>(model->normal(iface, nthvert), 0.f));
		//��������
		faceVer.world_pos = proj<3>(ModelMatrix * embed<4>(model->vert(iface, nthvert)));
		//��������
		faceVer.clip_coord = MVP * embed<4>(model->vert(iface, nthvert));

		//��Դ�ռ��������
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

		// ���㶥���ڹ�Դ����ϵ����Ļ����
		//for (int i = 0; i < 3; i++) {
		//	double light_recip_w = 1 / verInf[i]->clipPosLightSpace.w;
		//	verInf[i]->ndcLightSpace = proj<3>(verInf[i]->clipPosLightSpace * light_recip_w);
		//	Vec3f window_coord = viewport_transform(1024, 1024, verInf[i]->ndcLightSpace);
		//	verInf[i]->screenPosLightSpace = Vec2i((int)window_coord.x, (int)window_coord.y);
		//	verInf[i]->depthLightSpace = window_coord.z;
		//}

		// �任����Ӱ��ͼ�ķ�Χ
		//std::cout << "ndcPosLightSpace:" << ndcPosLightSpace << std::endl;
		// ȡ�����������(ʹ��[0,1]��Χ�µ�fragPosLight������)
		//std::cout << "shadowPos:" << shadowTextureCoords << std::endl;
		float closestDepth = ShadowBuffer[(int)shadowTextureCoords.x + (int)shadowTextureCoords.y * SHADOW_WIDTH];
		// ȡ�õ�ǰƬԪ�ڹ�Դ�ӽ��µ����
		float currentDepth = LinearizeDepth(shadowTextureCoords.z);
		//float currentDepth = shadowTextureCoords.z;
		//std::cout << "currentDepth:" << currentDepth << std::endl;
		//std::cout << "closestDepth:" << closestDepth << std::endl;
		// ��鵱ǰƬԪ�Ƿ�����Ӱ��
		//������Ӱƫ��

		return currentDepth - bias > closestDepth ? 1.0 : 0.0;
	}
	
	virtual bool fragment(VerInf verInf, TGAColor& color) {
		//��ֵuv
		Vec2f uv = verInf.uv;

		//��ֵ������
		//Vec3f normal = verInf.normal;

		//ʹ�÷�����ͼ
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

		//���ù���
		float lightPower = 10;
		//������
		float ambient = 0.2f;
		Vec3f vertPos = verInf.world_pos;
		
		//˥��
		Vec3f lightDir = (lightPos - vertPos);
		float dis = lightDir.norm();
		lightDir = lightDir / dis;

		//������
		float diff = clamp<float>(std::abs(lightDir * normal), 0, 1);

		//���淴��
		Vec3f viewDir = (defaultCamera.getPos() - vertPos).normalize();
		Vec3f hafe = (lightDir + viewDir).normalize();
		float spec = clamp<float>(std::abs(normal * hafe), 0, 1);

		

		//������Ӱƫ��
		float bias = std::max(0.05 * (1.0 - (normal*lightDir)), 0.005);
		//����õ��ڲ�����Ӱ����
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
	//���shadowMap���ݣ����»���
	std::fill(shadowBuffer, shadowBuffer + SHADOW_WIDTH * SHADOW_HEIGHT, 1);
	std::fill(shadowTexture, shadowTexture + SHADOW_WIDTH * SHADOW_WIDTH, white);

	//��¼�����������
	Camera oldCam = defaultCamera;
	//��������ڷŵ���Դλ��
	defaultCamera.setCamera(lightPos, center, up);
	defaultCamera.setFov(100);

	//�������
	ViewMatrix = defaultCamera.getViewMatrix();
	ProjectionMatrix = defaultCamera.getProjMatrix();
	lightSpaceMatrix = ProjectionMatrix * ViewMatrix;
	MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

	//�ر����޳�
	enableFaceCulling = false;

	//������Ӱ��ͼ
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
	//�ָ������
	defaultCamera = oldCam;
	//���������޳�
	enableFaceCulling = true;
}

void draw(std::vector<Model>& models, ColorVec* drawBuffer,double* shadowBuffer, double* zbuffer) {
	//���drawbuffer��zbuffer��shadowBuffer�������µĻ���
	std::fill(zbuffer, zbuffer + SCREEN_WIDTH * SCREEN_HEIGHT, 1);
	std::fill(drawBuffer, drawBuffer + SCREEN_WIDTH * SCREEN_HEIGHT, backGroundColor);

	//����MVP����
	//����ִ�����ţ�������ת��������ƽ��
	//ModelMatrix = translate(0, 1, 0) * rotate(up, timer) * scale(1, 1, 1);
	ViewMatrix = defaultCamera.getViewMatrix();
	ProjectionMatrix = defaultCamera.getProjMatrix();
	MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

	//����ģ�͵�������Ƭ
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
	//�����Ӵ�
	SDLWindow window("SoftRenderer",SCREEN_WIDTH, SCREEN_HEIGHT);
	//SDLWindow shadow("shadow",SHADOW_WIDTH, SHADOW_HEIGHT);

	//���������¼�
	KeyboardAndMouseHandle KMH(SCREEN_WIDTH, SCREEN_HEIGHT);

	if (!window.initSuccess)
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Main loop flag
		KMH.SDL_Runing = true;

		//ģ����Ϣ
		std::vector<std::string> modleName = {
			//"obj/african_head/african_head.obj",
			//"obj/african_head/african_head_eye_inner.obj",
			"obj/diablo3_pose/diablo3_pose.obj",
			"obj/floor.obj",
			"obj/window.obj",
		};

		//������Ҫ��Ⱦ��ģ��
		std::vector<Model> models;
		for (int m = 0; m < modleName.size(); m++) {
			models.push_back(Model(modleName[m]));
		}

		//�������
		defaultCamera = Camera((float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.3, 7, 60);

		//��ʼ������
		ViewMatrix = Matrix::identity();
		ModelMatrix = Matrix::identity();
		ProjectionMatrix = Matrix::identity();
		lightSpaceMatrix = Matrix::identity();

		//����zbuffer
		double* zbuffer = new double[SCREEN_HEIGHT * SCREEN_WIDTH];
		std::fill(zbuffer, zbuffer + SCREEN_WIDTH * SCREEN_HEIGHT, 1);

		//����˫����
		ColorVec* drawBuffer = new ColorVec[SCREEN_HEIGHT * SCREEN_WIDTH];
		ColorVec* showBuffer = new ColorVec[SCREEN_HEIGHT * SCREEN_WIDTH];
		ColorVec* temp = NULL;

		//������Ӱ��ͼ
		double* shadowBuffer = new double[SHADOW_WIDTH * SHADOW_HEIGHT];
		std::fill(shadowBuffer, shadowBuffer + SHADOW_WIDTH * SHADOW_HEIGHT, 1);
		ColorVec* shadowTexture = new ColorVec[SHADOW_WIDTH * SHADOW_HEIGHT];
		std::fill(shadowTexture, shadowTexture + SHADOW_WIDTH * SHADOW_HEIGHT, white);

		// ���������޳�
		enableFaceCulling = true;
		enableFrontFaceCulling = false;
		
		//֡������
		int timer = 0;
		float angle = 0;

		//While application is running
		while (KMH.SDL_Runing){
			//����deltaTime��fps
			timer++;
			clock_t currentFrame = clock();
			deltaTime = float((float)currentFrame - lastFrame)/ CLOCKS_PER_SEC;
			lastFrame = currentFrame;
			if (timer >= 1 / deltaTime) {
				std::cout << "Current Frames Per Second:" << 1 / deltaTime << std::endl;
				timer = 0;
			}
			
			//��������¼�
			KMH.getMouseKeyEven(NULL, deltaTime);

			//��Դ��ת
			angle += 60 * deltaTime;
			float radio = 2;
			float lightX = radio * cos(angle * DegToRad);
			float lightZ = radio * sin(angle * DegToRad);
			lightPos.x = lightX;
			lightPos.z = lightZ;

			//����shadow map
			drawShadowMap(models, shadowBuffer, shadowTexture);

			//����shadow map����ģ��
			draw(models, drawBuffer, shadowBuffer, zbuffer);

			//��������
			temp = drawBuffer;
			drawBuffer = showBuffer;
			showBuffer = temp;

			//������Ļ��ʾ����
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


