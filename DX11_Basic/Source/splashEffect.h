#pragma once

#include "gameObject.h"

struct SplashParticle {
	Vector3 position;
	Vector3 velocity;
	Vector3 rotation;
	Vector3 angularVelocity;
	float size;
	float life;
	float maxLife;
	bool active;
};

class SplashEffect : public GameObject {
public:
	SplashEffect();
	virtual ~SplashEffect() = default;

	// パーティクル発生
	void EmitSplash(const Vector3& position, const Vector3& direction, float intensity = 1.0f);
	void EmitContinuousSplash(const Vector3& position, const Vector3& direction, float intensity = 1.0f);
	void EmitWaveSplash(const Vector3& position, float waveHeight);

	// パーティクルパラメータ設定
	void SetMaxParticles(int maxParticles);
	void SetParticleLifeTime(float minLifeTime, float maxLifeTime);
	void SetParticleSize(float minSize, float maxSize);
	void SetParticleSpeed(float minSpeed, float maxSpeed);
	void SetGravity(float gravity) { m_gravity = gravity; }

	// 情報取得
	int GetActiveParticleCount() const;

protected:
	virtual bool Initialize() override;
	virtual void Finalize() override;
	virtual void Update(double deltaTime) override;
	virtual void Draw() const override;

private:
	struct ParticleVertex {
		XMFLOAT4 position;
		XMFLOAT4 color;
		XMFLOAT4 rotation;
	};

	// パーティクルデータ
	std::vector<SplashParticle> m_particles;
	int m_maxParticles;

	// 頂点バッファ
	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_instanceBuffer;

	// シェーダー
	class VertexShader* m_vertexShader;
	class PixelShader* m_pixelShader;

	// テクスチャ
	class Texture* m_texture;

	// ブレンドステート
	ID3D11BlendState* m_blendState;

	// パーティクルパラメータ
	float m_minLifeTime;
	float m_maxLifeTime;
	float m_minSize;
	float m_maxSize;
	float m_minSpeed;
	float m_maxSpeed;
	float m_gravity;

	// 内部メソッド
	void UpdateInstanceBuffer();
	void EmitParticle(const Vector3& position, const Vector3& velocity, float size, float lifeTime);

};
