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

//MVP������ز���
Vec3f up(0, 1, 0);
Vec3f right(1, 0, 0);
Vec3f forword(0, 0, 1);

Model* model;
Vec3f lightPos(1, 0.5,1);

clock_t lastFrame = 0;
float deltaTime = 0;

struct Shader : public IShader {
	virtual void vertex(int iface, int nthvert, VerInf& faceVer) {//�Ե�iface�ĵ�nthvert��������б任
		//���ģ��uv
		faceVer.uv = model->uv(iface, nthvert);
		//���߱任
		faceVer.normal = proj<3>(MVP.invert_transpose() * embed<4>(model->normal(iface, nthvert), 0.f));
		//��������
		faceVer.world_pos = proj<3>(ModelMatrix * embed<4>(model->vert(iface, nthvert)));
		//��������
		faceVer.clip_coord = MVP * embed<4>(model->vert(iface, nthvert));
	}

	virtual bool fragment(VerInf verInf, TGAColor& color) {
		//��ֵuv
		Vec2f uv = verInf.uv;

		//��ֵ������
		//Vec3f normal = verInf.normal;
		Vec3f normal = model->normal(uv); 

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

		//Vec3f normal = (B * model->normal(uv)).normalize();

		//���ù���
		float ambient = 0.0f;
		float lightPower = 2;

		Vec3f vertPos = verInf.world_pos;
		Vec3f lightDir = (lightPos - vertPos);
		float dis = lightDir.norm();
		lightDir = lightDir / dis;

		Vec3f viewDir = (defaultCamera.camPos - vertPos).normalize();

		Vec3f hafe = (lightDir + viewDir).normalize();

		float diff = clamp<float>(std::abs(lightDir * normal), 0, 1);
		float spec = clamp<float>(std::abs(normal * hafe), 0, 1);

		TGAColor c = model->diffuse(uv);
		color = c;
		for (int i = 0; i < 3; i++) {
			float rate = clamp<float>(ambient + (1 * diff + 1 * spec)/dis / dis * lightPower,0,10);
			color[i] = clamp<int>((float)c[i] * rate,0,255);
		}

		return false;
	}
};

int main(int argc, char** argv) {
	//�����Ӵ�
	SDLWindow window(SCREEN_WIDTH, SCREEN_HEIGHT);
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
		defaultCamera = Camera((float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.3, 3, 60);

		//��ʼ��MVP����
		ViewMatrix = Matrix::identity();
		ModelMatrix = Matrix::identity();
		ProjectionMatrix = Matrix::identity();

		//˫���� �� zbuffer
		double* zbuffer = new double[SCREEN_HEIGHT * SCREEN_WIDTH];
		ColorVec* drawBuffer = new ColorVec[SCREEN_HEIGHT * SCREEN_WIDTH];
		ColorVec* showBuffer = new ColorVec[SCREEN_HEIGHT * SCREEN_WIDTH];
		ColorVec* temp = NULL;

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

			//���drawbuffer��zbuffer�������µĻ���
			ColorVec backGroundColor(100, 30, 0x00, 0xFF);
			std::fill(zbuffer, zbuffer + SCREEN_WIDTH * SCREEN_HEIGHT, 1);
			std::fill(drawBuffer, drawBuffer + SCREEN_WIDTH * SCREEN_HEIGHT, backGroundColor);

			//����MVP����
			//����ִ�����ţ�������ת��������ƽ��
			//ModelMatrix = translate(0, 1, 0) * rotate(up, timer) * scale(1, 1, 1);
			ViewMatrix = defaultCamera.getViewMatrix();
			ProjectionMatrix = defaultCamera.getProjMatrix();
			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

			//��Դ��ת
			//angle += 60 * deltaTime;
			//float radio = 2;
			//float lightX = radio * cos(angle * DegToRad);
			//float lightZ = radio * sin(angle * DegToRad);
			//lightPos.x = lightX;
			//lightPos.z = lightZ;

			//����ģ�͵�������Ƭ
			for (int m = 0; m < models.size(); m++) {
				Shader shader;
				VerInf faceVer[3];
				model = &models[m];
				for (int i = 0; i < model->nfaces(); i++) {
					for (int j = 0; j < 3; j++) {
						 shader.vertex(i, j, faceVer[j]);
					}
					triangle(false,faceVer,shader, SCREEN_WIDTH, SCREEN_HEIGHT,zbuffer,drawBuffer);
					//triangle(true,faceVer,shader, SCREEN_WIDTH, SCREEN_HEIGHT, zbuffer, drawBuffer);
				}
			}

			//��������
			temp = drawBuffer;
			drawBuffer = showBuffer;
			showBuffer = temp;

			//���������ݸ�ֵ����Ļ
			for (int x = 0; x < SCREEN_WIDTH; x++) {
				for (int y = 0; y < SCREEN_HEIGHT; y++) {
					window.drawPixel(x, y, SCREEN_WIDTH, SCREEN_HEIGHT, showBuffer);
				}
			}

			//������Ļ��ʾ����
			window.refresh();
			
		}
		delete[] zbuffer;
		delete[] showBuffer;
		delete[] drawBuffer;
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


