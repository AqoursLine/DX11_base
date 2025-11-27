#include "main.h"
#include "light.h"

#include "renderer.h"

#ifdef _DEBUG
#include "imguiSystem.h"

#endif // _DEBUG


void Light::CalculateLightMatrices() {

	// ライト射影行列の計算
	float nearPlane = 0.01f;
	float farPlane = 100.0f;
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

//#ifdef _DEBUG
//	ImGui::Begin("Light Debug");
//	ImGui::Text("Light Type: %d", static_cast<int>(m_type));
//	ImGui::Text("Position: (%.2f, %.2f, %.2f)", m_position.x, m_position.y, m_position.z);
//	ImGui::Text("Direction: (%.2f, %.2f, %.2f)", m_direction.x, m_direction.y, m_direction.z);
//	ImGui::Text("Intensity: %.2f", m_intensity);
//	ImGui::Text("Diffuse Color: (%.2f, %.2f, %.2f, %.2f)", m_diffuseColor.x, m_diffuseColor.y, m_diffuseColor.z, m_diffuseColor.w);
//	ImGui::Text("Range: %.2f", m_range);
//	ImGui::Text("Inner Cone: %.2f", m_innerCone);
//	ImGui::Text("Outer Cone: %.2f", m_outerCone);
//	ImGui::Text("Falloff: %.2f", m_falloff);
//	ImGui::Text("Enabled: %s", IsEnabled() ? "True" : "False");
//
//	ImGui::Text("Light View Matrix:");
//	for (int i = 0; i < 4; ++i) {
//		XMFLOAT4 row;
//		XMStoreFloat4(&row, m_lightViewMatrix.r[i]);
//		ImGui::Text("[%.2f, %.2f, %.2f, %.2f]", row.x, row.y, row.z, row.w);
//	}
//	ImGui::Text("Light Projection Matrix:");
//	for (int i = 0; i < 4; ++i) {
//		XMFLOAT4 row;
//		XMStoreFloat4(&row, m_lightProjectionMatrix.r[i]);
//		ImGui::Text("[%.2f, %.2f, %.2f, %.2f]", row.x, row.y, row.z, row.w);
//	}
//	ImGui::Text("Shadow Map Index: %d", m_shadowMapIndex);
//
//	ImGui::End();
//
//#endif // _DEBUG

	// レンダーターゲットのクリア
	RENDERER.ClearShadowMap(m_shadowMapIndex);

	// レンダーターゲットをシャドウマップ用に設定
	RENDERER.SetShadowMapAsRenderTarget(m_shadowMapIndex);

	// シャドウマップ生成用のマトリクス設定
	RENDERER.SetViewMatrix(m_lightViewMatrix);
	RENDERER.SetProjectionMatrix(m_lightProjectionMatrix);

}
