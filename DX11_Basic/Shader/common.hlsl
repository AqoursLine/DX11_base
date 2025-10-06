//ワールド行列バッファ
cbuffer WorldMatrixBuffer : register(b0)
{
	matrix WorldMatrix;
};

//ビュー行列バッファ
cbuffer ViewMatrixBuffer : register(b1)
{
	matrix ViewMatrix;
};

//プロジェクション行列バッファ
cbuffer ProjectionMatrixBuffer : register(b2)
{
	matrix ProjectionMatrix;
};

//マテリアル構造体
struct MATERIAL
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float4 Emission;
	float Shininess;
	bool TextureEnable;
	float2 Dummy;
};
//マテリアルバッファ
cbuffer MaterialBuffer : register(b3)
{
	MATERIAL Material;
};

//ライト構造体
struct LIGHT
{
	float4 Direction;
	float4 Diffuse;
	float4 Ambient;
	bool Enable;
	bool3 Dummy;
};
//ライトバッファ
cbuffer LightBuffer : register(b4)
{
	LIGHT Light;
};

//カメラ位置バッファ
cbuffer CameraPositionBuffer : register(b5)
{
	float4 CameraPosition; // カメラの位置
};

//シェーダーパラメータバッファ(汎用)
cbuffer ShaderProperties : register(b6)
{
	float4 params1;
	float4 params2;
};

//頂点シェーダー入力構造体
struct VS_INPUT
{
	float4 Position : POSITION0;
	float4 Normal : NORMAL0;
	float4 Diffuse : COLOR0;
	float2 TexCoord : TEXCOORD0;
};

//ピクセルシェーダー入力構造体
struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : POSITION0;
	float4 Normal : NORMAL0;
	float4 Diffuse : COLOR0;
	float2 TexCoord : TEXCOORD0;
};
