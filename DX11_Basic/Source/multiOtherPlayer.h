#pragma once

#include "racingBoat.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class MultiOtherPlayer : public RacingBoat {
public:
	MultiOtherPlayer() = default;
	~MultiOtherPlayer() = default;

	void SetDataFromNetwork(const json& data);
protected:
	bool Initialize() override;
	void Update(double deltaTime) override;

private:

};
