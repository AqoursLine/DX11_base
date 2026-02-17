#pragma once

#include "gameObject.h"

class StartRaceText : public GameObject {
public:
	StartRaceText() = default;
	~StartRaceText() = default;

	void SetReady(bool ready) { m_isReady = ready; }

protected:
	// ゲーム開始のテキスト表示オブジェクト
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() override;

private:
	class Sprite* m_sprite = nullptr;
	class Texture* m_background = nullptr;
	class Texture* m_textTexture = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;

	Vector3 m_textPosition = {0.0f, 0.0f, 0.0f};
	Vector3 m_textScale = { 1.0f, 1.0f, 1.0f };

	Vector3 m_textStartPos = { 0.0f, 0.0f, 0.0f };
	Vector3 m_textEndPos = { 0.0f, 0.0f, 0.0f };

	Vector3 m_backgroundPosition = { 0.0f, 0.0f, 0.0f };
	Vector3 m_backgroundScale = { 1.0f, 1.0f, 1.0f };

	Vector3 m_backgroundStartPos = { 0.0f, 0.0f, 0.0f };
	Vector3 m_backgroundEndPos = { 0.0f, 0.0f, 0.0f };

	float m_moveDuration = 0.3f; // テキストと背景が移動する時間
	float m_moveElapsed = 0.0f; // 移動の経過時間

	bool m_isReady = false; // レース開始の準備ができているか
};
