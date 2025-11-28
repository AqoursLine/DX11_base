#include "main.h"
#include "light.h"

#include "renderer.h"

#ifdef _DEBUG
#include "imguiSystem.h"

#endif // _DEBUG


void Light::CalculateLightMatrices() {

	// ライト射影行列の計算
	float nearPlane = 0.01f;
	float farPlane = m_range;
	if (m_type == LIGHT_TYPE::DIRECTIONAL) {
		// ライトビュー行列の計算
		XMVECTOR sceneCenter = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f); // シーンの中心点（適宜変更）
		XMVECTOR lightDir = XMVectorSet(m_direction.x, m_direction.y, m_direction.z, 0.0f);

		// ライトの位置をシーン中心から一定距離離す
		XMVECTOR lightPos = sceneCenter - XMVectorScale(lightDir, 50.0f); // 50.0fは適宜調整

		XMVECTOR upDir = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // 上方向ベクトル

		// ビュー行列の作成
		m_lightViewMatrix = XMMatrixLookAtLH(lightPos, sceneCenter, upDir);

		// 直線光源の場合は正射影行列を使用
		float orthoSize = 20.0f; // 適切なサイズに調整
		m_lightProjectionMatrix = XMMatrixOrthographicLH(orthoSize, orthoSize, nearPlane, farPlane);
	} else {
		// ライトビュー行列の計算
		XMVECTOR lightPos = XMVectorSet(m_position.x, m_position.y, m_position.z, 1.0f);
		XMVECTOR lightDir = XMVectorSet(m_direction.x, m_direction.y, m_direction.z, 0.0f);
		XMVECTOR upDir = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // 上方向ベクトル

		m_lightViewMatrix = XMMatrixLookToLH(lightPos, lightDir, upDir);

		// スポットライトの場合は透視投影行列を使用(コーン角度に基づく)
		float fovAngleY = m_outerCone; // 外側コーン角度を使用
		float aspectRatio = 1.0f; // 正方形の影マップを想定
		m_lightProjectionMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearPlane, farPlane);
	}
}

bool Light::Initialize() {
	SetShadowCaster(m_isShadowCaster);
	return true;
}

void Light::Finalize() {
}

void Light::Update(double deltaTime) {
}

void Light::Draw() {

#ifdef _DEBUG
	std::string title = "Light Debug Settings##" + std::to_string(reinterpret_cast<uintptr_t>(this));
	ImGui::Begin(title.c_str());
	ImGui::Text("Light Type: %d", static_cast<int>(m_type));

	ImGui::SliderFloat3("Position", (float*)&m_position, -100.0f, 100.0f);
	float direction[3] = { m_direction.x,  m_direction.y,  m_direction.z };

	if (ImGui::SliderFloat3("Direction", direction, -1.0f, 1.0f)) {
		SetDirection(Vector4(direction[0], direction[1], direction[2], 0.0f)); // 正規化も兼ねる
	}

	ImGui::SliderFloat("Intensity", &m_intensity, 0.0f, 10.0f);
	ImGui::SliderFloat("Range", &m_range, 0.1f, 100.0f);
	ImGui::ColorEdit4("Diffuse Color", (float*)&m_diffuseColor);
	float degree = m_innerCone * (180.0f / XM_PI);
	if(ImGui::SliderFloat("Inner Cone", &degree, 0.0f, 180.0f)){
		m_innerCone = degree * (XM_PI / 180.0f);
	}
	degree = m_outerCone * (180.0f / XM_PI);
	if(ImGui::SliderFloat("Outer Cone", &degree, 0.001f, 180.0f)){
		m_outerCone = degree * (XM_PI / 180.0f);
	}
	ImGui::SliderFloat("Falloff", &m_falloff, 0.0f, 5.0f);
	ImGui::SliderFloat3("Attenuation", (float*)&m_attenuationConstant, 0.0f, 5.0f);

	ImGui::End();

#endif // _DEBUG

	if (m_isShadowCaster) {
		// レンダーターゲットのクリア
		RENDERER.ClearShadowMap(m_shadowMapIndex);

		// レンダーターゲットをシャドウマップ用に設定
		RENDERER.SetShadowMapAsRenderTarget(m_shadowMapIndex);

		// シャドウマップ生成用のマトリクス設定
		RENDERER.SetViewMatrix(m_lightViewMatrix);
		RENDERER.SetProjectionMatrix(m_lightProjectionMatrix);
	}
}
