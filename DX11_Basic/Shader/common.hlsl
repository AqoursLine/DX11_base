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
	float4 PositionAndType; // w成分はライトの種類(0:平行光源、1:点光源、2:スポットライト)
	float4 DirectionAndIntensity; // w成分は光の強さ
	float4 DiffuseAndRange; // w成分は光の届く距離(点光源、スポットライト用)
	
	//スポットライト用
	float4 spotParams; //x:innerCone, y:outerCone, z:falloff, w:enabled(0:disable, 1:enable)
	float4 attenuation; //x:定数, y:線形, z:二次, w:ダミー
};

#define MAX_LIGHTS 16

//ライトバッファ
cbuffer LightBuffer : register(b4)
{
	LIGHT Lights[MAX_LIGHTS];
};

//カメラ位置バッファ
cbuffer CameraBuffer : register(b5)
{
	float4 CameraPosition; // カメラの位置
	float4 CameraRight; // カメラの右方向ベクトル
	float4 CameraUp; // カメラの上方向ベクトル
};

//シェーダーパラメータバッファ(汎用)
cbuffer ShaderProperties : register(b6)
{
	float4 params1;
	float4 params2;
	float4 params3;
};

//頂点シェーダー入力構造体
struct VS_INPUT
{
	float4 Position : POSITION0;
	float4 Normal : NORMAL0;
	float4 TexCoord : TEXCOORD0;
	float4 Tangent : TANGENT0;
	float4 Diffuse : COLOR0;
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
