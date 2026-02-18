#pragma once

#include "multiButton.h"

class MultiButtonReady : public MultiButton {
public:
	MultiButtonReady() = default;
	~MultiButtonReady() = default;

	void OnDecide() override;

protected:
	bool Initialize() override;
	void Finalize() override;
	void Draw() override;

private:
	class Sprite* m_sprite = nullptr;
	class Texture* m_readyTexture = nullptr;
	class Texture* m_cancelTexture = nullptr;

	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;

	bool m_isReady = false;
};

