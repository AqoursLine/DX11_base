#include "main.h"
#include "renderer.h"
#include "modelRenderer.h"

//静的メンバーの初期化
std::unordered_map<std::string, MODEL_CACHE_ENTRY> ModelRenderer::m_modelCache;

//========================================================
//モデルノードの変換行列計算
//========================================================
XMMATRIX MODEL_NODE::GetWorldTransform() const {
	if (parent) {
		return localTransform * parent->GetWorldTransform();
	}
	else {
		return localTransform;
	}
}

//========================================================
//デストラクタ
//========================================================
ModelRenderer::~ModelRenderer() {
	//参照カウントをデクリメント
	DecrementReference();
}

//========================================================
//モデルの読み込み
//========================================================
bool ModelRenderer::Load(const std::string& fileName) {
	//既に読み込んでいる場合はスキップ
	if (m_modelFileName == fileName && m_model != nullptr) {
		return true;
	}

	//参照カウントをデクリメント
	DecrementReference();

	//モデルキャッシュを確認
	auto it = m_modelCache.find(fileName);
	if (it != m_modelCache.end()) {
		//キャッシュから取得
		m_model = it->second.model.get();
		m_modelFileName = fileName;
		//参照カウント増加
		IncrementReference(fileName);
		return true;
	}

	//モデルの読み込み
	m_model = LoadModelInternal(fileName);
	if (!m_model) {
		return false;
	}

	//キャッシュに追加
	MODEL_CACHE_ENTRY entry;
	entry.model = std::unique_ptr<MODEL>(m_model);
	entry.refCount = 1;
	m_modelCache[fileName] = std::move(entry);

	m_modelFileName = fileName;
	return true;
}

//========================================================
//描画(オイラー角)
//========================================================
void ModelRenderer::Draw(const Vector3& position, const Vector3& rotation, const Vector3& scale) {
	if (!m_model) {
		return;
	}

	//オイラー角からワールド行列を計算
	XMMATRIX world =
		XMMatrixScaling(scale.x, scale.y, scale.z) *
		XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) *
		XMMatrixTranslation(position.x, position.y, position.z);

	DrawInternal(world);
}

//========================================================
//描画(クォータニオン)
//========================================================
void ModelRenderer::Draw(const Vector3& position, const Vector4& rotation, const Vector3& scale) {
	if (!m_model) {
		return;
	}

	//クォータニオンから回転行列を計算
	XMMATRIX rot = XMMatrixRotationQuaternion(
		XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w)
	);

	//ワールド行列を計算
	XMMATRIX world =
		XMMatrixScaling(scale.x, scale.y, scale.z) *
		rot *
		XMMatrixTranslation(position.x, position.y, position.z);

	DrawInternal(world);
}

void ModelRenderer::IncrementReference(const std::string& fileName) {
	auto it = m_modelCache.find(fileName);
	if (it != m_modelCache.end()) {
		it->second.refCount++;
	}
}

void ModelRenderer::DecrementReference() {
	if (m_modelCache.empty()) {
		return;
	}
	auto it = m_modelCache.find(m_modelFileName);
	if (it != m_modelCache.end()) {
		it->second.refCount--;
		if (it->second.refCount <= 0) {
			m_modelCache.erase(it);
		}
	}

	//モデルポインタをクリア
	m_model = nullptr;
	m_modelFileName.clear();
}

void ModelRenderer::DrawInternal(const XMMATRIX& world) {
	//プリミティブトポロジー設定
	RENDERER.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//ルートノードから描画
	if (m_model->rootNode) {
		DrawNode(m_model->rootNode.get(), world);
	}
}

void ModelRenderer::DrawNode(MODEL_NODE* node, const XMMATRIX& parentTransform) {
	if (!node) {
		return;
	}

	//ノードのワールド行列を計算
	XMMATRIX world = node->localTransform * parentTransform;

	//ノードに含まれるメッシュを描画
	for (unsigned int meshIndex : node->meshIndices) {
		if (meshIndex < m_model->meshes.size()) {
			DrawMesh(m_model->meshes[meshIndex], world);
		}
	}

	//子ノードを再帰的に描画
	for (auto& child : node->children) {
		DrawNode(child.get(), world);
	}
}

void ModelRenderer::DrawMesh(const MODEL_MESH& mesh, const XMMATRIX& world) {
	//ワールド行列をシェーダーに設定
	RENDERER.SetWorldMatrix(world);

	//マテリアル設定
	if (mesh.materialIndex < m_model->materials.size()) {
		SetMaterial(m_model->materials[mesh.materialIndex]);
	}

	//頂点バッファ設定
	UINT stride = sizeof(MODEL_VERTEX);
	UINT offset = 0;
	RENDERER.GetDeviceContext()->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &offset);

	//インデックスバッファ設定
	RENDERER.GetDeviceContext()->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	//描画
	RENDERER.GetDeviceContext()->DrawIndexed(mesh.indexCount, 0, 0);
}

void ModelRenderer::SetMaterial(const MODEL_MATERIAL& material) const {
	//マテリアル設定
	MATERIAL rendererMaterial = {};

	//基本色
	rendererMaterial.diffuse = XMFLOAT4(
		material.diffuseColor.x,
		material.diffuseColor.y,
		material.diffuseColor.z,
		material.diffuseColor.w
	);
	rendererMaterial.specular = XMFLOAT4(
		material.specularColor.x,
		material.specularColor.y,
		material.specularColor.z,
		material.shininess //w成分に光沢度を格納
	);
	rendererMaterial.ambient = XMFLOAT4(
		material.ambientColor.x,
		material.ambientColor.y,
		material.ambientColor.z,
		material.ambientColor.w
	);
	rendererMaterial.emission = XMFLOAT4(
		material.emissiveColor.x,
		material.emissiveColor.y,
		material.emissiveColor.z,
		material.emissiveColor.w
	);

	//物理パラメータ
	rendererMaterial.shininess = material.shininess;

	//テクスチャ設定
	rendererMaterial.textureEnable = !material.textures.empty();

	//マテリアルをレンダラーに設定
	RENDERER.SetMaterial(rendererMaterial);

	//テクスチャ設定
	for (const auto& texInfo : material.textures) {
		if (texInfo.textureIndex >= 0 && texInfo.textureIndex < static_cast<int>(m_model->textures.size())) {

			//テクスチャタイプに応じてスロットを変更
			UINT slot = 0; //デフォルトはスロット0
			switch (texInfo.TextureType) {
				case 1: slot = 0; break; //diffuse
				case 2: slot = 1; break; //specular
				case 6: slot = 2; break; //normal
				case 4: slot = 3; break; //emissive
				default: slot = 0; break; //その他はスロット0
			}

			//テクスチャをシェーダーに設定
			RENDERER.GetDeviceContext()->PSSetShaderResources(slot, 1, m_model->textures[texInfo.textureIndex].GetAddressOf());
		}
	}

}

//========================================================
//モデルの解放
//========================================================
void ModelRenderer::ReleaseAll() {
	m_modelCache.clear();
}

//========================================================
//マテリアル数を取得
//========================================================
int ModelRenderer::GetMaterialCount() const {
	if (!m_model) {
		return 0;
	}
	return static_cast<int>(m_model->materials.size());
}

//========================================================
//マテリアルを取得(インデックス)
//========================================================
MODEL_MATERIAL* ModelRenderer::GetMaterial(int index) {
	if (!m_model) {
		return nullptr;
	}
	if (index < 0 || index >= static_cast<int>(m_model->materials.size())) {
		return nullptr;
	}
	return &m_model->materials[index];
}

const MODEL_MATERIAL* ModelRenderer::GetMaterial(int index) const {
	if (!m_model) {
		return nullptr;
	}
	if (index < 0 || index >= static_cast<int>(m_model->materials.size())) {
		return nullptr;
	}
	return &m_model->materials[index];
}

//========================================================
//マテリアルを取得(名前)
//========================================================
MODEL_MATERIAL* ModelRenderer::GetMaterial(const std::string& name) {
	if (!m_model) {
		return nullptr;
	}
	for (auto& material : m_model->materials) {
		if (material.name == name) {
			return &material;
		}
	}
	return nullptr;
}

const MODEL_MATERIAL* ModelRenderer::GetMaterial(const std::string& name) const {
	if (!m_model) {
		return nullptr;
	}
	for (const auto& material : m_model->materials) {
		if (material.name == name) {
			return &material;
		}
	}
	return nullptr;
}

//========================================================
//マテリアルのインデックスを取得(名前)
//========================================================
int ModelRenderer::GetMaterialIndex(const std::string& name) const {
	if (!m_model) {
		return -1;
	}
	for (size_t i = 0; i < m_model->materials.size(); ++i) {
		if (m_model->materials[i].name == name) {
			return static_cast<int>(i);
		}
	}
	return -1;
}

//========================================================
//マテリアルのdiffuseColorを設定
//========================================================
bool ModelRenderer::SetMaterialDiffuseColor(int index, const Vector4& color) {
	MODEL_MATERIAL* material = GetMaterial(index);
	if (!material) {
		return false;
	}
	material->diffuseColor = color;
	return true;
}

bool ModelRenderer::SetMaterialDiffuseColor(const std::string& name, const Vector4& color) {
	MODEL_MATERIAL* material = GetMaterial(name);
	if (!material) {
		return false;
	}
	material->diffuseColor = color;
	return true;
}

//========================================================
//マテリアルのspecularColorを設定
//========================================================
bool ModelRenderer::SetMaterialSpecularColor(int index, const Vector4& color) {
	MODEL_MATERIAL* material = GetMaterial(index);
	if (!material) {
		return false;
	}
	material->specularColor = color;
	return true;
}

bool ModelRenderer::SetMaterialSpecularColor(const std::string& name, const Vector4& color) {
	MODEL_MATERIAL* material = GetMaterial(name);
	if (!material) {
		return false;
	}
	material->specularColor = color;
	return true;
}

//========================================================
//マテリアルのambientColorを設定
//========================================================
bool ModelRenderer::SetMaterialAmbientColor(int index, const Vector4& color) {
	MODEL_MATERIAL* material = GetMaterial(index);
	if (!material) {
		return false;
	}
	material->ambientColor = color;
	return true;
}

bool ModelRenderer::SetMaterialAmbientColor(const std::string& name, const Vector4& color) {
	MODEL_MATERIAL* material = GetMaterial(name);
	if (!material) {
		return false;
	}
	material->ambientColor = color;
	return true;
}

//========================================================
//マテリアルのemissiveColorを設定
//========================================================
bool ModelRenderer::SetMaterialEmissiveColor(int index, const Vector4& color) {
	MODEL_MATERIAL* material = GetMaterial(index);
	if (!material) {
		return false;
	}
	material->emissiveColor = color;
	return true;
}

bool ModelRenderer::SetMaterialEmissiveColor(const std::string& name, const Vector4& color) {
	MODEL_MATERIAL* material = GetMaterial(name);
	if (!material) {
		return false;
	}
	material->emissiveColor = color;
	return true;
}

//========================================================
//テクスチャ数を取得
//========================================================
int ModelRenderer::GetTextureCount() const {
	if (!m_model) {
		return 0;
	}
	return static_cast<int>(m_model->textures.size());
}

//========================================================
//テクスチャを取得(インデックス)
//========================================================
ID3D11ShaderResourceView* ModelRenderer::GetTexture(int index) const {
	if (!m_model) {
		return nullptr;
	}
	if (index < 0 || index >= static_cast<int>(m_model->textures.size())) {
		return nullptr;
	}
	return m_model->textures[index].Get();
}
