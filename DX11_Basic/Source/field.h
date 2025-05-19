#pragma once

class Field {
public:
	Field() = default;
	~Field() = default;

	bool Initialize(std::wstring fileName);
	void Finalize();
	void Draw(const Vector3& pos, const Vector3& rot, const Vector3& scale) const;

private:
	//頂点バッファ
	ComPtr<ID3D11Buffer> m_vertexBuffer;

	//頂点シェーダー
	ComPtr<ID3D11VertexShader> m_vertexShader;
	//ピクセルシェーダー
	ComPtr<ID3D11PixelShader> m_pixelShader;
	//入力レイアウト
	ComPtr<ID3D11InputLayout> m_inputLayout;

	//テクスチャ
	ComPtr<ID3D11ShaderResourceView> m_texture;
};
