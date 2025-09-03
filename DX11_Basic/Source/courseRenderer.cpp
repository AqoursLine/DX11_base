#include "main.h"
#include "renderer.h"
#include "courseRenderer.h"
#include "raceCourseManager.h"

CourseRenderer::CourseRenderer()
	: m_roadVertexCount(0), m_roadIndexCount(0)
	, m_lineVertexCount(0), m_lineIndexCount(0)
	, m_pointVertexCount(0), m_pointIndexCount(0)
	, m_needsRoadUpdate(true), m_needsLineUpdate(true), m_needsPointUpdate(true)
	, m_lastControlPointCount(0), m_lastClosedState(false) {
}

bool CourseRenderer::Initialize() {
	//シェーダーの作成
	RENDERER.CreateVertexShader(&m_vertexShader, &m_inputLayout, L"Shader\\unlitColorVS.cso");
	RENDERER.CreatePixelShader(&m_pixelShader, L"Shader\\unlitColorPS.cso");
	return true;
}

void CourseRenderer::Finalize() {
}

void CourseRenderer::DrawCourse(const RaceCourseManager* course) {
	if (course->GetControlPointCount() < 2) {
		return;
	}

	//必要に応じてメッシュ更新
	UpdateMeshIfNeeded(course);

	//シェーダー設定
	RENDERER.GetDeviceContext()->IASetInputLayout(m_inputLayout.Get());
	RENDERER.GetDeviceContext()->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	RENDERER.GetDeviceContext()->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	//デフォルトサンプラーステートセット
	RENDERER.SetSamplerState();
	RENDERER.SetDepthStencilState(true);

	//道路面を描画
	DrawRoadSurface();

	//道路線を描画
	const auto& settings = course->GetRenderSettings();
	if (settings.showCenterLine || settings.showSideLines) {
		DrawRoadLines();
	}
}

void CourseRenderer::UpdateMeshIfNeeded(const RaceCourseManager* course) {
	//コース状態の変更をチェック
	int currentPointCount = course->GetControlPointCount();
	bool currentClosedState = course->IsClosed();

	bool courseChanged = (currentPointCount != m_lastControlPointCount) || (currentClosedState != m_lastClosedState);

	if (courseChanged) {
		m_needsRoadUpdate = true;
		m_needsLineUpdate = true;
		m_needsPointUpdate = true;
		m_lastControlPointCount = currentPointCount;
		m_lastClosedState = currentClosedState;
	}

	//各バッファを必要に応じて更新
	if (m_needsRoadUpdate) {
		//道路メッシュ更新
		UpdateRoadMesh(course);
		m_needsRoadUpdate = false;
	}

	if (m_needsLineUpdate) {
		//道路線メッシュ更新
		UpdateLineMesh(course);
		m_needsLineUpdate = false;
	}

	if (m_needsPointUpdate) {
		//コントロールポイントメッシュ更新
		//UpdatePointMesh(course);
		m_needsPointUpdate = false;
	}
}

void CourseRenderer::UpdateRoadMesh(const RaceCourseManager* course) {
	//頂点・インデックスデータ生成
	std::vector<VERTEX_3D> vertices;
	std::vector<UINT> indices;
	GenerateRoadVertices(course, vertices, indices);
	//頂点バッファ作成
	if (!CreateBuffer(m_roadVertexBuffer, vertices.data(), static_cast<UINT>(vertices.size() * sizeof(VERTEX_3D)), D3D11_BIND_VERTEX_BUFFER)) {
		return;
	}
	m_roadVertexCount = static_cast<UINT>(vertices.size());
	//インデックスバッファ作成
	if (!CreateBuffer(m_roadIndexBuffer, indices.data(), static_cast<UINT>(indices.size() * sizeof(UINT)), D3D11_BIND_INDEX_BUFFER)) {
		return;
	}
	m_roadIndexCount = static_cast<UINT>(indices.size());
}

void CourseRenderer::UpdateLineMesh(const RaceCourseManager* course) {
	//頂点・インデックスデータ生成
	std::vector<VERTEX_3D> vertices;
	std::vector<UINT> indices;
	GenerateLineVertices(course, vertices, indices);
	//頂点バッファ作成
	if (!CreateBuffer(m_lineVertexBuffer, vertices.data(), static_cast<UINT>(vertices.size() * sizeof(VERTEX_3D)), D3D11_BIND_VERTEX_BUFFER)) {
		return;
	}
	m_lineVertexCount = static_cast<UINT>(vertices.size());
	//インデックスバッファ作成
	if (!CreateBuffer(m_lineIndexBuffer, indices.data(), static_cast<UINT>(indices.size() * sizeof(UINT)), D3D11_BIND_INDEX_BUFFER)) {
		return;
	}
	m_lineIndexCount = static_cast<UINT>(indices.size());
}

void CourseRenderer::UpdatePointMesh(const RaceCourseManager* course) {
	//頂点・インデックスデータ生成
	std::vector<VERTEX_3D> vertices;
	std::vector<UINT> indices;
	GeneratePointVertices(course, vertices, indices);
	//頂点バッファ作成
	if (!CreateBuffer(m_pointVertexBuffer, vertices.data(), static_cast<UINT>(vertices.size() * sizeof(VERTEX_3D)), D3D11_BIND_VERTEX_BUFFER)) {
		return;
	}
	m_pointVertexCount = static_cast<UINT>(vertices.size());
	//インデックスバッファ作成
	if (!CreateBuffer(m_pointIndexBuffer, indices.data(), static_cast<UINT>(indices.size() * sizeof(UINT)), D3D11_BIND_INDEX_BUFFER)) {
		return;
	}
	m_pointIndexCount = static_cast<UINT>(indices.size());
}

bool CourseRenderer::CreateBuffer(ComPtr<ID3D11Buffer>& buffer, const void* data, UINT size, D3D11_BIND_FLAG bindFlag) {
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = size;
	bufferDesc.BindFlags = bindFlag;
	bufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = data;

	HRESULT hr = RENDERER.GetDevice()->CreateBuffer(&bufferDesc, &initData, buffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"バッファの初期化に失敗しました。", hr);
		return false;
	}
	return true;
}

bool CourseRenderer::CreateDynamicBuffer(ComPtr<ID3D11Buffer>& buffer, UINT size, D3D11_BIND_FLAG bindFlag) {
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = size;
	bufferDesc.BindFlags = bindFlag;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = RENDERER.GetDevice()->CreateBuffer(&bufferDesc, nullptr, buffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"動的バッファの初期化に失敗しました。", hr);
		return false;
	}
	return true;
}

void CourseRenderer::GenerateRoadVertices(const RaceCourseManager* course, std::vector<VERTEX_3D>& outVertices, std::vector<UINT>& outIndices) {
	outVertices.clear();
	outIndices.clear();

	const int segmentCount = course->IsClosed() ? course->GetControlPointCount() : course->GetControlPointCount() - 1;

	const auto& settings = course->GetRenderSettings();
	const int resolution = settings.roadResolution; //セグメントあたりの分割数
	const float halfWidth = settings.roadWidth * 0.5f;
	const float height = settings.roadThickness;

	UINT baseIndex = 0;

	//各セグメントごとに頂点を生成
	for (int i = 0; i < segmentCount; i++) {
		const int nextIndex = (i + 1) % course->GetControlPointCount();
		const BezierControlPoint& p0 = course->GetControlPoint(i);
		const BezierControlPoint& p1 = course->GetControlPoint(nextIndex);

		//セグメントの頂点を生成
		for (int j = 0; j < resolution; j++) {
			const float t = static_cast<float>(j) / resolution;

			//ベジェ曲線上の位置と接線を取得
			Vector3 centerPos = course->CalculateBezierPoint(p0, p1, t);
			Vector3 direction = course->CalculateBezierTangent(p0, p1, t);
			Vector3 rightVector = GetPerpendicularVector(direction);

			//道路の左右の端点を計算
			Vector3 leftPos = centerPos - rightVector * halfWidth;
			Vector3 rightPos = centerPos + rightVector * halfWidth;

			//左側の頂点
			VERTEX_3D leftVertex = {};
			leftVertex.position = XMFLOAT3(leftPos.x, leftPos.y + height, leftPos.z);
			leftVertex.normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			leftVertex.diffuse = XMFLOAT4(settings.roadColor.x, settings.roadColor.y, settings.roadColor.z, 1.0f);
			leftVertex.texcoord = XMFLOAT2(t, 0.0f); //UVは適当に設定
			outVertices.push_back(leftVertex);

			//右側の頂点
			VERTEX_3D rightVertex = {};
			rightVertex.position = XMFLOAT3(rightPos.x, rightPos.y + height, rightPos.z);
			rightVertex.normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			rightVertex.diffuse = XMFLOAT4(settings.roadColor.x, settings.roadColor.y, settings.roadColor.z, 1.0f);
			rightVertex.texcoord = XMFLOAT2(t, 1.0f); //UVは適当に設定
			outVertices.push_back(rightVertex);

			//インデックスを設定(2つの三角形で四角形を構成)
			if (j > 0) {
				UINT currentLeft = baseIndex + j * 2;
				UINT currentRight = currentLeft + 1;
				UINT prevLeft = currentLeft - 2;
				UINT prevRight = currentLeft - 1;

				//四角形を2つの三角形に分割
				outIndices.push_back(prevLeft);
				outIndices.push_back(prevRight);
				outIndices.push_back(currentLeft);

				outIndices.push_back(currentLeft);
				outIndices.push_back(prevRight);
				outIndices.push_back(currentRight);
			}
		}

		baseIndex += (resolution + 1) * 2; //次のセグメントの基準インデックスを更新
	}
}

void CourseRenderer::GenerateLineVertices(const RaceCourseManager* course, std::vector<VERTEX_3D>& outVertices, std::vector<UINT>& outIndices) {
	outVertices.clear();
	outIndices.clear();

	const auto& settings = course->GetRenderSettings();
	if (!settings.showCenterLine && !settings.showSideLines) {
		return;
	}

	const int segmentCount = course->IsClosed() ? course->GetControlPointCount() : course->GetControlPointCount() - 1;

	const float halfWidth = settings.roadWidth * 0.5f;
	const float lineHeight = settings.roadThickness + 0.01f; //道路面より少し上に表示
	UINT baseIndex = 0;

	//各セグメントごとに頂点を生成
	for (int i = 0; i < segmentCount; i++) {
		const int nextIndex = (i + 1) % course->GetControlPointCount();
		const BezierControlPoint& p0 = course->GetControlPoint(i);
		const BezierControlPoint& p1 = course->GetControlPoint(nextIndex);

		//セグメントの頂点を生成
		for (int j = 0; j < settings.roadResolution; j++) {
			const float t0 = static_cast<float>(j) / settings.roadResolution;
			const float t1 = static_cast<float>(j + 1) / settings.roadResolution;

			Vector3 pos0 = course->CalculateBezierPoint(p0, p1, t0);
			Vector3 pos1 = course->CalculateBezierPoint(p0, p1, t1);
			Vector3 dir0 = course->CalculateBezierTangent(p0, p1, t0);
			Vector3 dir1 = course->CalculateBezierTangent(p0, p1, t1);

			Vector3 right0 = GetPerpendicularVector(dir0);
			Vector3 right1 = GetPerpendicularVector(dir1);

			//中央線
			if (settings.showCenterLine) {
				CreateLineSegment(pos0, pos1,
					settings.centerLineColor,
					settings.centerLineWidth,
					lineHeight,
					outVertices, outIndices, baseIndex);
			}

			//サイドライン
			if (settings.showSideLines) {
				Vector3 leftPos0 = pos0 - right0 * halfWidth;
				Vector3 leftPos1 = pos1 - right1 * halfWidth;
				Vector3 rightPos0 = pos0 + right0 * halfWidth;
				Vector3 rightPos1 = pos1 + right1 * halfWidth;

				CreateLineSegment(leftPos0, leftPos1,
					settings.sideLineColor,
					settings.sideLineWidth,
					lineHeight,
					outVertices, outIndices, baseIndex);
				CreateLineSegment(rightPos0, rightPos1,
					settings.sideLineColor,
					settings.sideLineWidth,
					lineHeight,
					outVertices, outIndices, baseIndex);
			}
		}
	}
}

void CourseRenderer::GeneratePointVertices(const RaceCourseManager* course, std::vector<VERTEX_3D>& outVertices, std::vector<UINT>& outIndices) {
	outVertices.clear();
	outIndices.clear();

	const auto& settings = course->GetRenderSettings();
	if (!settings.showcontrolPoints) {
		return;
	}

	//簡単な球体代わりに立方体でポイントを表現
	const float radius = settings.pointRadius;
	UINT baseIndex = 0;

	for (int i = 0; i < course->GetControlPointCount(); i++) {
		const BezierControlPoint& point = course->GetControlPoint(i);

		//りっぽたいの頂点を生成
		Vector3 positions[8] = {
			{-radius, -radius, -radius},
			{ radius, -radius, -radius},
			{ radius,  radius, -radius},
			{-radius,  radius, -radius},
			{-radius, -radius,  radius},
			{ radius, -radius,  radius},
			{ radius,  radius,  radius},
			{-radius,  radius,  radius},
		};

		//各頂点を追加
		for (int j = 0; j < 8; j++) {
			VERTEX_3D vertex = {};
			Vector3 worldPos = point.point + positions[j];
			vertex.position = XMFLOAT3(worldPos.x, worldPos.y, worldPos.z);
			vertex.normal = XMFLOAT3(positions[j].x, positions[j].y, positions[j].z); //簡易的に位置を法線として使用
			vertex.diffuse = XMFLOAT4(settings.pointColor.x, settings.pointColor.y, settings.pointColor.z, 1.0f);
			vertex.texcoord = XMFLOAT2(0.0f, 0.0f); //UVは適当に設定
			outVertices.push_back(vertex);
		}

		//立方体のインデックスを追加
		UINT cubeIndices[36] = {
			0,1,2,0,2,3, //前面
			4,7,6,4,6,5, //背面
			0,4,5,0,5,1, //底面
			2,6,7,2,7,3, //上面
			0,3,7,0,7,4, //左側面
			1,5,6,1,6,2  //右側面
		};

		for (int j = 0; j < 36; j++) {
			outIndices.push_back(baseIndex + cubeIndices[j]);
		}
		baseIndex += 8; //次のポイントの基準インデックスを更新
	}
}

void CourseRenderer::DrawRoadSurface() {
	if (m_roadIndexCount == 0) {
		return;
	}

	//マテリアル設定
	MATERIAL material = {};
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.textureEnable = false;
	RENDERER.SetMaterial(material);

	//バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	RENDERER.GetDeviceContext()->IASetVertexBuffers(0, 1, m_roadVertexBuffer.GetAddressOf(), &stride, &offset);
	RENDERER.GetDeviceContext()->IASetIndexBuffer(m_roadIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	//プリミティブトポロジ設定
	RENDERER.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//描画
	RENDERER.GetDeviceContext()->DrawIndexed(m_roadIndexCount, 0, 0);
}

void CourseRenderer::DrawRoadLines() {
	if (m_lineIndexCount == 0) {
		return;
	}

	//マテリアル設定
	MATERIAL material = {};
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.textureEnable = false;
	RENDERER.SetMaterial(material);

	//バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	RENDERER.GetDeviceContext()->IASetVertexBuffers(0, 1, m_lineVertexBuffer.GetAddressOf(), &stride, &offset);
	RENDERER.GetDeviceContext()->IASetIndexBuffer(m_lineIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	//プリミティブトポロジ設定
	RENDERER.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//描画
	RENDERER.GetDeviceContext()->DrawIndexed(m_lineIndexCount, 0, 0);
}

void CourseRenderer::DrawControlPoints() {
	if (m_pointIndexCount == 0) {
		return;
	}
	//マテリアル設定
	MATERIAL material = {};
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.textureEnable = false;
	RENDERER.SetMaterial(material);
	//バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	RENDERER.GetDeviceContext()->IASetVertexBuffers(0, 1, m_pointVertexBuffer.GetAddressOf(), &stride, &offset);
	RENDERER.GetDeviceContext()->IASetIndexBuffer(m_pointIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	//プリミティブトポロジ設定
	RENDERER.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//描画
	RENDERER.GetDeviceContext()->DrawIndexed(m_pointIndexCount, 0, 0);
}

Vector3 CourseRenderer::GetPerpendicularVector(const Vector3& direction) const {
	Vector3 right = direction.Cross(Vector3::UP);
	right.Normalize();
	return right;
}

void CourseRenderer::CreateLineSegment(const Vector3& start, const Vector3& end, const Vector3& color, float width, float height, std::vector<VERTEX_3D>& vertices, std::vector<UINT>& indices, UINT& baseIndex) {
	Vector3 direction = end - start;
	direction.Normalize();
	Vector3 perpendicular = GetPerpendicularVector(direction);
	float halfWidth = width * 0.5f;

	//4つの頂点で四角形を作成
	Vector3 p0 = start - perpendicular * halfWidth; //左下
	Vector3 p1 = start + perpendicular * halfWidth; //右下
	Vector3 p2 = end + perpendicular * halfWidth;   //右上
	Vector3 p3 = end - perpendicular * halfWidth;   //左上

	//頂点データを作成
	VERTEX_3D lineVertices[4] = {};
	lineVertices[0].position = XMFLOAT3(p0.x, p0.y + height, p0.z);
	lineVertices[1].position = XMFLOAT3(p1.x, p1.y + height, p1.z);
	lineVertices[2].position = XMFLOAT3(p2.x, p2.y + height, p2.z);
	lineVertices[3].position = XMFLOAT3(p3.x, p3.y + height, p3.z);

	for (int i = 0; i < 4; i++) {
		lineVertices[i].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
		lineVertices[i].diffuse = XMFLOAT4(color.x, color.y, color.z, 1.0f);
		lineVertices[i].texcoord = XMFLOAT2(0.0f, 0.0f); //UVは適当に設定
	}

	//頂点を追加
	for (int i = 0; i < 4; i++) {
		vertices.push_back(lineVertices[i]);
	}

	//インデックスを追加(2つの三角形で四角形を構成)
	UINT lineindices[6] = {
		0,1,2,
		0,2,3
	};
	for (int i = 0; i < 6; i++) {
		indices.push_back(baseIndex + lineindices[i]);
	}
	baseIndex += 4; //次のセグメントの基準インデックスを更新
}
