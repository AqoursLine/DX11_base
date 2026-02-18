#pragma once

#include "gameObject.h"

class MultiButton : public GameObject {
public:
	MultiButton() = default;
	~MultiButton() = default;

	virtual void OnDecide() = 0;

	void SetSelected(bool selected) { m_isSelected = selected; }
	bool IsSelected() const { return m_isSelected; }

protected:
	void Update(double deltaTime) override {
		if (m_isSelected) {
			// ターゲットスケールに向かってスムーズに拡大
			m_scale += (m_targetScale - m_scale) * m_scalingRate * static_cast<float>(deltaTime);
		} else {
			// ターゲットスケールに向かってスムーズに縮小
			m_scale += (m_startScale - m_scale) * m_scalingRate * static_cast<float>(deltaTime);
		}

	}

	void SetTargetScale(const Vector3& targetScale) { m_targetScale = targetScale; }
	void SetStartScale(const Vector3& startScale) { m_startScale = startScale; }
private:
	bool m_isSelected = false;

	Vector3 m_startScale = Vector3(1.0f, 1.0f, 1.0f);
	Vector3 m_targetScale = Vector3(1.0f, 1.0f, 1.0f);

	float m_scalingRate = 5.0f; // スケールの変化速度
};
