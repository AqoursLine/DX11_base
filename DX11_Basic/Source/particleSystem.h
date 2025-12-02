#pragma once

#include "gameObject.h"

// パーティクル構造体
struct Particle {
	Vector3 position;
	Vector3 velocity;
	Vector4 color;
	float size;
	float life;
	float maxLife;
	float rotation;
	float rotationSpeed;
	bool active;
};

// パーティクルインスタンスデータ
struct ParticleInstance {
	XMFLOAT3 position;
	float size;
	XMFLOAT4 color;
	XMFLOAT2 texOffset;
	float rotation;
	float padding;
};

// ビルボード用要点データ
struct BillboardVertex {
	XMFLOAT2 offset;	// -1 ~ 1 の範囲で指定
	XMFLOAT2 texCoord;
};

// エミッター設定
struct EmitterSettings {
	Vector3 position = Vector3::ZERO;		// エミッター位置のばらつき
	Vector3 velocity = Vector3::UP;			// 初速度
	Vector3 velocityVariation = Vector3(0.5f, 0.5f, 0.5f);	// 初速度のばらつき
	Vector4 startColor = Vector4::ONE;		// 開始色
	Vector4 endColor = Vector4::ONE;		// 終了色
	float startSize = 1.0f;					// 開始サイズ
	float endSize = 0.0f;					// 終了サイズ
	float lifeTime = 2.0f;					// 寿命
	float emitRate = 10.0f;					// 発生レート（1秒あたりの発生数）
	float gravity = -9.81f;					// 重力
	float emissionAngle = 0.0f;				// 発生角度（rad）
	float emissionAngleVariation = 0.0f;	// 発生角度のばらつき（rad）
	float rotationSpeed = 0.0f;				// 回転速度
	float rotationSpeedMin = 0.0f;			// 回転速度最小値
	float rotationSpeedMax = 0.0f;			// 回転速度最大値
	int maxParticles = 100;					// 最大パーティクル数
	bool loop = true;						// ループ設定
	bool oneShot = false;					// ワンショット設定
	int oneShotCount = 10;					// ワンショット時の発生数
};

class ParticleSystem : public GameObject {
public:
	ParticleSystem() = default;
	~ParticleSystem() = default;

	// エミッター設定の取得と設定
	void SetEmitterSettings(const EmitterSettings& settings) { m_settings = settings; }
	EmitterSettings& GetEmitterSettings() { return m_settings; }

	// テクスチャ設定
	void SetTexture(ID3D11ShaderResourceView* textureSRV) { m_textureSRV = textureSRV; }

	// パーティクル発生
	void Emit(int count = 1);

	// ワンショット発生
	void EmitOneShot(const Vector3& position, int count = -1);

	// 一時停止・再開
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

private:
	// パーティクル管理
	std::vector<Particle> m_particles;

	// GPUリソース
	ID3D11Buffer* m_vertexBuffer = nullptr;
	ID3D11Buffer* m_indexBuffer = nullptr;
	ID3D11Buffer* m_structuredBuffer = nullptr;
	ID3D11ShaderResourceView* m_structuredBufferSRV = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;
	ID3D11ShaderResourceView* m_textureSRV = nullptr;

	// ブレンドステート
	ID3D11BlendState* m_blendState = nullptr;

	// 発生タイマー
	float m_emitTimer = 0.0f;
	bool m_isPlaying = true;
	bool m_isPaused = false;

	// パーティクル更新
	void UpdateParticles(float deltaTime);

	// バッファ作成
	bool CreateBuffers();

	// シェーダー作成
	bool CreateShaders();

	// ステート作成
	bool CreateStates();

};
