#pragma once

#include "gameObject.h"

class SkyDome : public GameObject {
public:
	SkyDome() = default;
	~SkyDome() = default;

protected:
	virtual bool Initialize() override;
	virtual void Finalize() override;
	virtual void Update(double deltaTime) override;
	virtual void Draw() const override;

private:
	class Model* m_model = nullptr;

};
