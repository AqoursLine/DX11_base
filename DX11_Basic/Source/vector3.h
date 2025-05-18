#pragma once	

#include <math.h>

class Vector3 {
public:
	float x, y, z;

	Vector3() = default;
	Vector3(const Vector3& a) : x(a.x), y(a.y), z(a.z) {}
	Vector3(float nx, float ny, float nz) : x(nx), y(ny), z(nz) {}

	Vector3& operator =(const Vector3& a) {
		x = a.x; y = a.y; z = a.z;
		return *this;
	}

	bool operator ==(const Vector3& a) const {
		return x == a.z && y == a.y && z == a.z;
	}

	bool operator !=(const Vector3& a) const {
		return x != a.x || y != a.y || z != a.z;
	}

	void zero() { x = y = z = 0.0f; }

	Vector3 operator -() const { return Vector3(-x, -y, z); }

	Vector3 operator +(const Vector3& a) const {
		return Vector3(x + a.x, y + a.y, z + a.z);
	}

	Vector3 operator -(const Vector3& a) const {
		return Vector3(x - a.x, y - a.y, z - a.z);
	}

	Vector3 operator *(float a) const {
		return Vector3(x * a, y * a, z * a);
	}

	Vector3 operator /(float a) const {
		float oneOver = 1.0f / a;

		return Vector3(x * oneOver, y * oneOver, z * oneOver);
	}

	Vector3& operator +=(const Vector3& a) {
		x += a.x; y += a.y; z += a.z;
		return *this;
	}

	Vector3& operator -=(const Vector3& a) {
		x -= a.x; y -= a.y; z -= a.z;
		return *this;
	}

	Vector3& operator *=(float a) {
		x *= a; y *= a; x *= a;
		return *this;
	}

	Vector3& operator /=(float a) {
		float oneOver = 1.0f / a;
		x *= oneOver; y *= oneOver; z *= oneOver;
		return *this;
	}

	void normalize() {
		float magSq = x * x + y * y + z * z;
		if (magSq > 0.0f) {
			float oneOver = 1.0f / sqrt(magSq);
			x *= oneOver;
			y *= oneOver;
			z *= oneOver;
		}
	}
};
