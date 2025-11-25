#pragma once

#include "gameObject.h"
#include "raceManager.h"

class ResultTime : public GameObject {
public:
	ResultTime(int resultCount, const BoatResultData& resultData, int index);
	~ResultTime() = default;

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() override;

private:
	class Sprite* m_sprite = nullptr;
	class Texture* m_texture = nullptr;
	class Texture* m_numberTexture = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;

	int m_resultCount = 0;
	int m_index = 0;
	BoatResultData m_resultData;
};
