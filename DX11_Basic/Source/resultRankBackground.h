#pragma once

#include "gameObject.h"

#include "raceManager.h"

class ResultRankBackground : public GameObject {
public:
	ResultRankBackground() = default;
	~ResultRankBackground() = default;

	ResultRankBackground* SetResultCount(int count) {
		m_resultCount = count;
		return this;
	}

	ResultRankBackground* SetResultData(const BoatResultData& data) {
		m_resultData = data;
		return this;
	}

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() override;

private:
	class Sprite* m_sprite = nullptr;
	class Texture* m_texture = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;

	int m_resultCount = 0;
	BoatResultData m_resultData;
};

