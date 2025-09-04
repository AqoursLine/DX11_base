#include "raceCourseManager.h"
#include "courseRenderer.h"
#include "renderer.h"
#include <algorithm>

RaceCourseManager::RaceCourseManager()
	: m_totalLength(0.0f), m_isClosed(false) {

	m_renderer = std::make_unique<CourseRenderer>();
}

void RaceCourseManager::AddPoint(const Vector3& point, const Vector3& nextHandle, const Vector3& prevHandle) {
	//新しいコントロールポイントを追加
	m_controlPoints.emplace_back(point, nextHandle, prevHandle);

	//コースの長さを再計算
	RecalculateLengths();

}

void RaceCourseManager::ClearCourse() {
	//全てのデータを初期状態にリセット
	m_controlPoints.clear();
	m_segmentLengths.clear();
	m_totalLength = 0.0f;
	m_isClosed = false;
}

void RaceCourseManager::CloseCourse() {
	m_isClosed = true;
	RecalculateLengths();
}

float RaceCourseManager::GetProgressFromPosition(const Vector3& position) const {
	//コースが存在しない場合は0を返す
	if (m_controlPoints.size() < 2 || m_totalLength <= 0.0f) {
		return 0.0f;
	}

	//最近接点を検索
	auto [segmentIndex, t] = FindClosestPointOnCourse(position);

	//セグメント開始位置までの累積距離を計算
	float progressDistance = 0.0f;
	for (int i = 0; i < segmentIndex; ++i) {
		progressDistance += m_segmentLengths[i];
	}

	//セグメント内での距離を追加
	progressDistance += t * m_segmentLengths[segmentIndex];

	//総長で輪って正規化して進行度を計算
	return progressDistance / m_totalLength;
}

CoursePosition RaceCourseManager::GetCoursePosition(const Vector3& position) const {
	CoursePosition coursePos = {};

	//コースが存在しない場合はデフォルト値を返す
	if (m_controlPoints.size() < 2) {
		coursePos.segmentIndex = 0;
		coursePos.t = 0.0f;
		coursePos.totalProgress = 0.0f;
		coursePos.position = Vector3::ZERO;
		coursePos.direction = Vector3::FORWARD;
		return coursePos;
	}

	//最近接点を検索
	auto [segmentIndex, t] = FindClosestPointOnCourse(position);

	//CoursePositionの各フィールドを設定
	coursePos.segmentIndex = segmentIndex;
	coursePos.t = t;
	coursePos.totalProgress = GetProgressFromPosition(position);

	//そのセグメント・tに対応する位置と方向を計算
	const int nextIndex = (segmentIndex + 1) % m_controlPoints.size();
	coursePos.position = CalculateBezierPoint(m_controlPoints[segmentIndex], m_controlPoints[nextIndex], t);
	coursePos.direction = CalculateBezierTangent(m_controlPoints[segmentIndex], m_controlPoints[nextIndex], t);

	return coursePos;
}

Vector3 RaceCourseManager::GetPositionFromProgress(float progress) const {
	//コースが存在しない場合は原点を返す
	if (m_controlPoints.size() < 2 || m_totalLength <= 0.0f) {
		return Vector3::ZERO;
	}

	//進行度を0.0f ~ 1.0fの範囲にクランプ
	progress = std::clamp(progress, 0.0f, 1.0f);
	const float targetDistance = progress * m_totalLength;

	float accumulatedDistance = 0.0f;
	const int segmentCount = m_isClosed ? m_controlPoints.size() : m_controlPoints.size() - 1;

	//目標距離がどのセグメントにあるかを特定
	for (int i = 0; i < segmentCount; i++) {
		const float segmentLength = m_segmentLengths[i];
		//目標距離がこのセグメント内にある場合
		if (accumulatedDistance + segmentLength >= targetDistance) {
			//セグメント内での位置を計算
			const float segmentProgress = (targetDistance - accumulatedDistance) / segmentLength;
			const int nextIndex = (i + 1) % m_controlPoints.size();
			return CalculateBezierPoint(m_controlPoints[i], m_controlPoints[nextIndex], segmentProgress);
		}
		accumulatedDistance += segmentLength;
	}

	//進行度が1.0fの場合、最後のポイントを返す
	return m_controlPoints.back().point;
}

Vector3 RaceCourseManager::GetDirectionFromProgress(float progress) const {
	//コースが存在しない場合は前方ベクトルを返す
	if (m_controlPoints.size() < 2 || m_totalLength <= 0.0f) {
		return Vector3::FORWARD;
	}

	//進行度を0.0f ~ 1.0fの範囲にクランプ
	progress = std::clamp(progress, 0.0f, 1.0f);
	const float targetDistance = progress * m_totalLength;

	float accumulatedDistance = 0.0f;
	const int segmentCount = m_isClosed ? m_controlPoints.size() : m_controlPoints.size() - 1;

	//目標距離がどのセグメントにあるかを特定
	for (int i = 0; i < segmentCount; i++) {
		const float segmentLength = m_segmentLengths[i];
		//目標距離がこのセグメント内にある場合
		if (accumulatedDistance + segmentLength >= targetDistance) {
			//セグメント内での位置を計算
			const float segmentProgress = (targetDistance - accumulatedDistance) / segmentLength;
			const int nextIndex = (i + 1) % m_controlPoints.size();
			return CalculateBezierTangent(m_controlPoints[i], m_controlPoints[nextIndex], segmentProgress);
		}
		accumulatedDistance += segmentLength;
	}

	//進行度が1.0fの場合、最後のセグメントの方向を返す
	return Vector3::FORWARD;
}

const BezierControlPoint& RaceCourseManager::GetControlPoint(int index) const {
	//指定インデックスのコントロールポイントを返す
	return m_controlPoints[index];
}

bool RaceCourseManager::Initialize() {
	AddPoint({ 0.0f, 0.0f, 0.0f }, { 10.0f, 0.0f, 0.0f }, { -10.0f, 0.0f, 0.0f });
	AddPoint({ 20.0f, 0.0f, 0.0f }, { 30.0f, 0.0f, 10.0f }, { 10.0f, 0.0f, -10.0f });
	AddPoint({ 20.0f, 0.0f, 20.0f }, { 10.0f, 0.0f, 30.0f }, { 30.0f, 0.0f, 10.0f });
	AddPoint({ 0.0f, 0.0f, 20.0f }, { -10.0f, 0.0f, 10.0f }, { 10.0f, 0.0f, 30.0f });

	//コースを閉じる
	CloseCourse();

	//描画オブジェクト初期化
	if (!m_renderer->Initialize()) {
		ErrorMessage(L"CourseRendererの初期化に失敗しました。", E_FAIL);
		return false;
	}
	return true;
}

void RaceCourseManager::Finalize() {
}

void RaceCourseManager::Update(double deltaTime) {
}

void RaceCourseManager::Draw() const {
	//描画オブジェクトに描画を委譲
	if (IsVisible() && m_renderer) {
		//ワールド行列を設定
		XMMATRIX worldMatrix = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z) *
			XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z) *
			XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

		RENDERER.SetWorldMatrix(worldMatrix);
		m_renderer->DrawCourse(this);


	}
}

Vector3 RaceCourseManager::CalculateBezierPoint(const BezierControlPoint& p0, const BezierControlPoint& p1, float t) const {
	//4点ベジェ曲線の公式を使用して点を計算
	
	const float u = 1.0f - t;
	const float tt = t * t;
	const float uu = u * u;
	const float uuu = uu * u;
	const float ttt = tt * t;

	//ベジェ曲線の各項を計算
	Vector3 result = p0.point * uuu; // (1-t)^3 * P0
	result += p0.nextHandle * (3 * uu * t); // 3(1-t)^2 * t * P1
	result += p1.prevHandle * (3 * u * tt); // 3(1-t) * t^2 * P2
	result += p1.point * ttt; // t^3 * P3

	return result;
}

Vector3 RaceCourseManager::CalculateBezierTangent(const BezierControlPoint& p0, const BezierControlPoint& p1, float t) const {
	//ベジェ曲線の1次微分を使用して接線ベクトルを計算
	
	const float u = 1.0f - t;
	const float tt = t * t;
	const float uu = u * u;

	//ベジェ曲線の各項の微分を計算
	Vector3 result = (p0.nextHandle - p0.point) * (3 * uu); // 3(1-t)^2 * (P1 - P0)
	result += (p1.prevHandle - p0.nextHandle) * (6 * u * t); // 6(1-t)t * (P2 - P1)
	result += (p1.point - p1.prevHandle) * (3 * tt); // 3t^2 * (P3 - P2)

	//正規化して方向ベクトルに変換
	result.Normalize();
	return result;
}

float RaceCourseManager::CalculateSegmentLength(const BezierControlPoint& p0, const BezierControlPoint& p1) const {
	//セグメントの長さを数値積分で近似
	float length = 0.0f;
	Vector3 previousPoint = p0.point;

	//CURVE_RESOLUTIONに基づいて分割して長さを計算
	for (int i = 1; i <= CURVE_RESOLUTION; ++i) {
		float t = static_cast<float>(i) / CURVE_RESOLUTION;
		Vector3 currentPoint = CalculateBezierPoint(p0, p1, t);
		length += (currentPoint - previousPoint).Length();
		previousPoint = currentPoint;
	}

	return length;
}

void RaceCourseManager::RecalculateLengths() {
	//コントロールポイントが2未満の場合、長さをリセットして終了
	if (m_controlPoints.size() < 2) {
		m_segmentLengths.clear();
		m_totalLength = 0.0f;
		return;
	}

	//セグメント数を決定(閉じたコースの場合はポイント数、開いたコースの場合はポイント数-1)
	const int segmentCount = m_isClosed ? m_controlPoints.size() : m_controlPoints.size() - 1;
	m_segmentLengths.resize(segmentCount);
	m_totalLength = 0.0f;

	//各セグメントの長さを計算
	for (int i = 0; i < segmentCount; i++) {
		//次のポイントのインデックス
		const int nextIndex = (i + 1) % m_controlPoints.size();
		const float segmentLength = CalculateSegmentLength(m_controlPoints[i], m_controlPoints[nextIndex]);
		m_segmentLengths[i] = segmentLength;
		m_totalLength += segmentLength;
	}
}

std::pair<int, float> RaceCourseManager::FindClosestPointOnCourse(const Vector3& position) const {
	float mindistance = FLT_MAX;
	int closestSegment = 0;
	float closestT = 0.0f;

	const int segmentCount = m_isClosed ? m_controlPoints.size() : m_controlPoints.size() - 1;

	//全セグメントを走査して最近接点を特定
	for (int i = 0; i < segmentCount; i++) {
		//このセグメントをCURVE_RESOLUTIONに分割してチェック
		for (int j = 0; j < CURVE_RESOLUTION; j++) {
			const float t = static_cast<float>(j) / CURVE_RESOLUTION;
			const float distance = DistanceToSegment(position, i, t);

			//より近い点が見つかった場合、更新
			if (distance < mindistance) {
				mindistance = distance;
				closestSegment = i;
				closestT = t;
			}
		}
	}

	return { closestSegment, closestT };
}

float RaceCourseManager::DistanceToSegment(const Vector3& position, int segmentIndex, float t) const {
	//指定されたセグメントの指定されたtパラメータでのベジェ曲線上の点を計算
	const int nextIndex = (segmentIndex + 1) % m_controlPoints.size();
	const Vector3 pointOnCurve = CalculateBezierPoint(m_controlPoints[segmentIndex], m_controlPoints[nextIndex], t);

	//指定座標とベジェ曲線上の点との3D距離を計算
	return (position - pointOnCurve).Length();
}
