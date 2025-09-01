#pragma once

#include "gameObject.h"

class CourseRenderer;

//ベジェ曲線のコントロールポイント構造体
struct BezierControlPoint {
	Vector3 point; //メインポイント(曲線上の点)
	Vector3 nextHandle; //次のポイントへの曲線の制御ハンドル
	Vector3 prevHandle; //前のポイントからの曲線の制御ハンドル

	BezierControlPoint() = default;
	BezierControlPoint(const Vector3& p, const Vector3& next, const Vector3& prev)
		: point(p), nextHandle(next), prevHandle(prev) {
	}
};

//コース上の位置情報を詳細に保持する構造体
struct CoursePosition {
	int segmentIndex; //そのセグメント内にいるか
	float t; //セグメント内の位置(0.0f ~ 1.0f)
	float totalProgress; //コース全体に対する進行度(0.0f ~ 1.0f)
	Vector3 position; //ベジェ曲線上の位置
	Vector3 direction; //その位置での進行方向ベクトル(正規化済み)
};

class RaceCourseManager : public GameObject {
public:
	//描画設定構造体
	struct CourseRenderSettings {
		Vector3 roadColor = { 0.2f, 0.2f, 0.2f }; //コースの色
		Vector3 centerLineColor = { 1.0f, 1.0f, 0.0f }; //センターラインの色
		Vector3 sideLineColor = { 1.0f, 1.0f, 1.0f }; //サイドラインの色
		Vector3 pointColor = { 1.0f, 0.0f, 0.0f }; //コントロールポイントの色
		Vector3 handleColor = { 0.0f, 1.0f, 0.0f }; //ハンドルポイントの色

		float roadWidth = 10.0f; //コースの幅
		float centerLineWidth = 0.2f; //センターラインの幅
		float sideLineWidth = 0.3f; //サイドラインの幅
		float roadThickness = 0.1f; //コースの厚み

		float pointRadius = 0.5f; //コントロールポイントの半径
		float handleRadius = 0.2f; //ハンドルポイントの半径

		bool showCenterLine = true; //センターラインを表示するか
		bool showSideLines = true; //サイドラインを表示するか
		bool showcontrolPoints = false; //コントロールポイントを表示するか
		bool showHandles = false; //ハンドルポイントを表示するか
		
		int roadResolution = 32; //コースの描画解像度(分割数)
	};


	RaceCourseManager();
	~RaceCourseManager() = default;

	//ベジェ曲線のポイント追加
	void AddPoint(const Vector3& point, const Vector3& nextHandle, const Vector3& prevHandle);

	//コースをクリア
	void ClearCourse();

	//コースを閉じる(最初と最後のポイントを接続)
	void CloseCourse();

	//座標からコース上の進行度を取得
	float GetProgressFromPosition(const Vector3& position) const;

	//より詳細な位置情報を取得
	CoursePosition GetCoursePosition(const Vector3& position) const;

	//進行度からコース上の座標を取得
	Vector3 GetPositionFromProgress(float progress) const;

	//進行度からコース上の方向ベクトルを取得
	Vector3 GetDirectionFromProgress(float progress) const;

	//コースの長さを取得
	float GetTotalLength() const { return m_totalLength; }

	//コントロールポイントの数を取得
	int GetControlPointCount() const { return static_cast<int>(m_controlPoints.size()); }

	//指定インデックスのコントロールポイントを取得
	const BezierControlPoint& GetControlPoint(int index) const;

	//コースが閉じているか
	bool IsClosed() const { return m_isClosed; }

	//描画設定を取得
	void SetRenderSettings(const CourseRenderSettings& settings) { m_renderSettings = settings; }
	const CourseRenderSettings& GetRenderSettings() const { return m_renderSettings; }

	//ベジェ曲線計算
	//4点ベジェ曲線上の点を計算(tは0.0f~1.0f)
	Vector3 CalculateBezierPoint(const BezierControlPoint& p0, const BezierControlPoint& p1, float t) const;

	//ベジェ曲線の接線ベクトル(方向)を計算
	Vector3 CalculateBezierTangent(const BezierControlPoint& p0, const BezierControlPoint& p1, float t) const;

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() const override;

private:
	//ベジェ曲線のコントロールポイント配列
	std::vector<BezierControlPoint> m_controlPoints;
	//各セグメントの長さ配列
	std::vector<float> m_segmentLengths;
	//コース全体の長さ
	float m_totalLength = 0.0f;
	//コースが閉じているか
	bool m_isClosed = false;
	//描画設定
	CourseRenderSettings m_renderSettings;

	//描画専用クラス
	std::unique_ptr<CourseRenderer> m_renderer;

	//解像度・制度設定定数
	static constexpr int CURVE_RESOLUTION = 50; //ベジェ曲線の分割数
	static constexpr float MIN_DISTANCE_THRESHOLD = 1.0f; //最小距離閾値

	//2つコントロールポイント間のセグメント長を計算
	float CalculateSegmentLength(const BezierControlPoint& p0, const BezierControlPoint& p1) const;

	//全セグメントの長さを再計算
	void RecalculateLengths();

	//指定座標に最も近いコース上の点を検索
	std::pair<int, float> FindClosestPointOnCourse(const Vector3& position) const;

	//指定座標とセグメント上の特定点との距離を計算
	float DistanceToSegment(const Vector3& position, int segmentIndex, float t) const;

};