#pragma once

#include "gameObject.h"

#include "multiButton.h"

class MultiSelector : public GameObject {
public:
	MultiSelector() = default;
	~MultiSelector() = default;

	void AddButton(MultiButton* button) {
		m_buttons.push_back(button);
	}

	void ClearButtons() {
		m_buttons.clear();
		m_selectedIndex = 0;
	}

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() override;

private:
	class Sprite* m_sprite;
	class Texture* m_texture;
	class VertexShader* m_vertexShader;
	class PixelShader* m_pixelShader;

	std::vector<MultiButton*> m_buttons;

	int m_selectedIndex = 0;

};
