#include "main.h"
#include "Dx11/renderer.h"
#include "model.h"
#include <fstream>

//static変数の初期化
//テクスチャキャッシュ
std::unordered_map<std::string, TEXTURE_CACHE_ENTRY> Model::m_textureCache;
//マテリアルキャッシュ
std::unordered_map<std::string, MATERIAL_CACHE_ENTRY> Model::m_materialCache;


//assimpでFBXを読み込むための設定
bool Model::LoadModelFBX(const std::string& fileName, const std::wstring& vertexShader, const std::wstring& pixelShader) {
	//ファイルパスからディレクトリパスを抽出
	m_directory = fileName.substr(0, fileName.find_last_of("/\\"));

	//インポーター
	Assimp::Importer importer;

	//モデルの読み込みオプション
	unsigned int flags;
	flags = aiProcess_Triangulate |				//全ての図形を三角形化
			aiProcess_GenSmoothNormals |		//法線ベクトルを生成
			aiProcess_FlipUVs |					//UVをD3D向けに反転
			aiProcess_CalcTangentSpace |		//接戦空間を計算
			aiProcess_JoinIdenticalVertices;	//頂点を結合

	//モデルを読み込む
	const aiScene* scene = importer.ReadFile(fileName, flags);

	//読み込み失敗
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
		return false;
	}

	//ルートノードから処理を開始
	ProcessNode(scene->mRootNode, scene, m_directory);

	//シェーダーの作成
	RENDERER.CreateVertexShader(&m_vertexShader, &m_inputLayout, vertexShader);
	RENDERER.CreatePixelShader(&m_pixelShader, pixelShader);

	return true;
}

//メッシュのメモリを解放する関数
void Model::ReleaseModel() {
	//使用したマテリアルの参照カウントを減らす
	for (const auto& key : m_usedMaterialKeys) {
		auto it = m_materialCache.find(key);
		if (it != m_materialCache.end()) {
			it->second.referenceCount--;

			//参照カウントが0になったらキャッシュから削除
			if (it->second.referenceCount <= 0) {
				m_materialCache.erase(it);	//キャッシュから削除
			}
		}
	}

	//使用したテクスチャの参照カウントを減らす
	for (const auto& key : m_usedTexturePaths) {
		auto it = m_textureCache.find(key);
		if (it != m_textureCache.end()) {
			it->second.referenceCount--;

			//参照カウントが0になったらキャッシュから削除
			if (it->second.referenceCount <= 0) {
				if (it->second.srv) {
					it->second.srv->Release();
				}
				m_textureCache.erase(it);	//キャッシュから削除
			}
		}
	}

	//リソースの解放
	for (auto& mesh : m_meshes) {
		if (mesh.vertexBuffer) {
			mesh.vertexBuffer->Release();	//頂点バッファの解放
		}
		if (mesh.indexBuffer) {
			mesh.indexBuffer->Release();	//インデックスバッファの解放
		}
	}
}

void Model::Draw(const Vector3& position, const Vector3& rotation, const Vector3& scale) const {
	//プリミティブトポロジーを設定
	RENDERER.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//ワールド行列を設定
	XMMATRIX worldMatrix, scaleMatrix, rotMatrix, posMatrix;	//単位行列
	scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);	//スケーリング
	rotMatrix = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);	//回転
	posMatrix = XMMatrixTranslation(position.x, position.y, position.z);	//平行移動
	worldMatrix = scaleMatrix * rotMatrix * posMatrix;	//ワールド行列を計算
	RENDERER.SetWorldMatrix(worldMatrix);	//ワールド行列をセット

	//サンプラーステートをセット
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;	//線形フィルタリング
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;	//テクスチャ座標のラッピング
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;	//テクスチャ座標のラッピング
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;	//テクスチャ座標のラッピング
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;	//最大LOD
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;	//比較関数

	ID3D11SamplerState* samplerState = nullptr;
	HRESULT hr = RENDERER.GetDevice()->CreateSamplerState(&samplerDesc, &samplerState);
	if (FAILED(hr)) {
		ErrorMessage(L"サンプラーステートの作成に失敗しました。", hr);
		return;
	}
	RENDERER.GetDeviceContext()->PSSetSamplers(0, 1, &samplerState);	//サンプラーステートをセット

	//入力レイアウトをセット
	RENDERER.GetDeviceContext()->IASetInputLayout(m_inputLayout);	//入力レイアウトをセット
	//頂点シェーダーをセット
	RENDERER.GetDeviceContext()->VSSetShader(m_vertexShader, nullptr, 0);	//頂点シェーダーをセット
	//ピクセルシェーダーをセット
	RENDERER.GetDeviceContext()->PSSetShader(m_pixelShader, nullptr, 0);	//ピクセルシェーダーをセット

	//メッシュを描画
	for (const auto& mesh : m_meshes) {
		//頂点バッファをセット
		UINT stride = sizeof(VERTEX_3D);
		UINT offset = 0;
		RENDERER.GetDeviceContext()->IASetVertexBuffers(0, 1, &mesh.vertexBuffer, &stride, &offset);
		//インデックスバッファをセット
		RENDERER.GetDeviceContext()->IASetIndexBuffer(mesh.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		//マテリアルをD3D用に変換してセット
		MATERIAL mat = {};
		mat.diffuse = mesh.material.diffuse;
		mat.specular = mesh.material.specular;
		mat.ambient = mesh.material.ambient;
		mat.shininess = mesh.material.shininess;
		mat.textureEnable = (mesh.material.texture != nullptr);
		RENDERER.SetMaterial(mat);	//マテリアルをセット

		//テクスチャをセット
		if (mesh.material.texture) {
			RENDERER.GetDeviceContext()->PSSetShaderResources(0, 1, &mesh.material.texture);	//テクスチャをセット
		}

		//描画
		RENDERER.GetDeviceContext()->DrawIndexed(mesh.numIndices, 0, 0);
	}

	//サンプラーステートを解放
	if (samplerState) {
		samplerState->Release();	//サンプラーステートを解放
	}
}

void Model::ClearCache() {
	//テクスチャキャッシュをクリア
	for (auto& it : m_textureCache) {
		if (it.second.srv) {
			it.second.srv->Release();	//テクスチャを解放
		}
	}
	m_textureCache.clear();	//キャッシュをクリア

	//マテリアルキャッシュをクリア
	m_materialCache.clear();	//キャッシュをクリア
}

///ノードを処理する関数
void Model::ProcessNode(aiNode* node, const aiScene* scene, const std::string& modelDirectory) {
	//ノードのメッシュを処理する
	for (UINT i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		ProcessMesh(mesh, scene, modelDirectory);
	}

	//ノードの子ノードを処理する
	for (UINT i = 0; i < node->mNumChildren; i++) {
		ProcessNode(node->mChildren[i], scene, modelDirectory);
	}
}

//メッシュを処理する関数
void Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, const std::string& modelDirectory) {
	std::vector<VERTEX_3D> vertices;
	std::vector<UINT> indices;

	//頂点データを処理
	for (UINT i = 0; i < mesh->mNumVertices; i++) {
		VERTEX_3D vertex;

		//頂点座標
		vertex.position.x = mesh->mVertices[i].x;
		vertex.position.y = mesh->mVertices[i].y;
		vertex.position.z = mesh->mVertices[i].z;

		//法線ベクトル
		if (mesh->HasNormals()) {
			vertex.normal.x = mesh->mNormals[i].x;
			vertex.normal.y = mesh->mNormals[i].y;
			vertex.normal.z = mesh->mNormals[i].z;
		}

		//テクスチャ座標
		if (mesh->mTextureCoords[0]) {
			vertex.texcoord.x = mesh->mTextureCoords[0][i].x;
			vertex.texcoord.y = mesh->mTextureCoords[0][i].y;
		} else {
			vertex.texcoord.x = 0.0f;
			vertex.texcoord.y = 0.0f;
		}

		//頂点カラー
		if (mesh->HasVertexColors(0)) {
			vertex.diffuse.x = mesh->mColors[0][i].r;
			vertex.diffuse.y = mesh->mColors[0][i].g;
			vertex.diffuse.z = mesh->mColors[0][i].b;
			vertex.diffuse.w = mesh->mColors[0][i].a;
		} else {
			vertex.diffuse.x = 1.0f;
			vertex.diffuse.y = 1.0f;
			vertex.diffuse.z = 1.0f;
			vertex.diffuse.w = 1.0f;
		}

		vertices.push_back(vertex);
	}

	//インデックスデータを処理
	for (UINT i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (UINT j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	//頂点バッファを作成
	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;	//使用法
	vertexBufferDesc.ByteWidth = sizeof(VERTEX_3D) * static_cast<UINT>(vertices.size());	//バッファサイズ
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;	//バインドフラグ
	vertexBufferDesc.CPUAccessFlags = 0;	//CPUアクセスフラグ

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = vertices.data();	//データポインタ

	ID3D11Buffer* vertexBuffer = nullptr;
	HRESULT hr = RENDERER.GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);
	if (FAILED(hr)) {
		ErrorMessage(L"頂点バッファの作成に失敗しました。", hr);
		return;
	}

	//インデックスバッファを作成
	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;	//使用法
	indexBufferDesc.ByteWidth = sizeof(UINT) * static_cast<UINT>(indices.size());	//バッファサイズ
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;	//バインドフラグ
	indexBufferDesc.CPUAccessFlags = 0;	//CPUアクセスフラグ

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indices.data();	//データポインタ

	ID3D11Buffer* indexBuffer = nullptr;
	hr = RENDERER.GetDevice()->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);
	if (FAILED(hr)) {
		ErrorMessage(L"インデックスバッファの作成に失敗しました。", hr);
		return;
	}

	//マテリアルを処理
	MODEL_MATERIAL material{};
	if (mesh->mMaterialIndex < scene->mNumMaterials) {
		material = LoadMaterial(scene->mMaterials[mesh->mMaterialIndex], scene, modelDirectory);
	}

	//メッシュを保存
	MESH newMesh;
	newMesh.vertexBuffer = vertexBuffer;	//頂点バッファ
	newMesh.indexBuffer = indexBuffer;	//インデックスバッファ
	newMesh.numIndices = static_cast<UINT>(indices.size());	//インデックス数
	newMesh.material = material;	//マテリアル
	m_meshes.push_back(newMesh);	//メッシュを保存
}

MODEL_MATERIAL Model::LoadMaterial(aiMaterial* aiMat, const aiScene* scene, const std::string& modeDirectory) {
	//一時的なマテリアルを作成してキーを作成
	MODEL_MATERIAL tmpMaterial{};

	tmpMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);	//デフォルトの色
	tmpMaterial.specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);	//デフォルトの色
	tmpMaterial.ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);	//デフォルトの色
	tmpMaterial.shininess = 32.0f;	//デフォルトの値
	tmpMaterial.texture = nullptr;	//デフォルトのテクスチャ
	tmpMaterial.texturePath = "";	//デフォルトのパス

	//マテリアルプロパティを読み込む
	aiColor4D color(0.0f, 0.0f, 0.0f, 0.0f);
	float shininess = 0.0f;

	if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
		tmpMaterial.diffuse = XMFLOAT4(color.r, color.g, color.b, color.a);
	}

	if (aiMat->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
		tmpMaterial.specular = XMFLOAT4(color.r, color.g, color.b, color.a);
	}

	if (aiMat->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS) {
		tmpMaterial.ambient = XMFLOAT4(color.r, color.g, color.b, color.a);
	}

	if (aiMat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
		tmpMaterial.shininess = shininess;
	}

	//テクスチャパスを取得
	aiString texturePath;
	if (aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
		tmpMaterial.texturePath = texturePath.C_Str();	//テクスチャパスを保存
	}

	//マテリアルのハッシュキーを生成
	std::string materialKey = tmpMaterial.GenerateHashKey();

	//マテリアルがすでに存在するか確認
	auto it = m_materialCache.find(materialKey);
	if (it != m_materialCache.end()) {
		//存在する場合はキャッシュから取得
		it->second.referenceCount++;
		m_usedMaterialKeys.push_back(materialKey);	//使用中のマテリアルキーを保存
		return it->second.material;
	}

	//存在しない場合は新しいマテリアルを作成
	MODEL_MATERIAL newMaterial = tmpMaterial;

	//テクスチャをロード
	if (!tmpMaterial.texturePath.empty()) {
		//組み込みテクスチャかどうかを確認
		UINT texnum = scene->mNumTextures;
		if (texnum > 0) {
			unsigned int textureIndex = 0;
			for (textureIndex = 0; textureIndex < scene->mNumTextures; textureIndex++) {
				if (strcmp(scene->mTextures[textureIndex]->mFilename.C_Str(), newMaterial.texturePath.c_str()) == 0) {
					break;
				}
			}
			aiMat->GetTextureCount(aiTextureType_DIFFUSE);	//テクスチャの数を取得
			//組み込みテクスチャをロード
			newMaterial.texture = LoadEmbeddedTexture(scene->mTextures[textureIndex], textureIndex);
			//失敗
			if (!newMaterial.texture) {
				ErrorMessage(L"組み込みテクスチャのロードに失敗しました。", E_FAIL);
			}
		} else {
			//外部テクスチャをロード
			std::string fullPath;
			if (m_directory.empty()) {
				fullPath = tmpMaterial.texturePath;
			} else {
				fullPath = m_directory + "\\" + tmpMaterial.texturePath;
			}

			//ファイルが存在するか確認
			std::ifstream file(fullPath);

			if (file.good()) {
				file.close();

				//外部テクスチャをロード
				newMaterial.texture = LoadTexture(fullPath);
				//失敗
				if (!newMaterial.texture) {
					ErrorMessage(L"外部テクスチャのロードに失敗しました。", E_FAIL);
				}
			} else {
				//ファイルが存在しない
				ErrorMessage(L"外部テクスチャが存在しません。", E_FAIL);
			}
		}
	}

	//マテリアルをキャッシュに追加
	m_materialCache[materialKey] = MATERIAL_CACHE_ENTRY(newMaterial);
	m_usedMaterialKeys.push_back(materialKey);	//使用中のマテリアルキーを保存

	return newMaterial;
}

ID3D11ShaderResourceView* Model::LoadEmbeddedTexture(const aiTexture* embeddedTexture, int textureIndex) {
	//組み込みテクスチャの識別子を作成
	std::string embeddedKey = "EmbeddedTexture_" + std::to_string(textureIndex);

	//キャッシュに存在するか確認
	auto it = m_textureCache.find(embeddedKey);
	if (it != m_textureCache.end()) {
		//存在する場合はキャッシュから取得
		it->second.referenceCount++;
		m_usedTexturePaths.push_back(embeddedKey);	//使用中のテクスチャキーを保存
		return it->second.srv;
	}

	//存在しない場合は新しいテクスチャを作成
	ScratchImage image;
	ID3D11ShaderResourceView* textureView = nullptr;
	HRESULT hr;

	if (embeddedTexture->mHeight == 0) {
		//圧縮されたテクスチャ形式
		//フォーマットを判別
		if (strncmp(reinterpret_cast<const char*>(embeddedTexture->achFormatHint), "DDS", 4) == 0) {
			//DDS形式
			hr = LoadFromDDSMemory((uint8_t*)embeddedTexture->pcData, embeddedTexture->mWidth, DDS_FLAGS_NONE, nullptr, image);
		}else if (strncmp(reinterpret_cast<const char*>(embeddedTexture->achFormatHint), "TGA", 4) == 0) {
			//TGA形式
			hr = LoadFromTGAMemory((uint8_t*)embeddedTexture->pcData, embeddedTexture->mWidth, TGA_FLAGS_NONE, nullptr, image);
		} else {
			//WIC形式
			hr = LoadFromWICMemory((uint8_t*)embeddedTexture->pcData, embeddedTexture->mWidth, WIC_FLAGS_NONE, nullptr, image);
		}

		//失敗
		if (FAILED(hr)) {
			ErrorMessage(L"組み込みテクスチャのロードに失敗しました。", hr);
			return nullptr;
		}
	} else {
		//非圧縮されたテクスチャ形式
		Image img;
		img.width = embeddedTexture->mWidth;
		img.height = embeddedTexture->mHeight;
		img.format = DXGI_FORMAT_R8G8B8A8_UNORM;
		img.rowPitch = embeddedTexture->mWidth * 4;	//1行のバイト数
		img.slicePitch = img.rowPitch * embeddedTexture->mHeight;	//1スライスのバイト数
		img.pixels = reinterpret_cast<uint8_t*>(embeddedTexture->pcData);	//ピクセルデータ

		//画像を作成
		hr = image.InitializeFromImage(img);
		if (FAILED(hr)) {
			ErrorMessage(L"組み込みテクスチャのロードに失敗しました。", hr);
			return nullptr;
		}
	}

	//シェーダーリソースビューを作成
	hr = CreateShaderResourceView(RENDERER.GetDevice(), image.GetImages(), image.GetImageCount(), image.GetMetadata(), &textureView);
	if (FAILED(hr)) {
		ErrorMessage(L"組み込みテクスチャのシェーダーリソースビューの作成に失敗しました。", hr);
		return nullptr;
	}

	//テクスチャをキャッシュに追加
	if (textureView) {
		m_textureCache[embeddedKey] = TEXTURE_CACHE_ENTRY(textureView);
		m_usedTexturePaths.push_back(embeddedKey);	//使用中のテクスチャキーを保存
	}

	return textureView;
}

ID3D11ShaderResourceView* Model::LoadTexture(const std::string& texturePath) {
	//キャッシュを確認
	auto it = m_textureCache.find(texturePath);

	if (it != m_textureCache.end()) {
		//存在する場合はキャッシュから取得
		it->second.referenceCount++;
		m_usedTexturePaths.push_back(texturePath);	//使用中のテクスチャキーを保存
		return it->second.srv;
	}

	//存在しない場合は新しいテクスチャを作成
	ID3D11ShaderResourceView* textureView = LoadTextureFromFile(texturePath);
	if (textureView) {
		//キャッシュに追加
		m_textureCache[texturePath] = TEXTURE_CACHE_ENTRY(textureView);
		m_usedTexturePaths.push_back(texturePath);	//使用中のテクスチャキーを保存
	}

	return textureView;
}

ID3D11ShaderResourceView* Model::LoadTextureFromFile(const std::string& texturePath) {
	//テクスチャファイルをワイド文字列に変換
	std::wstring wTexturePath(texturePath.begin(), texturePath.end());

	//テクスチャを読み込む
	ScratchImage image;
	HRESULT hr;

	//拡張子によって読み込み方法を変更
	std::string extension = texturePath.substr(texturePath.find_last_of('.'));
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

	if (extension == ".dds") {
		//DDS形式
		hr = LoadFromDDSFile(wTexturePath.c_str(), DDS_FLAGS_NONE, nullptr, image);
	} else if (extension == ".tga") {
		//TGA形式
		hr = LoadFromTGAFile(wTexturePath.c_str(), nullptr, image);
	} else if (extension == ".hdr") {
		//HDR形式
		hr = LoadFromHDRFile(wTexturePath.c_str(), nullptr, image);
	} else {
		//WIC形式
		hr = LoadFromWICFile(wTexturePath.c_str(), WIC_FLAGS_NONE, nullptr, image);
	}

	//失敗
	if (FAILED(hr)) {
		ErrorMessage(L"テクスチャのロードに失敗しました。", hr);
		return nullptr;
	}

	//シェーダーリソースビューを作成
	ID3D11ShaderResourceView* textureView = nullptr;
	hr = CreateShaderResourceView(RENDERER.GetDevice(), image.GetImages(), image.GetImageCount(), image.GetMetadata(), &textureView);
	if (FAILED(hr)) {
		ErrorMessage(L"テクスチャのシェーダーリソースビューの作成に失敗しました。", hr);
		return nullptr;
	}

	return textureView;
}
