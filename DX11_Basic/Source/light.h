#pragma once

#include "gameObject.h"

#include <vector>

enum class LIGHT_TYPE {
	DIRECTIONAL = 0,
	POINT,
	SPOT,
};

class Light : public GameObject {
public:
	Light() = default;
	~Light() = default;

	static int GetCurrentLightCount();

	// ライトの各種パラメータ設定用セッター
	Light* SetType(LIGHT_TYPE type) { m_type = type; return this; }
	Light* SetDirection(const Vector4& direction) { m_direction = direction; m_direction.Normalize(); return this; }
	Light* SetIntensity(float intensity) { m_intensity = intensity; return this; }
	Light* SetDiffuseColor(const Vector4& color) { m_diffuseColor = color; return this; }
	Light* SetRange(float range) { m_range = range; return this; }
	Light* SetInnerCone(float innerCone) { m_innerCone = innerCone; return this; }
	Light* SetOuterCone(float outerCone) { m_outerCone = outerCone; return this; }
	Light* SetFalloff(float falloff) { m_falloff = falloff; return this; }
	Light* SetEnabled(bool enabled) { m_enabled = enabled ? 1.0f : 0.0f; return this; }
	Light* SetAttenuation(float constant, float linear, float quadratic) { m_attenuationConstant = constant; m_attenuationLinear = linear; m_attenuationQuadratic = quadratic; return this; }

protected:
	virtual bool Initialize() override;
	virtual void Finalize() override;
	virtual void Update(double deltaTime) override;
	virtual void Draw() override;

	LIGHT_TYPE m_type = LIGHT_TYPE::DIRECTIONAL;

	Vector4 m_direction = Vector4(0.0f, -1.0f, 0.0f, 0.0f); // ライトの方向（平行光源用）
	float m_intensity = 1.0f; // 光の強さ
	Vector4 m_diffuseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 拡散反射色
	float m_range = 10.0f; // 光の届く範囲（点光源、スポットライト用）

	float m_innerCone = 0.9f; // スポットライトの内側コーン角度（0.0〜1.0）
	float m_outerCone = 0.8f; // スポットライトの外側コーン角度（0.0〜1.0）
	float m_falloff = 1.0f; // スポットライトの減衰（1.0で線形、2.0で二次）
	float m_enabled = 1.0f; // ライトの有効フラグ（1.0で有効、0.0で無効）
	float m_attenuationConstant = 1.0f; // 減衰定数項
	float m_attenuationLinear = 0.09f; // 減衰線形項
	float m_attenuationQuadratic = 0.032f; // 減衰二次項

private:

	int m_lightIndex = -1; // ライトインデックス（シェーダー用）

	//使用済みインデックス管理
	static int s_maxLights;
	static std::vector<bool> s_usedLightMask;

	int AllocateLightIndex();
	void ReleaseLightIndex(int index);
};
