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

// 前方宣言
class Vector2;
class Vector3;
class Vector4;

//=============================================================================
// Vector2 - 軽量版
//=============================================================================
class Vector2 {
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

//=============================================================================
// Vector3 - 軽量版
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

//=============================================================================
// Vector4 - 軽量版
//=============================================================================
class Vector4 {
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
};

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