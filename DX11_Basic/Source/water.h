#pragma once

#include "gameObject.h"

/// <summary>
/// 波紋データ構造体
/// </summary>
struct Ripple {
	Vector3 position;
	float amplitude;
	float frequency;
	float speed;
	float time;
	bool active;
};

/// <summary>
/// 水システム
/// </summary>
class Water : public GameObject {
public:
	Water();
	virtual ~Water() = default;

	// ==== 基本設定 ====
	void SetWaterSize(float size) { m_waterSize = size; }
	void SetWaveHeight(float height) { m_waveHeight = height; }
	float GetWaterSize() const { return m_waterSize; }
	float GetWaveHeight() const { return m_waveHeight; }

	// ==== 環境マッピングパラメータ設定 ====
	void SetEnvironmentMap(ID3D11ShaderResourceView* srv) { m_environmentMapSRV = srv; }
	void SetReflectionStrength(float strength) { m_reflectionStrength = strength; }
	void SetRefractionStrength(float strength) { m_refractionStrength = strength; }
	void SetFresnelPower(float power) { m_fresnelPower = power; }
	void SetWaterClarityDepth(float depth) { m_waterClarityDepth = depth; }

	// ==== 基本波パラメータ設定 ====
	void SetWaveSharpness(float sharpness) { m_waveSharpness = sharpness; }
	float GetWaveSharpness() const { return m_waveSharpness; }

	void SetBaseWaveParameters(float freq1, float freq2, float freq3,
		float speed1, float speed2, float speed3)
	{
		m_baseWaveFreqency1 = freq1;
		m_baseWaveFreqency2 = freq2;
		m_baseWaveFreqency3 = freq3;
		m_baseWaveSpeed1 = speed1;
		m_baseWaveSpeed2 = speed2;
		m_baseWaveSpeed3 = speed3;
	}

	// ==== 波紋管理 ====
	
	/// <summary>
	/// 波紋追加
	/// </summary>
	/// <param name="position">座標</param>
	/// <param name="amplitude">振幅</param>
	/// <param name="frequency">周波数</param>
	/// <param name="speed">速度</param>
	void AddRipple(const Vector3& position, float amplitude = 2.0f, float frequency = 2.0f, float speed = 8.0f);
	int GetActiveRippleCount() const { return m_activeRippleCount; }

	// ==== 波高取得（CPU計算） ====
	float GetWaterHeight(const Vector3& position) const;
	Vector3 GetWaterNormal(const Vector3& position) const;

protected:
	virtual bool Initialize() override;
	virtual void Finalize() override;
	virtual void Update(double deltaTime) override;
	virtual void Draw() override;
private:
	constexpr static int MAX_RIPPLES = 32; // 最大波紋数

	/// <summary>
	/// 定数バッファ構造体
	/// </summary>
	struct WaterConstantBuffer {
		float time;
		float waveHeight;
		float waterSize;
		int activeRippleCount;

		// 基本波パラメータ
		float baseWaveFreq1;
		float baseWaveFreq2;
		float baseWaveFreq3;
		float baseWaveSpeed1;
		float baseWaveSpeed2;
		float baseWaveSpeed3;
		float waveSharpness;
		float padding3; // パディング

		// 環境マッピング用パラメータ
		float reflectionStrength;
		float refractionStrength;
		float fresnelPower;
		float waterClarityDepth;

		// 波紋データ
		struct {
			XMFLOAT4 positionAndTime;	// xyz:位置、w:時間
			XMFLOAT4 params;			// x:振幅 y:波長 z:速度 w:使用フラグ
		} ripples[MAX_RIPPLES];
	};

	// メッシュ生成
	void CreateMesh();
	void CreateNormalMap();
	void CreateFoamTexture();

	//定数バッファ更新
	void UpdateConstantBuffer();

	//CPU側波紋計算
	float CalculateWaveHeight(const Vector3& position, float time) const;

	//バッファ
	ID3D11Buffer* m_vertexBuffer = nullptr;
	ID3D11Buffer* m_indexBuffer = nullptr;
	ID3D11Buffer* m_constantBuffer = nullptr;

	//シェーダー
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;

	//テクスチャ
	class Texture* m_normalMap = nullptr;
	class Texture* m_foamTexture = nullptr;
	ID3D11ShaderResourceView* m_environmentMapSRV = nullptr;

	// サンプラーステート
	ID3D11SamplerState* m_samplerState = nullptr;

	// 水面パラメータ
	float m_waterSize;
	float m_waveHeight;
	float m_time;
	int m_gridResolution;

	//基本波のパラメータ
	float m_baseWaveFreqency1;
	float m_baseWaveFreqency2;
	float m_baseWaveFreqency3;
	float m_baseWaveSpeed1;
	float m_baseWaveSpeed2;
	float m_baseWaveSpeed3;
	float m_waveSharpness;

	// 環境マッピング用パラメータ
	float m_reflectionStrength;
	float m_refractionStrength;
	float m_fresnelPower;
	float m_waterClarityDepth;

	// 波紋管理
	std::vector<Ripple> m_ripples;
	int m_activeRippleCount;

	// メッシュ情報
	UINT m_indexCount;
};
