#include "lightManager.h"
#include "renderer.h"
#include "scene.h"
#include "light.h"

bool LightManager::Initialize() {
	return true;
}

void LightManager::Finalize() {}

void LightManager::Update(double deltaTime) {}

void LightManager::Draw() {
	//ライトの情報をシェーダーにセット
	auto lights = m_scene->GetGameObjectsOfType<Light>(TYPE_LIGHT);

	LIGHTS lightData = {};

	std::vector<Light*> directionalLights;
	std::vector<Light*> pointLights;
	std::vector<Light*> spotLights;

	for (auto& light : lights) {
		if (!light->IsActive()) continue;

		switch (light->GetType()) {
		case LIGHT_TYPE::DIRECTIONAL:
			directionalLights.push_back(light);
			break;
		case LIGHT_TYPE::POINT:
			pointLights.push_back(light);
			break;
		case LIGHT_TYPE::SPOT:
			spotLights.push_back(light);
			break;
		}
	}

	lightData.directionalLightCount = static_cast<UINT>(directionalLights.size());
	lightData.pointLightCount = static_cast<UINT>(pointLights.size());
	lightData.spotLightCount = static_cast<UINT>(spotLights.size());

	int lightIndex = 0;

	// 平行光源の情報をセット
	for (auto& light : directionalLights) {
		if (lightIndex >= MAX_LIGHTS) break;
		lightData.lights[lightIndex].positionAndType = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f); // 平行光源は位置を使用しない
		Vector4 dir = light->GetDirection();
		lightData.lights[lightIndex].directionAndIntensity = XMFLOAT4(dir.x, dir.y, dir.z, light->GetIntensity());
		Vector4 color = light->GetDiffuseColor();
		lightData.lights[lightIndex].diffuseAndRange = XMFLOAT4(color.x, color.y, color.z, light->GetRange());
		// スポットライト用パラメータは無効化
		lightData.lights[lightIndex].spotParams = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		lightData.lights[lightIndex].attenuation = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
		lightIndex++;
	}

	// 点光源の情報をセット
	for (auto& light : pointLights) {
		if (lightIndex >= MAX_LIGHTS) break;
		Vector3 pos = light->GetPosition();
		lightData.lights[lightIndex].positionAndType = XMFLOAT4(pos.x, pos.y, pos.z, 1.0f); // w成分にライトタイプ(1:点)
		lightData.lights[lightIndex].directionAndIntensity = XMFLOAT4(0.0f, 0.0f, 0.0f, light->GetIntensity());
		Vector4 color = light->GetDiffuseColor();
		lightData.lights[lightIndex].diffuseAndRange = XMFLOAT4(color.x, color.y, color.z, light->GetRange());
		// スポットライト用パラメータは無効化
		lightData.lights[lightIndex].spotParams = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		lightData.lights[lightIndex].attenuation = XMFLOAT4(light->GetAttenuationConstant(), light->GetAttenuationLinear(), light->GetAttenuationQuadratic(), 0.0f);
		lightIndex++;
	}

	// スポットライトの情報をセット
	for (auto& light : spotLights) {
		if (lightIndex >= MAX_LIGHTS) break;
		Vector3 pos = light->GetPosition();
		lightData.lights[lightIndex].positionAndType = XMFLOAT4(pos.x, pos.y, pos.z, 2.0f); // w成分にライトタイプ(2:スポット)
		Vector4 dir = light->GetDirection();
		lightData.lights[lightIndex].directionAndIntensity = XMFLOAT4(dir.x, dir.y, dir.z, light->GetIntensity());
		Vector4 color = light->GetDiffuseColor();
		lightData.lights[lightIndex].diffuseAndRange = XMFLOAT4(color.x, color.y, color.z, light->GetRange());
		lightData.lights[lightIndex].spotParams = XMFLOAT4(light->GetInnerCone(), light->GetOuterCone(), light->GetFalloff(), light->IsEnabled() ? 1.0f : 0.0f);
		lightData.lights[lightIndex].attenuation = XMFLOAT4(light->GetAttenuationConstant(), light->GetAttenuationLinear(), light->GetAttenuationQuadratic(), 0.0f);
		lightIndex++;
	}
	
	// シェーダーにライトデータをセット
	RENDERER.SetLights(lightData);

	// シャドウキャスターのリストを更新
	std::vector<Light*> shadowCasters;
	for (auto& light : lights) {
		if (light->IsActive() && light->IsShadowCaster()) {
			shadowCasters.push_back(light);
		}
	}

	// シャドウキャスターの数を制限
	if (shadowCasters.size() > MAX_SHADOW_LIGHTS) {
		//超過分のライトのシャドウキャストを無効化
		for (size_t i = MAX_SHADOW_LIGHTS; i < shadowCasters.size(); i++) {
			shadowCasters[i]->SetShadowCaster(false);
		}
		shadowCasters.resize(MAX_SHADOW_LIGHTS);
	}

	// シャドウキャスター情報をシェーダーにセット
	SHADOW_LIGHTS shadowLightData = {};
	for (size_t i = 0; i < shadowCasters.size(); i++) {
		auto& light = shadowCasters[i];
		light->CalculateLightMatrices();
		XMMATRIX viewMatrix = light->GetLightViewMatrix();
		XMMATRIX projMatrix = light->GetLightProjectionMatrix();
		XMMATRIX biasMatrix = XMMatrixSet(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f
		);
		XMMATRIX lightMatrix = XMMatrixMultiply(viewMatrix, projMatrix);
		lightMatrix = XMMatrixMultiply(lightMatrix, biasMatrix);
		shadowLightData.shadowLights[i] = lightMatrix;

		// シャドウマップのインデックスをセット
		light->SetShadowMapIndex(static_cast<int>(i));
	}
	shadowLightData.shadowLightCount = static_cast<UINT>(shadowCasters.size());

	RENDERER.SetShadowLights(shadowLightData);
}
