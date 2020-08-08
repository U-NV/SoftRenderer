#include "Light.h"

inline float Light::beIlluminated(const Vec3f& world_pos, float bias) {
	//���㶥���ڹ�������ϵ������
	Vec4f clipPosLightSpace = lightMatrix * embed<4>(world_pos,1.0f);
	//����͸�ӳ���
	//clipPosLightSpace.w = std::max(0.000000001f, clipPosLightSpace.w);
	float light_recip_w = 1 / clipPosLightSpace.w;
	Vec3f ndcLightSpace = proj<3>(clipPosLightSpace) * light_recip_w;
	//��Ϊû�н��и��ֲ��У�������ndc��������
	//����Ĭ�ϳ�����ͼ�ĵط��������ܵ����գ�ֱ�ӷ���1
	if (ndcLightSpace.x > 1.0 || ndcLightSpace.x < -1.0)return 1;
	if (ndcLightSpace.y > 1.0 || ndcLightSpace.y < -1.0)return 1;
	if (ndcLightSpace.z > 1.0 || ndcLightSpace.z < -1.0)return 1;
	//�任����Ӱ��ͼ�ķ�Χ
	Vec3f shadowTextureCoords = ShadowPort->transform(ndcLightSpace);
	//�ӻ����еõ���������
	int x = clamp((int)shadowTextureCoords.x, 0, ShadowPort->v_width - 1);
	int y = clamp((int)shadowTextureCoords.y, 0, ShadowPort->v_height - 1);
	int id = x + y * ShadowPort->v_width;
	float closestDepth = depthBuffer[id];

	//ȡ�õ�ǰƬԪ�ڹ�Դ�ӽ��µ����
	float currentDepth = shadowTextureCoords.z;
	//if (lightCamera->getProjectMode()) {
	//	//͸��ͶӰ����Ҫ��zֵ���Ի��������ͻ
	//	float near = lightCamera->getNear();
	//	float far = lightCamera->getFar();
	//	closestDepth = LinearizeDepth(closestDepth, near, far);
	//	currentDepth = LinearizeDepth(currentDepth, near, far);
	//	//std::cout << "ProjectionMode" << defaultCamera.ProjectionMode << std::endl;
	//}
	//// ��鵱ǰƬԪ�Ƿ�����Ӱ��
	return currentDepth - bias >= closestDepth ? 0.0 : 1.0;
	//return 0;
}

Vec3f PointLight::calcLightColor(const Vec3f& normal, const  Vec3f& worldPos, const  Vec3f& viewDir, const  Material& material)
{
	//beIlluminated(worldPos,0.0f);

	//˥��
	Vec3f lightDir = (lightPos - worldPos);
	float distance = lightDir.norm();
	lightDir = lightDir / distance;
	//˥��ϵ��
	float attenuation = 1.0f / (constant + linear * distance + quadratic * (distance * distance));

	//������
	float diff = std::max(normal * lightDir, 0.0f);

	Vec3f diffColor(0, 0, 0);
	Vec3f specColor(0, 0, 0);
	if (diff > 0.0f) {
		diffColor = colorMulit(lightColor * diff * attenuation * lightPower, material.diffuse);
		//���淴��ǿ��
		Vec3f hafe = (lightDir + viewDir).normalize();
		float spec = std::pow(std::max(normal * hafe, 0.0f), material.shininess);
		//���㷴����ɫ
		specColor = colorMulit(lightColor * spec * attenuation * lightPower, material.specular);
	}
	else {
		return diffColor;
	}

	//����õ��ڲ�����Ӱ����
	if (enableShadow) {
		float bias = std::max(0.1 * (1.0 - diff), 0.005);
		return (diffColor + specColor) * beIlluminated(worldPos, bias);
	}
	else {
		return (diffColor + specColor);
	}
}


Vec3f DirLight::calcLightColor(const Vec3f& normal, const  Vec3f& worldPos, const  Vec3f& viewDir, const  Material& material)
{
	//������
	float diff = std::max(normal * lightDir, 0.0f);
	Vec3f diffColor = colorMulit(lightColor * diff * lightPower, material.diffuse);

	//���淴��ǿ��
	Vec3f hafe = (lightDir + viewDir).normalize();
	float spec = std::pow(std::max(normal * hafe, 0.0f), material.shininess);
	//���㷴����ɫ
	Vec3f specColor = colorMulit(lightColor * spec * lightPower, material.specular);

	//����õ��ڲ�����Ӱ����
	if (enableShadow) {
		float bias = std::max(0.1 * (1.0 - diff), 0.005);
		return (diffColor + specColor) * beIlluminated(worldPos, bias);
	}
	else {
		return (diffColor + specColor);
	}
}
