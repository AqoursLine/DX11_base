#pragma once

//頂点構造体
struct VERTEX_3D {
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT4 diffuse;
	XMFLOAT2 texcoord;
};

//マテリアル構造体
struct MATERIAL {
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
	XMFLOAT4 emission;
	float shininess;
	BOOL textureEnable;
	float dummy[2];
};

//ライト構造体
struct LIGHT {
	XMFLOAT4 direction;
	XMFLOAT4 diffuse;
	XMFLOAT4 ambient;
	BOOL enable;
	float dummy[3];
};

class Renderer {
private:
	Renderer() = default;

	static Renderer* s_instance;

public:
	/// シングルトンパターン
	static void CreateInstance() {
		DestroyInstance();

		s_instance = new Renderer();
	}
	static void DestroyInstance() {
		if (s_instance != nullptr) {
			delete s_instance;
			s_instance = nullptr;
		}
	}
	static Renderer& GetInstance() {
		return *s_instance;
	}

	//初期化
	bool Initialize(HWND hWnd);
	//終了
	void Finalize();
	//描画開始
	void BeginDraw();
	//描画終了
	void EndDraw();

	//深度バッファ設定
	void SetDepthStencilState(bool enable);
	//ブレンドステート設定
	void SetATCEnable(bool enable);

	//2D用行列設定
	void Set2DMatrix();
	//ワールド行列設定
	void SetWorldMatrix(const XMMATRIX& worldMatrix);
	//ビュー行列設定
	void SetViewMatrix(const XMMATRIX& viewMatrix);
	//プロジェクション行列設定
	void SetProjectionMatrix(const XMMATRIX& projectionMatrix);

	//マテリアル設定
	void SetMaterial(const MATERIAL& material);
	//ライト設定
	void SetLight(const LIGHT& light);

	//デバイス取得
	ID3D11Device* GetDevice() { return m_device.Get(); }
	//デバイスコンテキスト取得
	ID3D11DeviceContext* GetDeviceContext() { return m_deviceContext.Get(); }

	//頂点シェーダー作成
	void CreateVertexShader(ID3D11VertexShader** vertexShader, ID3D11InputLayout** inputLayout, std::wstring fileName);
	//ピクセルシェーダー作成
	void CreatePixelShader(ID3D11PixelShader** pixelShader, std::wstring fileName);

private:
	ComPtr<IDXGISwapChain> m_swapChain;
	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_deviceContext;
	D3D_FEATURE_LEVEL m_featureLevel;

	ComPtr<ID3D11RenderTargetView> m_renderTargetView;
	ComPtr<ID3D11DepthStencilView> m_depthStencilView;

	ComPtr<ID3D11BlendState> m_blendState;
	ComPtr<ID3D11BlendState> m_blendStateATC;

	ComPtr<ID3D11DepthStencilState> m_depthStencilStateEnable;
	ComPtr<ID3D11DepthStencilState> m_depthStencilStateDisable;

	ComPtr<ID3D11Buffer> m_worldBuffer;
	ComPtr<ID3D11Buffer> m_viewBuffer;
	ComPtr<ID3D11Buffer> m_projectionBuffer;
	ComPtr<ID3D11Buffer> m_lightBuffer;
	ComPtr<ID3D11Buffer> m_materialBuffer;

};

//シングルトンインスタンス取得マクロ
#define RENDERER Renderer::GetInstance()
