#pragma once

//前ぽ宣言
struct aiScene;
struct aiNode;
struct aiMesh;
struct aiMaterial;

//========================================================
// モデル頂点データ構造体
//========================================================
struct MODEL_VERTEX {
	Vector4 position;	//xyz座標
	Vector4 normal;		//法線ベクトル
	Vector4 texcoord;	//テクスチャ座標
	Vector4 tangent;	//接線ベクトル
	Vector4 color;		//頂点カラー
};

//========================================================
// モデルマテリアル用テクスチャ構造体
//========================================================
struct MODEL_MATERIAL_TEXTURE {
	int textureIndex = -1;		//テクスチャインデックス(-1はテクスチャ無し)
	int TextureType = 0;		//テクスチャタイプ(aiTextureType)
	unsigned int uvChannel = 0;	//UVチャンネル
};

//========================================================
// モデルマテリアル構造体
//========================================================
struct MODEL_MATERIAL {
	std::string name; //マテリアル名

	//基本色
	Vector4 diffuseColor;	//拡散反射色
	Vector4 specularColor;	//鏡面反射色
	Vector4 ambientColor;	//環境光色
	Vector4 emissiveColor;	//自己発光色

	//物理パラメータ
	float shininess = 0.0f;	//光沢度(Phongの指数)
	float opacity = 1.0f;	//不透明度(1.0で不透明)
	float metallic = 0.0f;	//金属性(0.0~1.0)
	float roughness = 1.0f;	//粗さ(0.0~1.0)

	//テクスチャ
	std::vector<MODEL_MATERIAL_TEXTURE> textures;

	//フラグ
	bool isTwoSided = false;	//両面描画するか
	int shadingModel = 0;		//シェーディングモデル(aiShadingMode)
};

//========================================================
// モデルメッシュ構造体
//========================================================
struct MODEL_MESH {
	std::vector<MODEL_VERTEX> vertices;	//頂点データ配列
	std::vector<unsigned int> indices;	//インデックスデータ配列
	unsigned int materialIndex;			//マテリアルインデックス

	//DirectXリソース
	ComPtr<ID3D11Buffer> vertexBuffer;	//頂点バッファ
	ComPtr<ID3D11Buffer> indexBuffer;	//インデックスバッファ
	unsigned int indexCount = 0;		//インデックス数

};

//========================================================
// モデルノード構造体
//========================================================
struct MODEL_NODE {
	std::string name;			//ノード名
	XMMATRIX localTransform;	//ローカルトランスフォーム
	std::vector<unsigned int> meshIndices;	//メッシュインデックス配列

	MODEL_NODE* parent = nullptr;		//親ノード
	std::vector<std::unique_ptr<MODEL_NODE>> children;	//子ノード配列

	//ワールド行列計算
	XMMATRIX GetWorldTransform() const;

};

//========================================================
// モデル構造体
//========================================================
struct MODEL {
	std::vector<MODEL_MESH> meshes;							//メッシュ配列
	std::vector<MODEL_MATERIAL> materials;					//マテリアル配列
	std::vector<ComPtr<ID3D11ShaderResourceView>> textures;	//テクスチャ配列
	std::unordered_map<std::string, int> textureMap;		//テクスチャ名からテクスチャインデックスを検索するためのマップ
	std::unique_ptr<MODEL_NODE> rootNode;					//ルートノード
	std::unordered_map<std::string, MODEL_NODE*> nodeMap;	//ノード名からノードを検索するためのマップ
};

//========================================================
// モデルキャッシュエントリ構造体
//========================================================
struct MODEL_CACHE_ENTRY {
	std::unique_ptr<MODEL> model;	//モデルデータ
	int refCount = 0;				//参照カウント
};

//========================================================
// モデルレンダラークラス
//========================================================
class ModelRenderer {
public:
	ModelRenderer() = default;
	~ModelRenderer();

	//コピー禁止
	ModelRenderer(const ModelRenderer&) = delete;
	ModelRenderer& operator=(const ModelRenderer&) = delete;

	//モデル読み込み
	bool Load(const std::string& fileName);

	//描画(オイラー角)
	void Draw(const Vector3& position, const Vector3& rotation, const Vector3& scale);

	//描画(クォータニオン)
	void Draw(const Vector3& position, const Vector4& rotation, const Vector3& scale);

	//全モデル解放
	static void ReleaseAll();

	/************************
	* マテリアル操作
	*************************/
	//マテリアル数を取得
	int GetMaterialCount() const;

	//マテリアルを取得(インデックス)
	MODEL_MATERIAL* GetMaterial(int index);
	const MODEL_MATERIAL* GetMaterial(int index) const;

	//マテリアルを取得(名前)
	MODEL_MATERIAL* GetMaterial(const std::string& name);
	const MODEL_MATERIAL* GetMaterial(const std::string& name) const;

	//マテリアルのインデックスを取得(名前)
	int GetMaterialIndex(const std::string& name) const;

	//マテリアルのdiffuseColorを設定
	bool SetMaterialDiffuseColor(int index, const Vector4& color);
	bool SetMaterialDiffuseColor(const std::string& name, const Vector4& color);

	//マテリアルのspecularColorを設定
	bool SetMaterialSpecularColor(int index, const Vector4& color);
	bool SetMaterialSpecularColor(const std::string& name, const Vector4& color);

	//マテリアルのambientColorを設定
	bool SetMaterialAmbientColor(int index, const Vector4& color);
	bool SetMaterialAmbientColor(const std::string& name, const Vector4& color);

	//マテリアルのemissiveColorを設定
	bool SetMaterialEmissiveColor(int index, const Vector4& color);
	bool SetMaterialEmissiveColor(const std::string& name, const Vector4& color);

private:
	//描画モデル
	MODEL* m_model = nullptr;

	//参照しているモデルファイル名
	std::string m_modelFileName;

	//モデルキャッシュ
	static std::unordered_map<std::string, MODEL_CACHE_ENTRY> m_modelCache;

	//参照カウント増加
	void IncrementReference(const std::string& fileName);

	//参照カウント減少
	void DecrementReference();

	//内部描画関数
	void DrawInternal(const XMMATRIX& world);

	//ノードの再帰描画
	void DrawNode(MODEL_NODE* node, const XMMATRIX& parentTransform);

	//メッシュの描画
	void DrawMesh(const MODEL_MESH& mesh, const XMMATRIX& world);

	//マテリアル設定
	void SetMaterial(const MODEL_MATERIAL& material) const;

	//モデルの読み込み
	MODEL* LoadModelInternal(const std::string& fileName);

	//assimpシーンからモデルを構築
	bool BuildModelFromScene(const aiScene* scene, MODEL* model);

	//テクスチャの読み込み
	bool LoadEmbeddedTexture(const aiScene* scene, MODEL* model);

	//テクスチャのインデックス取得(無ければ-1を返す)
	int GetTextureIndex(const std::string& texturePath, MODEL* model);

	//マテリアルの処理
	bool ProcessMaterial(const aiScene* scene, MODEL* model);

	//メッシュの処理
	bool ProcessMesh(const aiMesh* aiMesh, MODEL* model);

	//ノードの処理
	std::unique_ptr<MODEL_NODE> ProcessNode(const aiNode* aiNode, MODEL_NODE* parent, MODEL* model);

	//GPU頂点バッファとインデックスバッファの作成
	bool CreateBuffers(MODEL_MESH& mesh);
};
