#pragma once

class RaceCourseManager;
struct VERTEX_3D;

//コースを描画するクラス
class CourseRenderer {
public:
	CourseRenderer();
	~CourseRenderer() = default;

	//初期化
	bool Initialize();
	//終了
	void Finalize();

	//コース描画
	void DrawCourse(const RaceCourseManager* course);

	//デバッグ用コントロールポイント描画

	//頂点バッファの更新が必要かチェック
	void UpdateMeshIfNeeded(const RaceCourseManager* course);

private:
	//地面用リソース
	//頂点バッファ
	ComPtr<ID3D11Buffer> m_roadVertexBuffer;
	//頂点数
	UINT m_roadVertexCount = 0;
	//インデックスバッファ
	ComPtr<ID3D11Buffer> m_roadIndexBuffer;
	//インデックス数
	UINT m_roadIndexCount = 0;

	//ライン用リソース
	//頂点バッファ
	ComPtr<ID3D11Buffer> m_lineVertexBuffer;
	//頂点数
	UINT m_lineVertexCount = 0;
	//インデックスバッファ
	ComPtr<ID3D11Buffer> m_lineIndexBuffer;
	//インデックス数
	UINT m_lineIndexCount = 0;

	//デバッグポイント用リソース
	//頂点バッファ
	ComPtr<ID3D11Buffer> m_pointVertexBuffer;
	//頂点数
	UINT m_pointVertexCount = 0;
	//インデックスバッファ
	ComPtr<ID3D11Buffer> m_pointIndexBuffer;
	//インデックス数
	UINT m_pointIndexCount = 0;

	//シェーダーリソース
	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3D11PixelShader> m_pixelShader;
	ComPtr<ID3D11InputLayout> m_inputLayout;

	//更新管理
	bool m_needsRoadUpdate = false; //コースメッシュ更新フラグ
	bool m_needsLineUpdate = false; //ラインメッシュ更新フラグ
	bool m_needsPointUpdate = false; //ポイントメッシュ更新フラグ
	size_t m_lastControlPointCount = 0; //前回のコントロールポイント数
	bool m_lastClosedState = false; //前回の閉じているか状態

	//バッファ更新関数
	//コースメッシュ更新
	void UpdateRoadMesh(const RaceCourseManager* course);
	//ラインメッシュ更新
	void UpdateLineMesh(const RaceCourseManager* course);
	//ポイントメッシュ更新
	void UpdatePointMesh(const RaceCourseManager* course);

	//バッファ作成
	bool CreateBuffer(ComPtr<ID3D11Buffer>& buffer, const void* data, UINT size, D3D11_BIND_FLAG bindFlag);
	bool CreateDynamicBuffer(ComPtr<ID3D11Buffer>& buffer, UINT size, D3D11_BIND_FLAG bindFlag);

	//メッシュ生成ヘルパー
	void GenerateRoadVertices(const RaceCourseManager* course, std::vector<VERTEX_3D>& outVertices, std::vector<UINT>& outIndices);
	void GenerateLineVertices(const RaceCourseManager* course, std::vector<VERTEX_3D>& outVertices, std::vector<UINT>& outIndices);
	void GeneratePointVertices(const RaceCourseManager* course, std::vector<VERTEX_3D>& outVertices, std::vector<UINT>& outIndices);

	//描画サブ関数
	void DrawRoadSurface();
	void DrawRoadLines();
	void DrawControlPoints();

	//ユーティリティ
	Vector3 GetPerpendicularVector(const Vector3& direction) const;
	void CreateLineSegment(const Vector3& start, const Vector3& end, const Vector3& color, float width, float height, std::vector<VERTEX_3D>& vertices, std::vector<UINT>& indices, UINT& baseIndex);
};
