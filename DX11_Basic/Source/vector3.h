#pragma once	

#include <cmath>

class Vector3 {
public:
	//メンバ変数
	float x, y, z;

	//コンストラクタ
	Vector3() = default;
	constexpr Vector3(const Vector3& a) : x(a.x), y(a.y), z(a.z) {}
	constexpr Vector3(float nx, float ny, float nz) : x(nx), y(ny), z(nz) {}

	//静的定数
	static constexpr Vector3 Zero() noexcept { return Vector3(0.0f, 0.0f, 0.0f); }
	static constexpr Vector3 One() noexcept { return Vector3(1.0f, 1.0f, 1.0f); }
	static constexpr Vector3 Right() noexcept { return Vector3(1.0f, 0.0f, 0.0f); }
	static constexpr Vector3 Up() noexcept { return Vector3(0.0f, 1.0f, 0.0f); }
	static constexpr Vector3 Forward() noexcept { return Vector3(0.0f, 0.0f, 1.0f); }

	//演算子オーバーロード
	Vector3& operator =(const Vector3& a) noexcept {
		x = a.x; y = a.y; z = a.z;
		return *this;
	}

	bool operator ==(const Vector3& a) const noexcept {
		return x == a.x && y == a.y && z == a.z;
	}

	bool operator !=(const Vector3& a) const noexcept {
		return x != a.x || y != a.y || z != a.z;
	}

	void Reset() { x = y = z = 0.0f; }

	Vector3 operator -() const noexcept { return Vector3(-x, -y, -z); }

	Vector3 operator +(const Vector3& a) const noexcept {
		return Vector3(x + a.x, y + a.y, z + a.z);
	}

	Vector3 operator -(const Vector3& a) const noexcept {
		return Vector3(x - a.x, y - a.y, z - a.z);
	}

	Vector3 operator *(float a) const noexcept {
		return Vector3(x * a, y * a, z * a);
	}

	Vector3 operator /(float a) const noexcept {
		float oneOver = 1.0f / a;

		return Vector3(x * oneOver, y * oneOver, z * oneOver);
	}

	Vector3& operator +=(const Vector3& a) noexcept {
		x += a.x; y += a.y; z += a.z;
		return *this;
	}

	Vector3& operator -=(const Vector3& a) noexcept {
		x -= a.x; y -= a.y; z -= a.z;
		return *this;
	}

	Vector3& operator *=(float a) noexcept {
		x *= a; y *= a; z *= a;
		return *this;
	}

	Vector3& operator /=(float a) noexcept {
		float oneOver = 1.0f / a;
		x *= oneOver; y *= oneOver; z *= oneOver;
		return *this;
	}

	Vector3& normalize() noexcept {
		float magSq = x * x + y * y + z * z;
		if (magSq > 0.0f) {
			float oneOver = 1.0f / sqrt(magSq);
			x *= oneOver;
			y *= oneOver;
			z *= oneOver;
		}
		return *this;
	}

	float length() const noexcept {
		return sqrt(x * x + y * y + z * z);
	}

	float lengthSq() const noexcept {
		return x * x + y * y + z * z;
	}

	float dot(const Vector3& a) const noexcept {
		return x * a.x + y * a.y + z * a.z;
	}

	Vector3 cross(const Vector3& a) const noexcept {
		return Vector3(
			y * a.z - z * a.y,
			z * a.x - x * a.z,
			x * a.y - y * a.x
		);
	}
};
