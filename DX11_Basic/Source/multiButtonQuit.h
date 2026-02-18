#pragma once

#include "multiButton.h"

class MultiButtonQuit : public MultiButton {
public:
	MultiButtonQuit() = default;
	~MultiButtonQuit() = default;

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

	bool m_isDecided = false;
};

