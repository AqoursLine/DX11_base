#pragma once

#include "gameObject.h"

class LightManager : public GameObject {
public:
	LightManager() = default;
	~LightManager() = default;
protected:
	virtual bool Initialize() override;
	virtual void Finalize() override;
	virtual void Update(double deltaTime) override;
	virtual void Draw() override;
private:
};
