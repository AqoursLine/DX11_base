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
	static ComPtr<ID3D11Buffer> m_vertexBuffer;
	//インデックスバッファ
	static ComPtr<ID3D11Buffer> m_indexBuffer;
	//インデックス数
	static UINT m_numIndices;

	//参照カウント
	static int m_refCount;
};
