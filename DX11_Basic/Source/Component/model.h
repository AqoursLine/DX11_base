#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


#ifdef _DEBUG
#pragma comment (lib, "assimp-vc143-mtd.lib")
#else
#pragma comment (lib, "assimp-vc143-mt.lib")
#endif // _DEBUG

//モデル用マテリアル構造体
struct MODEL_MATERIAL {
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
	XMFLOAT4 ambient;
	float shininess;
	ID3D11ShaderResourceView* texture;
	std::string texturePath;

	//マテリアルのハッシュキーを生成
	std::string GenerateHashKey() const {
		char buffer[512];
		snprintf(buffer, sizeof(buffer),
			"D:%.3f%.3f%.3f%.3f S:%.3f%.3f%.3f%.3f A:%.3f%.3f%.3f%.3f Sh:%.2f T:%s",
			diffuse.x, diffuse.y, diffuse.z, diffuse.w,
			specular.x, specular.y, specular.z, specular.w,
			ambient.x, ambient.y, ambient.z, ambient.w,
			shininess, texturePath.c_str());
		return std::string(buffer);
	}

};

//メッシュ構造体
struct MESH {
	const aiScene* scene = nullptr;

	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* indexBuffer = nullptr;

	UINT numIndices;
	MODEL_MATERIAL material;
};

//テクスチャキャッシュエントリ
struct TEXTURE_CACHE_ENTRY {
	ID3D11ShaderResourceView* srv;
	int referenceCount;

	TEXTURE_CACHE_ENTRY() : srv(nullptr), referenceCount(0) {}
	TEXTURE_CACHE_ENTRY(ID3D11ShaderResourceView* srvPtr) : srv(srvPtr), referenceCount(1) {}
};

//マテリアルキャッシュエントリ
struct MATERIAL_CACHE_ENTRY {
	MODEL_MATERIAL material;
	int referenceCount;
	MATERIAL_CACHE_ENTRY() : referenceCount(0) {}
	MATERIAL_CACHE_ENTRY(const MODEL_MATERIAL& mat) : material(mat), referenceCount(1) {}
};

class Model {
public:
	Model() = default;
	~Model() = default;

	bool LoadModelFBX(const std::string& fileName);
	void ReleaseModel();

	void Draw(const Vector3& position, const Vector3& rotation, const Vector3& scale) const;

	static void ClearCache();

private:
	//ノード処理
	void ProcessNode(aiNode* node, const aiScene* scene, const std::string& modelDirectory);
	//メッシュ処理
	void ProcessMesh(aiMesh* mesh, const aiScene* scene, const std::string& modelDirectory);

	//マテリアルロード
	MODEL_MATERIAL LoadMaterial(aiMaterial* aiMat, const aiScene* scene, const std::string& modeDirectory);
	//組み込みテクスチャをロード
	ID3D11ShaderResourceView* LoadEmbeddedTexture(const aiTexture* embeddedTexture, int textureIndex);
	//キャッシュを使用してテクスチャをロード
	ID3D11ShaderResourceView* LoadTexture(const std::string& texturePath);
	//テクスチャロード用関数
	ID3D11ShaderResourceView* LoadTextureFromFile(const std::string& texturePath);


	std::vector<MESH> m_meshes;
	std::string m_directory;

	//テクスチャキャッシュ
	static std::unordered_map<std::string, TEXTURE_CACHE_ENTRY> m_textureCache;

	//テクスチャパスリスト
	std::vector<std::string> m_usedTexturePaths;

	//マテリアルキャッシュ
	static std::unordered_map<std::string, MATERIAL_CACHE_ENTRY> m_materialCache;

	//マテリアルキーリスト
	std::vector<std::string> m_usedMaterialKeys;

	//シェーダー
	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11InputLayout* m_inputLayout = nullptr;
};
