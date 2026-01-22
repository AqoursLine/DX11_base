#pragma once

#include "gameObject.h"
#include <array>

//==============================================================================
// 定数定義
//==============================================================================
namespace WaterConstants {
	// 波紋
	constexpr int MAX_RIPPLES = 32;

	// グリッド解像度
	constexpr int GRID_RESOLUTION = 512;
	constexpr int CS_THREAD_GROUP_SIZE = 16;

	// 水面サイズ
	constexpr float DEFAULT_WATER_SIZE = 512.0f;
	constexpr float DEFAULT_WAVE_HEIGHT = 2.0f;
	constexpr float DEFAULT_WAVE_SHARPNESS = 2.0f;

	// 波紋パラメータ
	constexpr float RIPPLE_LIFETIME = 5.0f;
	constexpr float RIPPLE_TIME_ATTENUATION = 0.5f;
	constexpr float RIPPLE_DISTANCE_ATTENUATION = 0.01f;

	// ディスパッチ数の事前計算
	constexpr int DISPATCH_GROUPS_X = GRID_RESOLUTION / CS_THREAD_GROUP_SIZE;
	constexpr int DISPATCH_GROUPS_Y = GRID_RESOLUTION / CS_THREAD_GROUP_SIZE;

	// コンパイル検証
	static_assert(GRID_RESOLUTION% CS_THREAD_GROUP_SIZE == 0, "GRID_RESOLUTION must be multiple of CS_THREAD_GROUP_SIZE");
}

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

	Ripple()
		: position(0.0f, 0.0f, 0.0f)
		, amplitude(0.0f)
		, frequency(0.0f)
		, speed(0.0f)
		, time(0.0f)
		, active(false)
	{
	}
};

/// <summary>
/// GPU用波紋データ構造体
/// </summary>
struct alignas(16) GPURippleData {
	XMFLOAT4 positionAndTime; // xyz:位置、w:時間
	XMFLOAT4 params;          // x:振幅 y:周波数 z:速度 w:使用フラグ
};

/// <summary>
/// 基本波パラメータ構造体
/// </summary>
struct WaveParameters {
	//基本波の周波数
	float frequency1 = 0.02f;
	float frequency2 = 0.015f;
	float frequency3 = 0.01f;

	//基本波の速度
	float speed1 = 2.0f;
	float speed2 = 1.5f;
	float speed3 = 1.0f;

	//波の鋭さ
	float sharpness = WaterConstants::DEFAULT_WAVE_SHARPNESS;
};

/// <summary>
/// 環境マッピング用パラメータ構造体
/// </summary>
struct EnvironmentParameters {
	float reflectionStrength = 0.6f;
	float refractionStrength = 0.4f;
	float fresnelPower = 3.0f;
	float waterClarityDepth = 5.0f;
};

/// <summary>
/// コンピュートシェーダー用定数バッファ構造体
/// </summary>
struct alignas(16) WaveComputeConstants {
	// 基本波パラメータ
	float time;
	float waveHeight;
	float waterSize;
	int activeRippleCount;

	// 基本波パラメータ1
	float baseWaveFreq1;
	float baseWaveFreq2;
	float baseWaveFreq3;
	float baseWaveSpeed1;

	// 基本波パラメータ2
	float baseWaveSpeed2;
	float baseWaveSpeed3;
	float waveSharpness;
	int gridResolution;

	// グリッド情報
	float cellSize;
	float normalDelta;
	float padding1;
	float padding2;

	// 波紋データ配列
	GPURippleData ripples[WaterConstants::MAX_RIPPLES];
};

/// <summary>
/// レンダー用定数バッファ構造体
/// </summary>
struct WaterRenderConstants {
	// 基本波パラメータ
	float time;
	float waveHeight;
	float waterSize;
	int activeRippleCount;

	// 基本波パラメータ1
	float baseWaveFreq1;
	float baseWaveFreq2;
	float baseWaveFreq3;
	float baseWaveSpeed1;

	// 基本波パラメータ2
	float baseWaveSpeed2;
	float baseWaveSpeed3;
	float waveSharpness;
	int padding1;

	// 環境マッピング用パラメータ
	float reflectionStrength;
	float refractionStrength;
	float fresnelPower;
	float waterClarityDepth;

	// 波紋データ配列
	GPURippleData ripples[WaterConstants::MAX_RIPPLES];
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

	// ==== 波パラメータ設定 ====
	void SetWaveParameters(const WaveParameters& params) { m_waveParams = params; }
	const WaveParameters& GetWaveParameters() const { return m_waveParams; }

	// 環境マッピング用パラメータ設定
	void SetEnvironmentMapSRV(ID3D11ShaderResourceView* srv) { m_environmentMapSRV = srv; }
	void SetEnvironmentParameters(const EnvironmentParameters& params) { m_envParams = params; }
	const EnvironmentParameters& GetEnvironmentParameters() const { return m_envParams; }

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
	void ClearRipples();

	// ==== 波高取得（CPU計算） ====
	float GetWaterHeight(const Vector3& position) const;
	Vector3 GetWaterNormal(const Vector3& position) const;

protected:
	virtual bool Initialize() override;
	virtual void Finalize() override;
	virtual void Update(double deltaTime) override;
	virtual void Draw() override;
private:

	// メッシュ生成
	bool CreateMesh();
	bool InitializeComputeShader();
	bool InitializeRenderResources();
	// テクスチャ生成
	bool CreateTextures();

	// コンピュートシェーダー実行
	void UpdateWaveWithComputeShader();
	void UpdateComputeConstants(WaveComputeConstants& constants) const;

	// レンダー定数バッファ更新
	void UpdateRenderConstants();

	// 波紋管理
	void UpdateRipples(float deltaTime);
	void RemoveInactiveRipples();

	// CPU側波計算
	float CalculateWaveHeight(const Vector3& position, float time) const;
	float CalculateBaseWaves(float x, float z, float time) const;
	float CalculateRippleWaves(float x, float z, float time) const;

	//バッファ
	ComPtr<ID3D11Buffer> m_vertexBuffer = nullptr;
	ComPtr<ID3D11Buffer> m_indexBuffer = nullptr;
	ComPtr<ID3D11Buffer> m_computeBuffer = nullptr;
	ComPtr<ID3D11Buffer> m_renderConstantBuffer = nullptr;
	ComPtr<ID3D11Buffer> m_computeConstantBuffer = nullptr;

	//シェーダー
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;
	class ComputeShader* m_computeShader = nullptr;
	ComPtr<ID3D11UnorderedAccessView> m_heightUAV = nullptr;

	//テクスチャ
	class Texture* m_normalMap = nullptr;
	class Texture* m_foamTexture = nullptr;
	ID3D11ShaderResourceView* m_environmentMapSRV = nullptr;

	// サンプラーステート
	ComPtr<ID3D11SamplerState> m_samplerState = nullptr;

	// 水面パラメータ
	float m_waterSize;
	float m_waveHeight;
	float m_time;
	int m_gridResolution;

	WaveParameters m_waveParams;
	EnvironmentParameters m_envParams;

	// 波紋管理
	std::array<Ripple, WaterConstants::MAX_RIPPLES> m_ripples;
	int m_activeRippleCount;

	// メッシュ情報
	UINT m_vertexCount;
	UINT m_indexCount;

	// 初期化フラグ
	bool m_needsInitialCopy = false;
};
