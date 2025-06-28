#pragma once

#include "main.h"

struct HoofPrint {
	XMFLOAT3 position; //蹄跡の位置
	XMFLOAT3 normal; //地面の法線
	float depth; //蹄跡の深さ
	float size; //蹄跡のサイズ
	float angle; //蹄跡の角度
	float timestamp; //蹄跡のタイムスタンプ
	bool isActive; //蹄跡が有効かどうか
};

//地面変形管理クラス
class TerrainDeformation {
public:
	TerrainDeformation();
	~TerrainDeformation();

	//初期化
	bool Initialize(float terrainWidth, float terrainHeight, int heightMapWidth, int heightMapHeight);
	//終了
	void Finalize();

	//蹄跡追加
	void AddHoofPrint(const XMFLOAT3& position, const XMFLOAT3& normal, float force, float size, float angle);

	//更新
	void Update(double deltaTime);

	//地形変形の適用
	void ApplyDeformation();

	//リソース取得
	ID3D11ShaderResourceView* GetHeightSRV() const { return m_heightSRV; }
	ID3D11ShaderResourceView* GetDecalSRV() const { return m_decalSRV; }

	//パラメータ設定
	void SetDeformationParams(float maxDepth, float radius, float recoverySpeed);
	void ResetTerrain();

private:
	//ハイトマップ関連
	ID3D11Texture2D* m_heightTexture;
	ID3D11ShaderResourceView* m_heightSRV;
	ID3D11UnorderedAccessView* m_heightUAV;
	ID3D11Texture2D* m_originalHeightTexture; //元のハイトマップテクスチャ

	//デカール関連
	ID3D11Texture2D* m_decalTexture; //デカールテクスチャ
	ID3D11ShaderResourceView* m_decalSRV; //デカールのSRV
	ID3D11UnorderedAccessView* m_decalUAV; //デカールのUAV

	//コンピュートシェーダー
	ID3D11ComputeShader* m_deformationCS; //地面変形用コンピュートシェーダー
	ID3D11ComputeShader* m_decalCS; //デカール適用用コンピュートシェーダー

	//蹄跡データ
	std::vector<HoofPrint> m_hoofPrints; //蹄跡のリスト
	ID3D11Buffer* m_hoofPrintBuffer; //蹄跡データ用のバッファ
	ID3D11ShaderResourceView* m_hoofPrintSRV; //蹄跡データのSRV

	//定数バッファ
	struct DeformationParams {
		XMFLOAT4 terrainSize; //地形のサイズ
		XMFLOAT4 deformationParams; //変形パラメータ（深さ、サイズ、角度など）
		int numHoofPrints; //蹄跡の数
		float deltaTime; //時間の経過
		XMFLOAT2 padding; //パディング用（16バイト境界に合わせるため）
	};
	ID3D11Buffer* m_deformationParamsBuffer; //定数バッファ

	//地形データ
	float m_terrainWidth; //地形の幅
	float m_terrainHeight; //地形の高さ
	int m_heightMapWidth; //ハイトマップの幅
	int m_heightMapHeight; //ハイトマップの高さ

	//変形パラメータ
	float m_maxDeformationDepth; //最大変形深さ
	float m_deformationRadius; //変形半径
	float m_recoverySpeed; //変形の回復速度

	bool CreateHeightTextures();
	bool CreateDecalTextures();
	bool CreateComputeShaders();
	bool CreateBuffers();
	void UpdateHoofPrintBuffer();
	void CleanupOldHoofPrints(double currentTime);
};
