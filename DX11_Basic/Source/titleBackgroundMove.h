#pragma once

#include "gameObject.h"

class TitleBackgroundMove : public GameObject {
public:
	TitleBackgroundMove() = default;
	~TitleBackgroundMove() = default;

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() override;
private:
	struct VideoTexture* m_videoTexture = nullptr;

	class Sprite* m_sprite = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;
};

