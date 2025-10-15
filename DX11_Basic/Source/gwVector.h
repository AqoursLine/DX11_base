#pragma once

// 最小限のインクルード
#ifdef __has_include
#if __has_include(<cmath>)
#include <cmath>
#endif
#if __has_include(<bit>) && __cplusplus >= 202002L
#include <bit>
#define VECTOR_HAS_BIT_CAST 1
#endif
#else
#include <cmath>
#endif

#include <cstdint>
#include <type_traits>

// 前方宣言
class Vector2;
class Vector3;
class Vector4;

//=============================================================================
// Vector2
//=============================================================================
class alignas(8) Vector2 {
public:
	float x, y;

	// デフォルトコンストラクタ（ゼロ初期化）
	constexpr Vector2() noexcept : x(0.0f), y(0.0f) {}
	// 成分指定コンストラクタ
	constexpr Vector2(float newX, float newY) noexcept : x(newX), y(newY) {}
	constexpr Vector2(const Vector2&) noexcept = default;
	constexpr Vector2& operator=(const Vector2&) noexcept = default;

	// 静的定数（クラス外で定義）
	static const Vector2 ZERO;
	static const Vector2 ONE;
	static const Vector2 RIGHT;
	static const Vector2 UP;

	// ベクトル加算
	constexpr Vector2 operator+(const Vector2& vector) const noexcept {
		return Vector2(x + vector.x, y + vector.y);
	}

	// ベクトル減算
	constexpr Vector2 operator-(const Vector2& vector) const noexcept {
		return Vector2(x - vector.x, y - vector.y);
	}

	// スカラー倍算
	constexpr Vector2 operator*(float scalar) const noexcept {
		return Vector2(x * scalar, y * scalar);
	}

	// スカラー除算
	constexpr Vector2 operator/(float scalar) const noexcept {
		const float inverseScalar = 1.0f / scalar;
		return Vector2(x * inverseScalar, y * inverseScalar);
	}

	// 符号反転
	constexpr Vector2 operator-() const noexcept {
		return Vector2(-x, -y);
	}

	// 加算代入
	constexpr Vector2& operator+=(const Vector2& vector) noexcept {
		x += vector.x; y += vector.y; return *this;
	}

	// 減算代入
	constexpr Vector2& operator-=(const Vector2& vector) noexcept {
		x -= vector.x; y -= vector.y; return *this;
	}

	// スカラー倍算代入
	constexpr Vector2& operator*=(float scalar) noexcept {
		x *= scalar; y *= scalar; return *this;
	}

	// スカラー除算代入
	constexpr Vector2& operator/=(float scalar) noexcept {
		const float inverseScalar = 1.0f / scalar;
		x *= inverseScalar; y *= inverseScalar; return *this;
	}

	// 等価比較
	constexpr bool operator==(const Vector2&) const noexcept = default;

	// 内積
	constexpr float Dot(const Vector2& vector) const noexcept {
		return x * vector.x + y * vector.y;
	}

	// 2D外積（スカラー値を返す）
	constexpr float Cross(const Vector2& vector) const noexcept {
		return x * vector.y - y * vector.x;
	}

	// 長さの2乗
	constexpr float LengthSquared() const noexcept {
		return x * x + y * y;
	}

	// 長さ
	float Length() const noexcept {
		return std::sqrt(LengthSquared());
	}

	// 正規化（自身を変更）
	Vector2& Normalize() noexcept {
		const float vectorLength = Length();
		if (vectorLength > 0.0f) {
			const float inverseLength = 1.0f / vectorLength;
			x *= inverseLength; y *= inverseLength;
		}
		return *this;
	}

	// ゼロベクトルに設定
	void Reset() noexcept {
		x = y = 0.0f;
	}

	// データポインタ取得
	constexpr float* Data() noexcept { return &x; }
	constexpr const float* Data() const noexcept { return &x; }
};

//Vector2のサイズチェック
static_assert(sizeof(Vector2) == 8, "Vector2 size is not 8 bytes");
static_assert(alignof(Vector2) == 8, "Vector2 alignment is not 8 bytes");

//=============================================================================
// Vector3
//=============================================================================
class Vector3 {
public:
	float x, y, z;

	// デフォルトコンストラクタ（ゼロ初期化）
	constexpr Vector3() noexcept : x(0.0f), y(0.0f), z(0.0f) {}
	// 成分指定コンストラクタ
	constexpr Vector3(float newX, float newY, float newZ) noexcept : x(newX), y(newY), z(newZ) {}
	constexpr Vector3(const Vector3&) noexcept = default;
	constexpr Vector3& operator=(const Vector3&) noexcept = default;

	// Vector2からの変換コンストラクタ
	constexpr Vector3(const Vector2& vector2, float newZ = 0.0f) noexcept : x(vector2.x), y(vector2.y), z(newZ) {}

	// 静的定数（クラス外で定義）
	static const Vector3 ZERO;
	static const Vector3 ONE;
	static const Vector3 RIGHT;
	static const Vector3 UP;
	static const Vector3 FORWARD;

	// ベクトル加算
	constexpr Vector3 operator+(const Vector3& vector) const noexcept {
		return Vector3(x + vector.x, y + vector.y, z + vector.z);
	}

	// ベクトル減算
	constexpr Vector3 operator-(const Vector3& vector) const noexcept {
		return Vector3(x - vector.x, y - vector.y, z - vector.z);
	}

	// スカラー倍算
	constexpr Vector3 operator*(float scalar) const noexcept {
		return Vector3(x * scalar, y * scalar, z * scalar);
	}

	// スカラー除算
	constexpr Vector3 operator/(float scalar) const noexcept {
		const float inverseScalar = 1.0f / scalar;
		return Vector3(x * inverseScalar, y * inverseScalar, z * inverseScalar);
	}

	// 符号反転
	constexpr Vector3 operator-() const noexcept {
		return Vector3(-x, -y, -z);
	}

	// 加算代入
	constexpr Vector3& operator+=(const Vector3& vector) noexcept {
		x += vector.x; y += vector.y; z += vector.z; return *this;
	}

	// 減算代入
	constexpr Vector3& operator-=(const Vector3& vector) noexcept {
		x -= vector.x; y -= vector.y; z -= vector.z; return *this;
	}

	// スカラー倍算代入
	constexpr Vector3& operator*=(float scalar) noexcept {
		x *= scalar; y *= scalar; z *= scalar; return *this;
	}

	// スカラー除算代入
	constexpr Vector3& operator/=(float scalar) noexcept {
		const float inverseScalar = 1.0f / scalar;
		x *= inverseScalar; y *= inverseScalar; z *= inverseScalar; return *this;
	}

	// 等価比較
	constexpr bool operator==(const Vector3&) const noexcept = default;

	// 内積
	constexpr float Dot(const Vector3& vector) const noexcept {
		return x * vector.x + y * vector.y + z * vector.z;
	}

	// 外積
	constexpr Vector3 Cross(const Vector3& vector) const noexcept {
		return Vector3(
			y * vector.z - z * vector.y,
			z * vector.x - x * vector.z,
			x * vector.y - y * vector.x
		);
	}

	// 長さの2乗
	constexpr float LengthSquared() const noexcept {
		return x * x + y * y + z * z;
	}

	// 長さ
	float Length() const noexcept {
		return std::sqrt(LengthSquared());
	}

	// 正規化（自身を変更）
	Vector3& Normalize() noexcept {
		const float vectorLength = Length();
		if (vectorLength > 0.0f) {
			const float inverseLength = 1.0f / vectorLength;
			x *= inverseLength; y *= inverseLength; z *= inverseLength;
		}
		return *this;
	}

	// ゼロベクトルに設定
	void Reset() noexcept {
		x = y = z = 0.0f;
	}

	// データポインタ取得
	constexpr float* Data() noexcept { return &x; }
	constexpr const float* Data() const noexcept { return &x; }

	// Vector2への変換（XY成分のみ）
	constexpr Vector2 XY() const noexcept { return Vector2(x, y); }
};

//Vector3のサイズチェック
static_assert(sizeof(Vector3) == 12, "Vector3 size is not 12 bytes");
static_assert(alignof(Vector3) == 4, "Vector3 alignment is not 4 bytes");

//=============================================================================
// Vector4
//=============================================================================
class alignas(16) Vector4 {
public:
	float x, y, z, w;

	// デフォルトコンストラクタ（ゼロ初期化）
	constexpr Vector4() noexcept : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
	// 成分指定コンストラクタ
	constexpr Vector4(float newX, float newY, float newZ, float newW) noexcept : x(newX), y(newY), z(newZ), w(newW) {}
	constexpr Vector4(const Vector4&) noexcept = default;
	constexpr Vector4& operator=(const Vector4&) noexcept = default;

	// Vector3からの変換コンストラクタ
	constexpr Vector4(const Vector3& vector3, float newW = 0.0f) noexcept : x(vector3.x), y(vector3.y), z(vector3.z), w(newW) {}
	// Vector2からの変換コンストラクタ
	constexpr Vector4(const Vector2& vector2, float newZ = 0.0f, float newW = 0.0f) noexcept : x(vector2.x), y(vector2.y), z(newZ), w(newW) {}

	// 静的定数（クラス外で定義）
	static const Vector4 ZERO;
	static const Vector4 ONE;
	static const Vector4 IDENTITY; // (0,0,0,1)

	// ベクトル加算
	constexpr Vector4 operator+(const Vector4& vector) const noexcept {
		return Vector4(x + vector.x, y + vector.y, z + vector.z, w + vector.w);
	}

	// ベクトル減算
	constexpr Vector4 operator-(const Vector4& vector) const noexcept {
		return Vector4(x - vector.x, y - vector.y, z - vector.z, w - vector.w);
	}

	// スカラー倍算
	constexpr Vector4 operator*(float scalar) const noexcept {
		return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
	}

	//クォータニオン乗算
	constexpr Vector4 operator*(const Vector4& q) const noexcept {
		return Vector4(
			w * q.x + x * q.w + y * q.z - z * q.y,
			w * q.y - x * q.z + y * q.w + z * q.x,
			w * q.z + x * q.y - y * q.x + z * q.w,
			w * q.w - x * q.x - y * q.y - z * q.z
		);
	}

	// スカラー除算
	constexpr Vector4 operator/(float scalar) const noexcept {
		const float inverseScalar = 1.0f / scalar;
		return Vector4(x * inverseScalar, y * inverseScalar, z * inverseScalar, w * inverseScalar);
	}

	// 符号反転
	constexpr Vector4 operator-() const noexcept {
		return Vector4(-x, -y, -z, -w);
	}

	// 加算代入
	constexpr Vector4& operator+=(const Vector4& vector) noexcept {
		x += vector.x; y += vector.y; z += vector.z; w += vector.w; return *this;
	}

	// 減算代入
	constexpr Vector4& operator-=(const Vector4& vector) noexcept {
		x -= vector.x; y -= vector.y; z -= vector.z; w -= vector.w; return *this;
	}

	// スカラー倍算代入
	constexpr Vector4& operator*=(float scalar) noexcept {
		x *= scalar; y *= scalar; z *= scalar; w *= scalar; return *this;
	}

	// スカラー除算代入
	constexpr Vector4& operator/=(float scalar) noexcept {
		const float inverseScalar = 1.0f / scalar;
		x *= inverseScalar; y *= inverseScalar; z *= inverseScalar; w *= inverseScalar; return *this;
	}

	// 等価比較
	constexpr bool operator==(const Vector4&) const noexcept = default;

	// 内積
	constexpr float Dot(const Vector4& vector) const noexcept {
		return x * vector.x + y * vector.y + z * vector.z + w * vector.w;
	}

	// 長さの2乗
	constexpr float LengthSquared() const noexcept {
		return x * x + y * y + z * z + w * w;
	}

	// 長さ
	float Length() const noexcept {
		return std::sqrt(LengthSquared());
	}

	// 正規化（自身を変更）
	Vector4& Normalize() noexcept {
		const float vectorLength = Length();
		if (vectorLength > 0.0f) {
			const float inverseLength = 1.0f / vectorLength;
			x *= inverseLength; y *= inverseLength; z *= inverseLength; w *= inverseLength;
		}
		return *this;
	}

	// ゼロベクトルに設定
	void Reset() noexcept {
		x = y = z = w = 0.0f;
	}

	// データポインタ取得
	constexpr float* Data() noexcept { return &x; }
	constexpr const float* Data() const noexcept { return &x; }

	// Vector3への変換（XYZ成分のみ）
	constexpr Vector3 XYZ() const noexcept { return Vector3(x, y, z); }
	// Vector2への変換（XY成分のみ）
	constexpr Vector2 XY() const noexcept { return Vector2(x, y); }

	//=============================================================================
	// クォータニオンとして機能
	//=============================================================================

	//クォータニオンの共役
	constexpr Vector4 Conjugate() const noexcept {
		return Vector4(-x, -y, -z, w);
	}

	//クォータニオンの逆数
	Vector4 Inverse() const noexcept {
		const float lenSq = LengthSquared();
		if (lenSq > 0.0f) {
			const float invLenSq = 1.0f / lenSq;
			return Conjugate() * invLenSq;
		}
		return Vector4(0.0f, 0.0f, 0.0f, 0.0f); // 長さがゼロの場合はゼロクォータニオンを返す
	}

	//Vector3をクォータニオンとして回転
	Vector3 RotateVector(const Vector3& vector) const noexcept {
		//v' = q * v * q^-1
		//最適化
		const Vector3 qVec(x, y, z);
		const Vector3 uv = qVec.Cross(vector);
		const Vector3 uuv = qVec.Cross(uv);
		return vector + (uv * w + uuv) * 2.0f;
	}

	//軸と角度からクォータニオンを作成
	static Vector4 FromAxisAngle(const Vector3& axis, float angleRadians) noexcept {
		const float halfAngle = angleRadians * 0.5f;
		const float sinHalf = std::sin(halfAngle);
		const float cosHalf = std::cos(halfAngle);

		Vector3 normalizedAxis = axis;
		normalizedAxis.Normalize();

		return Vector4(
			normalizedAxis.x * sinHalf,
			normalizedAxis.y * sinHalf,
			normalizedAxis.z * sinHalf,
			cosHalf
		);
	}

	//オイラー角からクォータニオンを作成（YXZ順）
	static Vector4 FromEuler(float yaw, float pitch, float roll) noexcept {
		const float halfYaw = yaw * 0.5f;
		const float halfPitch = pitch * 0.5f;
		const float halfRoll = roll * 0.5f;
		const float cy = std::cos(halfYaw);
		const float sy = std::sin(halfYaw);
		const float cp = std::cos(halfPitch);
		const float sp = std::sin(halfPitch);
		const float cr = std::cos(halfRoll);
		const float sr = std::sin(halfRoll);
		return Vector4(
			sr * cp * cy - cr * sp * sy, // x
			cr * sp * cy + sr * cp * sy, // y
			cr * cp * sy - sr * sp * cy, // z
			cr * cp * cy + sr * sp * sy  // w
		);
	}

	//クォータニオンをオイラー角に変換（XYZ順）
	Vector3 ToEuler() const noexcept {
		//ピッチ（X軸回りの回転）
		const float sinPitch = 2.0f * (w * x + y * z);
		const float pitch = (std::abs(sinPitch) >= 1.0f) ?
			std::copysign(1.57079632679f, sinPitch) : // 90度または-90度
			std::asin(sinPitch);

		//ヨー（Y軸回りの回転）
		const float yaw = std::atan2(2.0f * (w * y + x * z), 1.0f - 2.0f * (y * y + z * z));

		//ロール（Z軸回りの回転）
		const float roll = std::atan2(2.0f * (w * z + x * y), 1.0f - 2.0f * (x * x + z * z));

		return Vector3(pitch, yaw, roll);
	}

	//2つのクォータニオン間の球面線形補間（Slerp）
	static Vector4 Slerp(const Vector4& from, const Vector4& to, float t) noexcept {
		//tは0.0fから1.0fの範囲
		float cosTheta = from.Dot(to);
		Vector4 target = to;
		//クォータニオンが反対方向を向いている場合、補間を短くするために反転
		if (cosTheta < 0.0f) {
			target = -to;
			cosTheta = -cosTheta;
		}
		//線形補間で十分近い場合
		if (cosTheta > 0.9995f) {
			Vector4 result = from + (target - from) * t;
			result.Normalize();
			return result;
		}
		//球面線形補間
		const float angle = std::acos(cosTheta);
		const float sinAngle = std::sin(angle);
		const float invSinAngle = 1.0f / sinAngle;
		const float factorFrom = std::sin((1.0f - t) * angle) * invSinAngle;
		const float factorTo = std::sin(t * angle) * invSinAngle;
		return from * factorFrom + target * factorTo;
	}

	//2つのベクトル間の回転を表すクォータニオンを作成
	static Vector4 FromToRotation(const Vector3& from, const Vector3& to) noexcept {
		Vector3 f = from;
		Vector3 t = to;
		f.Normalize();
		t.Normalize();
		const float cosTheta = f.Dot(t);
		// ベクトルがほぼ同じ方向を向いている場合
		if (cosTheta > 0.9999f) {
			return Vector4(0.0f, 0.0f, 0.0f, 1.0f); // 単位クォータニオン
		}
		// ベクトルがほぼ反対方向を向いている場合
		if (cosTheta < -0.9999f) {
			// 適当な直交ベクトルを見つける
			Vector3 axis = Vector3::RIGHT.Cross(f);
			if (axis.LengthSquared() < 0.0001f) { // fがX軸に平行な場合
				axis = Vector3::UP.Cross(f);
			}
			axis.Normalize();
			return FromAxisAngle(axis, 3.14159265359f); // 180度回転
		}
		// 通常のケース
		const Vector3 axis = f.Cross(t);
		const float s = std::sqrt((1.0f + cosTheta) * 2.0f);
		const float invS = 1.0f / s;
		return Vector4(axis.x * invS, axis.y * invS, axis.z * invS, s * 0.5f).Normalize();
	}

	//指定された方向を向くクォータニオンを作成
	static Vector4 LookRotation(const Vector3& forward, const Vector3& up = Vector3::UP) noexcept {
		Vector3 f = forward;
		f.Normalize();
		Vector3 r = up.Cross(f);
		r.Normalize();
		Vector3 u = f.Cross(r);
		// 回転行列からクォータニオンを計算
		const float trace = r.x + u.y + f.z;
		if (trace > 0.0f) {
			const float s = std::sqrt(trace + 1.0f) * 2.0f;
			const float invS = 1.0f / s;
			return Vector4(
				(u.z - f.y) * invS,
				(f.x - r.z) * invS,
				(r.y - u.x) * invS,
				s * 0.25f
			);
		} else if ((r.x > u.y) && (r.x > f.z)) {
			const float s = std::sqrt(1.0f + r.x - u.y - f.z) * 2.0f;
			const float invS = 1.0f / s;
			return Vector4(
				0.25f * s,
				(u.x + r.y) * invS,
				(f.x + r.z) * invS,
				(u.z - f.y) * invS
			);
		} else if (u.y > f.z) {
			const float s = std::sqrt(1.0f + u.y - r.x - f.z) * 2.0f;
			const float invS = 1.0f / s;
			return Vector4(
				(r.x + u.y) * invS,
				0.25f * s,
				(f.y + u.z) * invS,
				(f.x - r.z) * invS
			);
		} else {
			const float s = std::sqrt(1.0f + f.z - r.x - u.y) * 2.0f;
			const float invS = 1.0f / s;
			return Vector4(
				(r.z + f.x) * invS,
				(u.z + f.y) * invS,
				0.25f * s,
				(r.y - u.x) * invS
			);
		}
	}
};

//Vector4のサイズチェック
static_assert(sizeof(Vector4) == 16, "Vector4 size is not 16 bytes");
static_assert(alignof(Vector4) == 16, "Vector4 alignment is not 16 bytes");

//=============================================================================
// 静的定数の定義（全クラス定義後）
//=============================================================================

// Vector2 静的定数
inline const Vector2 Vector2::ZERO = Vector2(0.0f, 0.0f);
inline const Vector2 Vector2::ONE = Vector2(1.0f, 1.0f);
inline const Vector2 Vector2::RIGHT = Vector2(1.0f, 0.0f);
inline const Vector2 Vector2::UP = Vector2(0.0f, 1.0f);

// Vector3 静的定数
inline const Vector3 Vector3::ZERO = Vector3(0.0f, 0.0f, 0.0f);
inline const Vector3 Vector3::ONE = Vector3(1.0f, 1.0f, 1.0f);
inline const Vector3 Vector3::RIGHT = Vector3(1.0f, 0.0f, 0.0f);
inline const Vector3 Vector3::UP = Vector3(0.0f, 1.0f, 0.0f);
inline const Vector3 Vector3::FORWARD = Vector3(0.0f, 0.0f, 1.0f);

// Vector4 静的定数
inline const Vector4 Vector4::ZERO = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
inline const Vector4 Vector4::ONE = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
inline const Vector4 Vector4::IDENTITY = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
