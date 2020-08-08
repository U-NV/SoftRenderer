#include "Light.h"

inline float Light::beIlluminated(const Vec3f& world_pos, float bias) {
	//计算顶点在光照坐标系的坐标
	Vec4f clipPosLightSpace = lightMatrix * embed<4>(world_pos,1.0f);
	//进行透视除法
	//clipPosLightSpace.w = std::max(0.000000001f, clipPosLightSpace.w);
	float light_recip_w = 1 / clipPosLightSpace.w;
	Vec3f ndcLightSpace = proj<3>(clipPosLightSpace) * light_recip_w;
	//因为没有进行各种裁切，故这里ndc坐标会溢出
	//这里默认超出贴图的地方都可以受到光照，直接返回1
	if (ndcLightSpace.x > 1.0 || ndcLightSpace.x < -1.0)return 1;
	if (ndcLightSpace.y > 1.0 || ndcLightSpace.y < -1.0)return 1;
	if (ndcLightSpace.z > 1.0 || ndcLightSpace.z < -1.0)return 1;
	//变换到阴影贴图的范围
	Vec3f shadowTextureCoords = ShadowPort->transform(ndcLightSpace);
	//从缓存中得到最近的深度
	int x = clamp((int)shadowTextureCoords.x, 0, ShadowPort->v_width - 1);
	int y = clamp((int)shadowTextureCoords.y, 0, ShadowPort->v_height - 1);
	int id = x + y * ShadowPort->v_width;
	float closestDepth = depthBuffer[id];

	//取得当前片元在光源视角下的深度
	float currentDepth = shadowTextureCoords.z;
	//if (lightCamera->getProjectMode()) {
	//	//透视投影中需要对z值线性化，避免冲突
	//	float near = lightCamera->getNear();
	//	float far = lightCamera->getFar();
	//	closestDepth = LinearizeDepth(closestDepth, near, far);
	//	currentDepth = LinearizeDepth(currentDepth, near, far);
	//	//std::cout << "ProjectionMode" << defaultCamera.ProjectionMode << std::endl;
	//}
	//// 检查当前片元是否在阴影中
	return currentDepth - bias >= closestDepth ? 0.0 : 1.0;
	//return 0;
}

Vec3f PointLight::calcLightColor(const Vec3f& normal, const  Vec3f& worldPos, const  Vec3f& viewDir, const  Material& material)
{
	//beIlluminated(worldPos,0.0f);

	//衰减
	Vec3f lightDir = (lightPos - worldPos);
	float distance = lightDir.norm();
	lightDir = lightDir / distance;
	//衰减系数
	float attenuation = 1.0f / (constant + linear * distance + quadratic * (distance * distance));

	//漫反射
	float diff = std::max(normal * lightDir, 0.0f);

	Vec3f diffColor(0, 0, 0);
	Vec3f specColor(0, 0, 0);
	if (diff > 0.0f) {
		diffColor = colorMulit(lightColor * diff * attenuation * lightPower, material.diffuse);
		//镜面反射强度
		Vec3f hafe = (lightDir + viewDir).normalize();
		float spec = std::pow(std::max(normal * hafe, 0.0f), material.shininess);
		//计算反射颜色
		specColor = colorMulit(lightColor * spec * attenuation * lightPower, material.specular);
	}
	else {
		return diffColor;
	}

	//计算该点在不在阴影区域
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
	//漫反射
	float diff = std::max(normal * lightDir, 0.0f);
	Vec3f diffColor = colorMulit(lightColor * diff * lightPower, material.diffuse);

	//镜面反射强度
	Vec3f hafe = (lightDir + viewDir).normalize();
	float spec = std::pow(std::max(normal * hafe, 0.0f), material.shininess);
	//计算反射颜色
	Vec3f specColor = colorMulit(lightColor * spec * lightPower, material.specular);

	//计算该点在不在阴影区域
	if (enableShadow) {
		float bias = std::max(0.1 * (1.0 - diff), 0.005);
		return (diffColor + specColor) * beIlluminated(worldPos, bias);
	}
	else {
		return (diffColor + specColor);
	}
}
