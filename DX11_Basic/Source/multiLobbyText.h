#pragma once

#include "gameObject.h"

class MultiLobbyText : public GameObject {
public:
	MultiLobbyText() = default;
	~MultiLobbyText() = default;

protected:
	bool Initialize() override;
	void Finalize() override;
	void Draw() override;

private:
	class Sprite* m_sprite = nullptr;
	class Texture* m_texture = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;

};

