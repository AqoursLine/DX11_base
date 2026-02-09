#pragma once

#include "gameObject.h"
#include "raceManager.h"

class ResultTime : public GameObject {
public:
	ResultTime() = default;
	~ResultTime() = default;

	ResultTime* SetResultCount(int count) {
		m_resultCount = count;
		return this;
	}

	ResultTime* SetResultData(const BoatResultData& data) {
		m_resultData = data;
		return this;
	}

	ResultTime* SetIndex(int index) {
		m_index = index;
		return this;
	}

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() override;

private:
	class Sprite* m_sprite = nullptr;
	class Texture* m_numberTexture = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;

	int m_resultCount = 0;
	int m_index = 0;
	BoatResultData m_resultData;
};
