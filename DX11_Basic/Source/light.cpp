#include "main.h"
#include "light.h"

#include "renderer.h"

// 静的メンバ変数の初期化
int Light::s_maxLights = MAX_LIGHTS;
std::vector<bool> Light::s_usedLightMask(MAX_LIGHTS, false);

int Light::GetCurrentLightCount() {
	int count = 0;
	for (bool used : s_usedLightMask) {
		if (used) {
			count++;
		}
	}
	return count;
}

bool Light::Initialize() {
	m_lightIndex = AllocateLightIndex();
	if (m_lightIndex == -1) {
		// 空きがない場合は初期化失敗
		return false;
	}

	return true;
}

void Light::Finalize() {
	if (m_lightIndex != -1) {
		ReleaseLightIndex(m_lightIndex);

		//シェーダーからライト情報をクリア
		LIGHT emptyLight {};
		RENDERER.SetLight(emptyLight, m_lightIndex);

		m_lightIndex = -1;
	}
}

void Light::Update(double deltaTime) {
}

void Light::Draw() {
	if (m_lightIndex == -1) {
		return; // 無効なライトインデックスの場合は何もしない
	}

	// ライト情報を設定
	LIGHT lightData {};
	lightData.positionAndType = XMFLOAT4(m_position.x, m_position.y, m_position.z, static_cast<float>(m_type));
	lightData.directionAndIntensity = XMFLOAT4(m_direction.x, m_direction.y, m_direction.z, m_intensity);
	lightData.diffuseAndRange = XMFLOAT4(m_diffuseColor.x, m_diffuseColor.y, m_diffuseColor.z, m_range);
	lightData.spotParams = XMFLOAT4(m_innerCone, m_outerCone, m_falloff, m_enabled);
	lightData.attenuation = XMFLOAT4(m_attenuationConstant, m_attenuationLinear, m_attenuationQuadratic, 0.0f);

	RENDERER.SetLight(lightData, m_lightIndex);
}

int Light::AllocateLightIndex() {
	for (int i = 0; i < s_maxLights; i++) {
		if (!s_usedLightMask[i]) {
			s_usedLightMask[i] = true; // 使用済みフラグを設定
			return i;
		}
	}

	return -1; // 空きがない場合
}

void Light::ReleaseLightIndex(int index) {
	if (index >= 0 && index < s_maxLights) {
		s_usedLightMask[index] = false; // 使用済みフラグをクリア
	}
}
