#pragma once

#include <random>

class MyRandom {
public:
	// 0以上max未満の整数の乱数を生成
	static int GetInt(int max) {
		if (max <= 0) {
			return 0;
		}
		std::uniform_int_distribution<int> dist(0, max - 1);
		return dist(GetEngine());
	}
	// min以上max以下の整数の乱数を生成
	static int GetInt(int min, int max) {
		// minがmax以上の場合はminを返す
		if (min >= max) {
			return min;
		}

		std::uniform_int_distribution<int> dist(min, max);
		return dist(GetEngine());
	}
	// 0.0以上1.0未満の浮動小数点数の乱数を生成
	static float GetFloat() {
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);
		return dist(GetEngine());
	}
	// min以上max未満の浮動小数点数の乱数を生成
	static float GetFloat(float min, float max) {
		// minがmax以上の場合はminを返す
		if (min >= max) {
			return min;
		}
		std::uniform_real_distribution<float> dist(min, max);
		return dist(GetEngine());
	}

	static std::mt19937& GetEngine() {
		static thread_local std::mt19937 engine(std::random_device {}());
		return engine;
	}

};
