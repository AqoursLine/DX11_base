#pragma once

#include "racingBoat.h"

struct VehicleInput {
	float throttle = 0.0f; //推進入力		(-1.0f ~ 1.0f)
	float steering = 0.0f; //ハンドル入力	(-1.0f ~ 1.0f)
	float brake = 0.0f;    //ブレーキ入力	(0.0f ~ 1.0f)
};

class Texture;
class VertexShader;
class PixelShader;

class Player : public RacingBoat {
public:
	Player() = default;
	~Player() = default;

	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() const override;

private:
	class Model* m_model = nullptr;

	//ボックス表示用
	class Box* m_box = nullptr;
	//shader
	VertexShader* m_boxVS = nullptr;
	PixelShader* m_boxPS = nullptr;
	
	//矢印表示用
	class Field* m_field = nullptr;
	//テクスチャ
	Texture* m_arrowTexture = nullptr;
	// Shader
	VertexShader* m_arrowVS = nullptr;
	PixelShader* m_arrowPS = nullptr;
	

	VehicleInput m_currentInput;
	VehicleInput m_smoothedInput;

	//入力平滑化用パラメータ
	float m_throttlSmoothRate = 5.0f; //推進平滑化レート
	float m_steerSmoothRate = 6.0f; //ステアリング平滑化レート

	//入力更新
	void UpdateInput(double deltaTime);
	//入力平滑化
	void SmoothInput(double deltaTime);
};
