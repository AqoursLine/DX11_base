#pragma once

#include "gameObject.h"

// 描画方式の選択
enum class ParticleDrawMode {
	CULL_DISTANCE,	// SV_CullDistanceを使用して描画
	INDIRECT_DRAW,	// 間接描画を使用して描画
};

// GPU側パーティクル構造体
struct GPUParticle {
	XMFLOAT3 position;		// 位置
	XMFLOAT3 velocity;		// 速度
	XMFLOAT4 color;			// 色
	XMFLOAT3 upVector;		// ローカル上方向ベクトル
	float size;				// 大きさ
	float life;				// 寿命
	float maxLife;			// 最大寿命
	float rotation;			// 角度
	float rotationSpeed;	// 回転速度
	UINT active;			// アクティブフラグ
	float padding;			// パディング
};

// パーティクルインスタンスデータ構造体
struct ParticleInstanceData {
	XMFLOAT3 position;	// 位置
	float size;			// 大きさ
	XMFLOAT4 color;		// 色
	XMFLOAT2 texOffset;	// テクスチャオフセット
	float rotation;		// 回転角度
	float padding;		// パディング
};

// ビルボード用頂点データ構造体
struct BillboardVertex {
	XMFLOAT2 offset;	// -1 ~ 1 の範囲で指定
	XMFLOAT2 texCoord;
};

// 更新用パラメータ
struct StaticUpdateParams {
	XMFLOAT3 worldAcceleration; // ワールド加速度
	float startSize;
	XMFLOAT3 localAcceleration; // ローカル加速度
	float endSize;
	XMFLOAT4 startColor;
	XMFLOAT4 endColor;
};
struct DynamicUpdateParams {
	float deltaTime;
	float padding[3]; // パディング
};

// GPUEmit用パラメータ
struct StaticEmitParams {
	float emissionAngle;
	XMFLOAT3 baseVelocity;
	float emissionAngleVariation;
	XMFLOAT3 positionVariation;
	float lifeTime;
	XMFLOAT3 velocityVariation;
	XMFLOAT4 startColor;
	float startSize;
	float rotationSpeed;
	float rotationSpeedMin;
	float rotationSpeedMax;
	UINT maxParticles;
	XMFLOAT3 upVector;
};
struct DynamicEmitParams {
	XMFLOAT3 emitterPosition;
	UINT randomSeed;
	UINT emitCount;
	UINT positionCount;
	XMFLOAT2 padding;
};

// Compact用パラメータ
struct CompactParams {
	UINT maxParticles;
	XMFLOAT3 padding;
};

// エミッター設定
struct EmitterSettings {
	Vector3 position = Vector3::ZERO;		// エミッター位置のばらつき
	Vector3 velocity = Vector3::UP;			// 初速度
	Vector3 velocityVariation = Vector3::ZERO;	// 初速度のばらつき
	Vector4 startColor = Vector4::ONE;		// 開始色
	Vector4 endColor = Vector4::ONE;		// 終了色
	float startSize = 1.0f;					// 開始サイズ
	float endSize = 0.0f;					// 終了サイズ
	float lifeTime = 2.0f;					// 寿命
	float emitRate = 10.0f;					// 発生レート（1秒あたりの発生数）
	Vector3 worldAcceleration = Vector3::ZERO;	// ワールド加速度
	Vector3 localAcceleration = Vector3::ZERO;	// ローカル加速度
	float emissionAngle = 0.0f;				// 発生角度（rad）
	float emissionAngleVariation = 0.0f;	// 発生角度のばらつき（rad）
	float rotationSpeed = 0.0f;				// 回転速度
	float rotationSpeedMin = 0.0f;			// 回転速度最小値
	float rotationSpeedMax = 0.0f;			// 回転速度最大値
	Vector3 upVector = Vector3::UP;			// ローカル上方向ベクトル
	UINT maxParticles = 100;				// 最大パーティクル数
	bool loop = true;						// ループ設定
	bool oneShot = false;					// ワンショット設定
	int oneShotCount = 10;					// ワンショット時の発生数
};

// GPUパーティクルシステムクラス
class GPUParticleSystem : public GameObject {
public:
	GPUParticleSystem(ParticleDrawMode drawMode = ParticleDrawMode::CULL_DISTANCE) 
		: m_drawMode(drawMode) {}
	~GPUParticleSystem() = default;

	/// <summary>
	/// エミッター設定
	/// </summary>
	/// <param name="settings">エミッター設定構造体</param>
	void SetEmitterSettings(const EmitterSettings& settings) {
		m_settings = settings; 
		m_staticEmitParamsDirty = true;
		m_staticUpdateParamsDirty = true;
	}

	/// <summary>
	/// エミッター設定の取得
	/// </summary>
	/// <returns>エミッター設定の参照</returns>
	EmitterSettings& GetEmitterSettings() { return m_settings; }

	/// <summary>
	/// パーティクルテクスチャの設定
	/// </summary>
	/// <param name="textureSRV">テクスチャSRV</param>
	void SetTexture(ID3D11ShaderResourceView* textureSRV) { m_textureSRV = textureSRV; }

	/// <summary>
	/// 描画方式の取得
	/// </summary>
	/// <returns>パーティクル描画方式</returns>
	ParticleDrawMode GetDrawMode() const { return m_drawMode; }

	/// <summary>
	/// パーティクル発生
	/// </summary>
	/// <param name="count">発生数</param>
	void Emit(int count);

	/// <summary>
	/// ワンショット発生
	/// </summary>
	/// <param name="position">発生位置</param>
	/// <param name="count">発生数</param>
	void EmitOneShot(const Vector3& position, int count = -1);

	// 再生・一時停止・停止
	void Pause() { m_isPaused = true; }
	void Resume() { m_isPaused = false; }
	void Stop() { m_isPlaying = false; }
	void Play() { m_isPlaying = true; m_isPaused = false; }

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() override;

	EmitterSettings m_settings;
	ParticleDrawMode m_drawMode;

private:
	// GPUリソース
	ComPtr<ID3D11Buffer> m_vertexBuffer = nullptr;						// ビルボード頂点バッファ
	ComPtr<ID3D11Buffer> m_indexBuffer = nullptr;						// ビルボードインデックスバッファ
	ComPtr<ID3D11Buffer> m_particleBuffer = nullptr;					// パーティクルデータバッファ
	ComPtr<ID3D11UnorderedAccessView> m_particleBufferUAV = nullptr;	// パーティクルデータUAV
	ComPtr<ID3D11ShaderResourceView> m_particleBufferSRV = nullptr;		// パーティクルデータSRV

	// 更新・発生用パラメータバッファ
	ComPtr<ID3D11Buffer> m_dynamicUpdateParamsBuffer = nullptr;			// 動的更新パラメータバッファ
	ComPtr<ID3D11Buffer> m_staticUpdateParamsBuffer = nullptr;			// 静的更新パラメータバッファ
	ComPtr<ID3D11Buffer> m_dynamicEmitParamsBuffer = nullptr;			// 動的発生パラメータバッファ
	ComPtr<ID3D11Buffer> m_staticEmitParamsBuffer = nullptr;			// 静的発生パラメータバッファ

	// フリーインデックス管理
	ComPtr<ID3D11Buffer> m_freeIndicesBuffer = nullptr;					// フリーインデックスバッファ
	ComPtr<ID3D11UnorderedAccessView> m_freeIndicesUAV = nullptr;		// フリーインデックスUAV（消費用）

	// 発生位置管理
	ComPtr<ID3D11Buffer> m_emitPositionsBuffer = nullptr;				// 発生位置バッファ
	ComPtr<ID3D11ShaderResourceView> m_emitPositionsSRV = nullptr;		// 発生位置SRV
	ComPtr<ID3D11Buffer> m_emitPositionIndicesBuffer = nullptr;			// 発生位置インデックスバッファ
	ComPtr<ID3D11ShaderResourceView> m_emitPositionIndicesSRV = nullptr;	// 発生位置インデックスSRV


	// IndirectDraw用リソース
	ComPtr<ID3D11Buffer> m_activeIndicesBuffer = nullptr;				// アクティブインデックスバッファ
	ComPtr<ID3D11UnorderedAccessView> m_activeIndicesUAV = nullptr;		// アクティブインデックスUAV
	ComPtr<ID3D11ShaderResourceView> m_activeIndicesSRV = nullptr;		// アクティブインデックスSRV
	ComPtr<ID3D11Buffer> m_drawArgsBuffer = nullptr;					// 間接描画引数バッファ
	ComPtr<ID3D11UnorderedAccessView> m_drawArgsUAV = nullptr;			// 間接描画引数UAV
	ComPtr<ID3D11Buffer> m_compactParamsBuffer = nullptr;				// Compact用パラメータバッファ

	// シェーダー
	class VertexShader* m_vertexShader = nullptr;				// 頂点シェーダ
	class PixelShader* m_pixelShader = nullptr;					// ピクセルシェーダ
	class ComputeShader* m_updateComputeShader = nullptr;		// 更新用コンピュートシェーダ
	class ComputeShader* m_emitComputeShader = nullptr;			// 発生用コンピュートシェーダ
	class ComputeShader* m_compactComputeShader = nullptr;		// Compact用コンピュートシェーダ(IndirectDraw用)

	ID3D11ShaderResourceView* m_textureSRV = nullptr;			// パーティクルテクスチャSRV

	// ブレンドステート
	ComPtr<ID3D11BlendState> m_blendState = nullptr;

	// 発生タイマー
	float m_emitTimer = 0.0f;
	bool m_isPlaying = true;
	bool m_isPaused = false;
	UINT m_randomSeed = 0;

	// フリーリストカウンタ初期化フラグ
	bool m_freeListInitialized = false;

	// 静的パラメータ変更フラグ
	bool m_staticUpdateParamsDirty = true;
	bool m_staticEmitParamsDirty = true;

	// バッチEmit用構造体
	struct EmitRequest {
		XMFLOAT3 position;
		UINT Count;
	};
	std::vector<EmitRequest> m_pendingEmits;

	// GPU側でのパーティクル更新
	void UpdateParticlesGPU(float deltaTime);

	// GPU側でのパーティクル発生
	void EmitGPU(UINT count, const std::vector<XMFLOAT4>& positions);

	// バッチEmitリクエストを処理
	void FlushPendingEmits();

	// GPU側でのアクティブパーティクル収集(IndirectDraw用)
	void CompactParticlesGPU();

	// バッファ作成
	bool CreateBuffers();

	// シェーダ読み込み
	bool LoadShaders();

	// ブレンドステート作成
	bool CreateState();

};
