#pragma once

#include "gameObject.h"

class RankDisplay : public GameObject {
public:
	RankDisplay() = default;
	~RankDisplay() = default;

protected:
	virtual bool Initialize() override;
	virtual void Finalize() override;
	virtual void Update(double deltaTime) override;
	virtual void Draw() override;

private:
	class Texture* m_texture = nullptr;
	class Sprite* m_sprite = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;

	class Player* m_player = nullptr;
};
