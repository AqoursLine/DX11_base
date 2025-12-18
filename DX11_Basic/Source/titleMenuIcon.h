#pragma once

#include "gameObject.h"

class TitleMenuIcon : public GameObject {
public:
	TitleMenuIcon() = delete;
	TitleMenuIcon(const std::wstring& textureFilePath)
		: m_textureFilePath(textureFilePath) {
	};
	~TitleMenuIcon() = default;

	//選択状態設定
	void IsSelected(bool selected) { m_isSelected = selected; }

	//決定処理
	virtual void OnDecide() = 0;

protected:
	virtual bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() override;

	float m_baseHeight = 128.0f;
private:
	float m_baseScale = 1.0f;
	float m_selectedScale = 1.2f;
	float m_scaleLerpSpeed = 5.0f;
	float m_currentScale = 1.0f;

	Vector3 m_basePosition = Vector3::ZERO;
	Vector3 m_baseScaleVector = Vector3::ONE;

	bool m_isSelected = false;

	class Sprite* m_sprite = nullptr;
	class Texture* m_texture = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;

	std::wstring m_textureFilePath = L"";
};
