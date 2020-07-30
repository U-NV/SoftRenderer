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
Vec3f center(0, 0, 0);
Vec3f camDefaultPos(0, 0, 1);

//��ɫ
const ColorVec backGroundColor(100, 30, 0x00, 0xFF);
const ColorVec white(0xFF, 0xFF, 0xFF, 0xFF);

//��Դλ��
Vec3f lightPos(1, 3, 1);

//ģ��ָ��
Model* model;

//������
clock_t lastFrame = 0;
float deltaTime = 0;

//�����
Camera defaultCamera;
Camera lightCamera;

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
		faceVer.normal = proj<3>(MVP.invert_transpose() * embed<4>(model->normal(iface, nthvert), 0.0));
		//��������
		faceVer.world_pos = proj<3>(ModelMatrix * embed<4>(model->vert(iface, nthvert)));
		//��������
		faceVer.clip_coord = MVP * embed<4>(model->vert(iface, nthvert));
	}

	float inShadow(Vec3f& world_pos, float& bias) {
		//���㶥���ڹ�������ϵ������
		Vec4f clipPosLightSpace = *LightSpaceMatrix * embed<4>(world_pos);
		//����͸�ӳ���
		clipPosLightSpace.w = std::max(0.000000001, clipPosLightSpace.w);
		float light_recip_w = 1 / clipPosLightSpace.w;
		Vec3f ndcLightSpace = proj<3>(clipPosLightSpace * light_recip_w);
		//��Ϊû�н��и��ֲ��У�������ndc��������
		//����Ĭ�ϳ�����ͼ�ĵط��������ܵ����գ�ֱ�ӷ���0
		if (ndcLightSpace.x > 1.0 || ndcLightSpace.x < -1.0)return 0;
		if (ndcLightSpace.y > 1.0 || ndcLightSpace.y < -1.0)return 0;
		if (ndcLightSpace.z > 1.0 || ndcLightSpace.z < -1.0)return 0;
		//ndcLightSpace.x = clamp(ndcLightSpace.x, -1.0, 1.0);
		//ndcLightSpace.y = clamp(ndcLightSpace.y, -1.0, 1.0);
		//ndcLightSpace.z = clamp(ndcLightSpace.z, -1.0, 1.0);

		//�任����Ӱ��ͼ�ķ�Χ
		Vec3f shadowTextureCoords = viewport_transform(SHADOW_WIDTH, SHADOW_HEIGHT, ndcLightSpace);
		//�ӻ����еõ���������
		int x = clamp((int)shadowTextureCoords.x, 0, SHADOW_WIDTH - 1);
		int y = clamp((int)shadowTextureCoords.y, 0, SHADOW_HEIGHT - 1);
		int id = x + y* SHADOW_WIDTH;
		float closestDepth = ShadowBuffer[id];
		//ȡ�õ�ǰƬԪ�ڹ�Դ�ӽ��µ����
		float currentDepth = shadowTextureCoords.z;
		if (defaultCamera.getProjectMode()) {
			//͸��ͶӰ����Ҫ��zֵ���Ի��������ͻ
			float near = defaultCamera.getNear();
			float far = defaultCamera.getFar();
			closestDepth = LinearizeDepth(closestDepth, near, far);
			currentDepth = LinearizeDepth(currentDepth, near, far);
			//std::cout << "ProjectionMode" << defaultCamera.ProjectionMode << std::endl;
		}
		//std::cout << "closestDepth"<< closestDepth <<std::endl;
		//std::cout << "currentDepth"<< currentDepth <<std::endl;
		//// ��鵱ǰƬԪ�Ƿ�����Ӱ��
		return currentDepth - bias > closestDepth ? 1.0 : 0.0;
		//return 0;
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
		float lightPower = 6;
		//������
		float ambient = 0.5f;
		Vec3f vertPos = verInf.world_pos;
		
		//˥��
		Vec3f lightDir = (lightPos - vertPos);
		float dis = lightDir.norm();
		float recip_dis = 1.0f / dis;
		lightDir = lightDir * recip_dis;

		//������
		float diff = clamp<float>(std::abs(lightDir * (-1) * normal), 0, 1);

		//���淴��
		Vec3f viewDir = (defaultCamera.getPos() - vertPos).normalize();
		Vec3f hafe = (lightDir + viewDir).normalize();
		float spec = clamp<float>(std::abs(normal * hafe), 0, 1);

		
		//������Ӱƫ��
		float bias = std::max(0.1 * (1.0 - diff), 0.005);
		//����õ��ڲ�����Ӱ����
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
	//���shadowMap���ݣ����»���
	std::fill(shadowBuffer, shadowBuffer + SHADOW_WIDTH * SHADOW_HEIGHT, 1);
	std::fill(shadowTexture, shadowTexture + SHADOW_WIDTH * SHADOW_WIDTH, white);

	//��������ڷŵ���Դλ��
	//lightCamera.enableProjectMode(false);
	Vec3f lightPos2 = lightPos;
	lightCamera.setCamera(lightPos2, center, up);
	lightCamera.setClipPlane(2,6);
	lightCamera.setFov(50);

	//�������
	ViewMatrix = lightCamera.getViewMatrix();
	ProjectionMatrix = lightCamera.getProjMatrix();
	lightSpaceMatrix = ProjectionMatrix * ViewMatrix;
	MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

	//�ر����޳�
	enableFaceCulling = false;
	//enableFrontFaceCulling = true;

	//������Ӱ��ͼ
	for (int m = 0; m < models.size(); m++) {
		DepthShader depthshader(&lightSpaceMatrix);
		VerInf faceVer[3];
		model = &models[m];
		for (int i = 0; i < model->nfaces(); i++) {
			for (int j = 0; j < 3; j++) {
				depthshader.vertex(i, j, faceVer[j]);
			}
			triangle(faceVer, depthshader,//���붥�����ݺ�shader
					SHADOW_WIDTH, SHADOW_HEIGHT,//������Ļ��С�����Ӵ��任
					lightCamera.getNear(),lightCamera.getFar(),//����͸��Զ��ƽ�����ڲ��к�����zbuffer
					shadowBuffer, shadowTexture,//�������buffer
					false,//�Ƿ�����߿�ģ��
					false//�Ƿ����
					);
		}
	}
	//���������޳�
	enableFaceCulling = true;
	enableFrontFaceCulling = false;
}

void draw(std::vector<Model>& models, ColorVec* drawBuffer,double* shadowBuffer, double* zbuffer) {
	//���drawbuffer��zbuffer�������µĻ���
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
			triangle(faceVer, shader, //���붥�����ݺ�shader
					SCREEN_WIDTH, SCREEN_HEIGHT,//������Ļ��С�����Ӵ��任
					defaultCamera.getNear(), defaultCamera.getFar(), //����͸��Զ��ƽ�����ڲ��к�����zbuffer
					zbuffer, drawBuffer,//�������buffer
					false,//�Ƿ�����߿�ģ��
					true//�Ƿ����
				);
		}
	}
}

//#define showShadow
int main(int argc, char** argv) {
	//�����Ӵ�
	SDLWindow window("SoftRenderer",SCREEN_WIDTH, SCREEN_HEIGHT);
#ifdef showShadow
	SDLWindow shadow("shadow", SHADOW_WIDTH, SHADOW_HEIGHT);
#endif // showShadow
	//���������¼�
	KeyboardAndMouseHandle KMH(SCREEN_WIDTH, SCREEN_HEIGHT,&defaultCamera);

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
			"obj/african_head/african_head.obj",
			"obj/african_head/african_head_eye_inner.obj",
			//"obj/diablo3_pose/diablo3_pose.obj",
			"obj/floor.obj",
			//"obj/floor1.obj",
			//"obj/window.obj",
		};

		//������Ҫ��Ⱦ��ģ��
		std::vector<Model> models;
		for (int m = 0; m < modleName.size(); m++) {
			models.push_back(Model(modleName[m]));
		}

		//�������
		defaultCamera = Camera((float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.3, 10, 60);
		lightCamera = Camera((float)SHADOW_WIDTH / SHADOW_HEIGHT, 0.3, 10, 60);

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
		
		//��Դ��ת
		//angle += 10 * deltaTime;
		//float radio = 2;
		//float lightX = radio * cos(angle * DegToRad);
		//float lightZ = radio * sin(angle * DegToRad);
		//lightPos.x = lightX;
		//lightPos.z = lightZ;
		//����shadow map
		drawShadowMap(models, shadowBuffer, shadowTexture);

		//While application is running
		while (KMH.SDL_Runing){
			//����deltaTime��fps
			timer++;
			clock_t currentFrame = clock();
			deltaTime = float((float)currentFrame - lastFrame)/ CLOCKS_PER_SEC;
			lastFrame = currentFrame;
			if (timer >= 1 / deltaTime) {
				std::cout << "Current Frames Per Second:" << int(1 / deltaTime + 0.5f) << std::endl;
				timer = 0;
			}
			
			//��������¼�
			KMH.getMouseKeyEven(NULL, deltaTime);

			

			
			
			//����shadow map����ģ��
			draw(models, drawBuffer, shadowBuffer, zbuffer);

			//��������
			temp = drawBuffer;
			drawBuffer = showBuffer;
			showBuffer = temp;

			//������Ļ��ʾ����
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


