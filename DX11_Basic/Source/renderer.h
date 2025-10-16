#pragma once
#include <vector>

//頂点構造体
struct VERTEX_3D {
	XMFLOAT4 position;
	XMFLOAT4 normal;
	XMFLOAT4 texcoord;
	XMFLOAT4 tangent;
	XMFLOAT4 diffuse;
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
	XMFLOAT4 positionAndType; // w成分にライトタイプ(0:平衡,1:点,2:スポット)
	XMFLOAT4 directionAndIntensity; // w成分に光の強さ
	XMFLOAT4 diffuseAndRange; // w成分に光の届く範囲

	// スポットライト用パラメータ
	XMFLOAT4 spotParams; // x:innerCone, y:outerCone, z:falloff, w:enabled
	XMFLOAT4 attenuation; // x:定数, y:線形, z:二次, w:未使用
};

constexpr int MAX_LIGHTS = 16; // 最大ライト数

//ライト配列構造体
struct LIGHTS {
	LIGHT lights[MAX_LIGHTS];
};

///シェーダープロパティ構造体
struct SHADER_PROPERTIES {
	Vector4 params1; // 汎用パラメータ1
	Vector4 params2; // 汎用パラメータ2
	Vector4 params3; // 汎用パラメータ3
};

//深度モード
enum class DEPTH_MODE {
	ENABLE, // 深度有効
	READ_ONLY, // 深度読み取り専用
	DISABLE // 深度無効
};

class Renderer {
private:
	Renderer() = default;

	static Renderer* s_instance;

public:
	/// シングルトンパターン
	Renderer(const Renderer&) = delete; // コピーコンストラクタを削除
	Renderer& operator=(const Renderer&) = delete; // コピー代入演算子を削除
	Renderer(Renderer&&) = delete; // ムーブコンストラクタを削除
	Renderer& operator=(Renderer&&) = delete; // ムーブ代入演算子を削除

	static void CreateInstance() {
		if (s_instance == nullptr) {
			s_instance = new Renderer();
		}
	}
	static void DestroyInstance() {
		if (s_instance != nullptr) {
			s_instance->Finalize(); // 既存のインスタンスを終了
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
	void SetDepthStencilState(DEPTH_MODE mode);
	//ブレンドステート設定
	void SetATCEnable(bool enable);
	//デフォルトサンプラーステート設定
	void SetSamplerState();

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
	void SetLight(const LIGHT& light, int lightIndex);
	//カメラ位置設定
	void SetCameraPosition(const Vector3& position);
	//シェーダープロパティ設定
	void SetShaderProperties(const SHADER_PROPERTIES& properties);

	//デバイス取得
	ID3D11Device* GetDevice() { return m_device.Get(); }
	//デバイスコンテキスト取得
	ID3D11DeviceContext* GetDeviceContext() { return m_deviceContext.Get(); }

	//頂点シェーダー作成
	void CreateVertexShader(ID3D11VertexShader** vertexShader, ID3D11InputLayout** inputLayout, std::wstring fileName);
	//ピクセルシェーダー作成
	void CreatePixelShader(ID3D11PixelShader** pixelShader, std::wstring fileName);

	//レンダーターゲット追加
	int AddRenderTarget(UINT width, UINT height);

	//レンダーターゲット設定
	void SetRenderTarget(int index);
	//デフォルトレンダーターゲット設定
	void SetDefaultRenderTarget();
	//srv取得
	ID3D11ShaderResourceView* GetRenderTargetSRV(int index);

private:
	ComPtr<IDXGISwapChain> m_swapChain;
	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_deviceContext;
	D3D_FEATURE_LEVEL m_featureLevel = D3D_FEATURE_LEVEL_11_0;

	ComPtr<ID3D11RenderTargetView> m_renderTargetView;
	ComPtr<ID3D11DepthStencilView> m_depthStencilView;


	//ブレンドステート
	ComPtr<ID3D11BlendState> m_blendState;
	ComPtr<ID3D11BlendState> m_blendStateATC;

	//深度ステンシルステート
	ComPtr<ID3D11DepthStencilState> m_depthStencilStateEnable;
	ComPtr<ID3D11DepthStencilState> m_depthStencilStateReadOnly;
	ComPtr<ID3D11DepthStencilState> m_depthStencilStateDisable;

	//サンプラーステート
	ComPtr<ID3D11SamplerState> m_samplerState;

	//コンスタントバッファ
	ComPtr<ID3D11Buffer> m_worldBuffer;
	ComPtr<ID3D11Buffer> m_viewBuffer;
	ComPtr<ID3D11Buffer> m_projectionBuffer;
	LIGHTS m_lightsData; // ライト配列データ
	ComPtr<ID3D11Buffer> m_lightBuffer;
	ComPtr<ID3D11Buffer> m_materialBuffer;
	ComPtr<ID3D11Buffer> m_cameraBuffer;
	ComPtr<ID3D11Buffer> m_shaderPropertiesBuffer;

	//レンダーターゲット
	std::vector<ComPtr<ID3D11ShaderResourceView>> m_renderTargetSRV;
	std::vector<ComPtr<ID3D11RenderTargetView>> m_renderTargetRTV;

};

//シングルトンインスタンス取得マクロ
#define RENDERER Renderer::GetInstance()
