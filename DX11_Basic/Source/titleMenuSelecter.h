#pragma once

#include "gameObject.h"

class TitleMenuIcon;

class TitleMenuSelecter : public GameObject {
public:
	TitleMenuSelecter() = default;
	~TitleMenuSelecter() = default;

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() override;

private:
	std::vector<TitleMenuIcon*> m_menuIcons;

	int m_currentIndex = 0;
	int m_previousIndex = 0;
	int m_maxIndex = 0;


	class Texture* m_texture = nullptr;
	class Sprite* m_sprite = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;
};
