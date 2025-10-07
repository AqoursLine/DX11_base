#pragma once

class Sprite {
public:
	Sprite() = default;
	~Sprite() = default;

	bool Initialize();
	void Finalize();
	void Draw(const Vector3& pos, const Vector3& rot, const Vector3& scale) const;

private:
	//頂点バッファ
	static ComPtr<ID3D11Buffer> m_vertexBuffer;

	//参照カウント
	static int m_refCount;

};
