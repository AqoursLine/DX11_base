#include "main.h"
#include "light.h"

#include "renderer.h"


void Light::CalculateLightMatrices() {
	// ライトビュー行列の計算
	XMVECTOR lightPos = XMVectorSet(m_position.x, m_position.y, m_position.z, 1.0f);
	XMVECTOR lightDir = XMVectorSet(m_direction.x, m_direction.y, m_direction.z, 0.0f);
	XMVECTOR upDir = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_lightViewMatrix = XMMatrixLookToLH(lightPos, lightDir, upDir);

	// ライト射影行列の計算
	float nearPlane = 0.1f;
	float farPlane = 100.0f;
	if (m_type == LIGHT_TYPE::DIRECTIONAL) {
		// 直線光源の場合は正射影行列を使用
		float orthoSize = 20.0f; // 適切なサイズに調整
		m_lightProjectionMatrix = XMMatrixOrthographicLH(orthoSize, orthoSize, nearPlane, farPlane);
	} else {
		// スポットライトの場合は透視投影行列を使用(コーン角度に基づく)
		float fovAngleY = m_outerCone * 2.0f; // 外側コーン角度を使用
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
	// レンダーターゲットをシャドウマップ用に設定
	RENDERER.SetShadowMapAsRenderTarget(m_shadowMapIndex);

	// シャドウマップ生成用のマトリクス設定
	RENDERER.SetViewMatrix(m_lightViewMatrix);
	RENDERER.SetProjectionMatrix(m_lightProjectionMatrix);
}
