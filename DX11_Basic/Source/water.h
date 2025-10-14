#pragma once

#include "gameObject.h"

//波紋構造体
struct Ripple {
	Vector3 position;	//波紋の中心位置
	float time;			//波紋の経過時間
	float amplitude;	//波紋の振幅
	float frequency;	//波紋の周波数
	float speed;		//波紋の広がる速度
	bool active;		//波紋が有効かどうか

	Ripple()
		: position(0.0f, 0.0f, 0.0f)
		, time(0.0f)
		, amplitude(1.0f)
		, frequency(1.0f)
		, speed(1.0f)
		, active(false)
	{
	}
};

class Water : public GameObject {
public:
	Water();
	virtual ~Water();

	//波紋を追加
	void AddRipple(const Vector3& position, float amplitude = 1.0f, float frequency = 2.0f, float speed = 5.0f);

	float GetWaterHeight(const Vector3& position) const;
	Vector3 GetWaterNormal(const Vector3& position) const;

	//設定
	void SetWaveHeight(float height) { m_waveHeight = height; }
	float GetWaveHeight() const { return m_waveHeight; }

	void SetWaterSize(float size) { m_waterSize = size; }
	float GetWaterSize() const { return m_waterSize; }

	//波の基本パラメータ設定
	void SetBaseWaveFrequency1(float freq) { m_baseWaveFrequency1 = freq; }
	void SetBaseWaveFrequency2(float freq) { m_baseWaveFrequency2 = freq; }
	void SetBaseWaveFrequency3(float freq) { m_baseWaveFrequency3 = freq; }
	void SetBaseWaveSpeed1(float speed) { m_baseWaveSpeed1 = speed; }
	void SetBaseWaveSpeed2(float speed) { m_baseWaveSpeed2 = speed; }
	void SetBaseWaveSpeed3(float speed) { m_baseWaveSpeed3 = speed; }
	//まとめて設定
	void SetBaseWaveParameters(float freq1, float freq2, float freq3, float speed1, float speed2, float speed3) {
		m_baseWaveFrequency1 = freq1;
		m_baseWaveFrequency2 = freq2;
		m_baseWaveFrequency3 = freq3;
		m_baseWaveSpeed1 = speed1;
		m_baseWaveSpeed2 = speed2;
		m_baseWaveSpeed3 = speed3;
	}

	//プリセット
	void SetWavePresetCalm() {
		SetBaseWaveParameters(0.02f, 0.015f, 0.01f, 1.0f, 0.8f, 0.5f);
		SetWaveHeight(2.0f);
	}
	void SetWavePresetWindy() {
		SetBaseWaveParameters(0.04f, 0.03f, 0.02f, 2.0f, 1.5f, 1.0f);
		SetWaveHeight(5.0f);
	}
	void SetWavePresetRough() {
		SetBaseWaveParameters(0.06f, 0.045f, 0.03f, 3.0f, 2.5f, 2.0f);
		SetWaveHeight(8.0f);
	}

protected:
	virtual bool Initialize() override;
	virtual void Finalize() override;
	virtual void Update(double deltaTime) override;
	virtual void Draw() const override;

private:
	//メッシュ生成
	bool CreateWaterMesh();
	bool CreateShaders();
	void UpdateConstantBuffer();

	//水面の高さ計算(CPU版)
	float CalculateWaveHeight(const Vector3& position, float time) const;
	//水面の法線計算(CPU版)
	Vector3 CalculateWaveNormal(const Vector3& position, float time) const;

	//DirectX 11リソース
	ID3D11Buffer* m_vertexBuffer = nullptr;
	ID3D11Buffer* m_indexBuffer = nullptr;
	ID3D11Buffer* m_constantBuffer = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;

	//水のパラメータ
	float m_waterSize;		//水面のサイズ
	float m_waveHeight;		//波の高さスケール
	float m_time;			//経過時間
	int m_gridResolution;	//グリッドの解像度
	
	//基本波のパラメータ
	float m_baseWaveFrequency1;	//基本波1の周波数
	float m_baseWaveFrequency2;	//基本波2の周波数
	float m_baseWaveFrequency3;	//基本波3の周波数
	float m_baseWaveSpeed1;		//基本波1の速度
	float m_baseWaveSpeed2;		//基本波2の速度
	float m_baseWaveSpeed3;		//基本波3の速度

	//波紋管理
	std::vector<Ripple> m_ripples;
	static const int MAX_RIPPLES = 10;
	int m_rippleIndex;	//次に使用する波紋インデックス

	//定数バッファ用構造体
	struct WaterConstantBuffer {
		float time;
		float waveHeight;
		float waterSize;
		float padding; // 16バイトアラインメント用

		//基本波パラメータ
		float baseWaveFreq1;
		float baseWaveFreq2;
		float baseWaveFreq3;
		float baseWaveSpeed1;
		float baseWaveSpeed2;
		float baseWaveSpeed3;
		float padding2[2]; // 16バイトアラインメント用

		//波紋データ
		struct {
			XMFLOAT4 positionAndTime;	// xyz:位置, w:時間
			XMFLOAT4 params;			// x:振幅, y:周波数, z:速度, w:使用フラグ
		} ripples[MAX_RIPPLES];
	};

};

