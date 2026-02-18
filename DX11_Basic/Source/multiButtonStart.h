#pragma once

#include "multiButton.h"

class MultiButtonStart : public MultiButton {
public:
	MultiButtonStart() = default;
	~MultiButtonStart() = default;

	void SetReady(bool isReady) { m_isReady = isReady; }

	void OnDecide() override;

protected:
	bool Initialize() override;
	void Finalize() override;
	void Draw() override;

private:
	class Sprite* m_sprite = nullptr;
	class Texture* m_texture = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;

	bool m_isReady = false;
	bool m_isDecided = false;
};

