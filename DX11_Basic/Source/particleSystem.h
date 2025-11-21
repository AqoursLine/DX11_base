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
	bool active;
};

// パーティクルインスタンスデータ
struct ParticleInstance {
	XMFLOAT3 position;
	float size;
	XMFLOAT4 color;
	XMFLOAT2 texOffset;
	float padding[2]; // アライメント調整用
};

// ビルボード用要点データ
struct BillboardVertex {
	XMFLOAT2 offset;	// -1 ~ 1 の範囲で指定
	XMFLOAT2 texCoord;
};

// エミッター設定
struct EmitterSettings {
	Vector3 position = Vector3::ZERO;
	Vector3 velocity = Vector3::UP;
	Vector3 velocityVariation = Vector3(0.5f, 0.5f, 0.5f);
	Vector4 startColor = Vector4::ONE;
	Vector4 endColor = Vector4::ONE;
	float startSize = 1.0f;
	float endSize = 0.0f;
	float lifeTime = 2.0f;
	float emitRate = 10.0f;
	float gravity = -9.81f;
	int maxParticles = 100;
	bool loop = true;
	bool oneShot = false;
	int oneShotCount = 10;
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
	ID3D11Buffer* m_instanceBuffer = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;
	ID3D11ShaderResourceView* m_textureSRV = nullptr;

	// ブレンドステート
	ID3D11BlendState* m_blendState = nullptr;

	// 発生タイマー
	float m_enmitTimer = 0.0f;
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
