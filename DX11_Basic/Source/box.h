#pragma once

class Box {
public:
	Box() = default;
	~Box() = default;

	bool Initialize();
	void Finalize();
	void Draw(const Vector3& pos, const Vector3& rot, const Vector3& scale) const;
	void Draw(const Vector3& pos, const Vector4& rot, const Vector3& scale) const;

private:
	//頂点バッファ
	ComPtr<ID3D11Buffer> m_vertexBuffer;
	//インデックスバッファ
	ComPtr<ID3D11Buffer> m_indexBuffer;
	//インデックス数
	UINT m_numIndices = 0;


	//頂点シェーダー
	ComPtr<ID3D11VertexShader> m_vertexShader;
	//ピクセルシェーダー
	ComPtr<ID3D11PixelShader> m_pixelShader;
	//入力レイアウト
	ComPtr<ID3D11InputLayout> m_inputLayout;
};
