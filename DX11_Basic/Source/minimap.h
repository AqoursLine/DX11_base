#pragma once

#include "gameObject.h"

class Texture;
class RacingBoat;

class MiniMap : public GameObject {
public:
	MiniMap() = default;
	~MiniMap() = default;

protected:
	virtual bool Initialize() override;
	virtual void Finalize() override;
	virtual void Update(double deltaTime) override;
	virtual void Draw() override;

private:
	class Sprite* m_sprite = nullptr;
	Texture* m_minimapTexture = nullptr;
	Texture* m_playerIconTexture = nullptr;
	Texture* m_buoyIconTexture = nullptr;
	class VertexShader* m_minimapVS = nullptr;
	class PixelShader* m_minimapPS = nullptr;

	std::vector<RacingBoat*> m_racingBoats;

	float m_mapConversion = 0.5f; //ワールド座標からミニマップ座標への変換率
	Vector3 m_iconScale = { 20.0f, 20.0f, 1.0f };
};
