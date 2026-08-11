#pragma once

#include <format>
#include <cmath>

#include "NvLib.h"

struct Vector3 final
{
	float X, Y, Z;

	inline Vector3() : X(0.f), Y(0.f), Z(0.f) {}
	inline Vector3(float x, float y, float z) : X(x), Y(y), Z(z) {}

	inline Vector3 operator+(const Vector3& rhs) const { return Vector3(X + rhs.X, Y + rhs.Y, Z + rhs.Z); }
	inline Vector3 operator-(const Vector3& rhs) const { return Vector3(X - rhs.X, Y - rhs.Y, Z - rhs.Z); }
	inline Vector3 operator*(float rhs) const { return Vector3(X * rhs, Y * rhs, Z * rhs); }
	inline Vector3 operator/(float rhs) const { return Vector3(X / rhs, Y / rhs, Z / rhs); }
	inline Vector3 operator-() const { return Vector3(-X, -Y, -Z); }

	inline Vector3& operator+=(const Vector3& rhs) { X += rhs.X; Y += rhs.Y; Z += rhs.Z; return *this; }
	inline Vector3& operator-=(const Vector3& rhs) { X -= rhs.X; Y -= rhs.Y; Z -= rhs.Z; return *this; }
	inline Vector3& operator*=(const float rhs) { X *= rhs; Y *= rhs; Z *= rhs; return *this; }
	inline Vector3& operator/=(const float rhs) { X /= rhs; Y /= rhs; Z /= rhs; return *this; }

	inline double Length() const { return std::sqrt(SqrLength()); }
	inline double SqrLength() const { return (double)X * X + (double)Y * Y + (double)Z * Z; }
	inline double LengthXZ() const { return std::sqrt(SqrLengthXZ()); }
	inline double SqrLengthXZ() const { return (double)X * X + (double)Z * Z; }

	inline void RotateAroundY(float angleRad)
	{
		float sinAngle = std::sin(angleRad);
		float cosAngle = std::cos(angleRad);
		float prevX = X, prevZ = Z;
		X = prevX * cosAngle + prevZ * sinAngle;
		Z = prevZ * cosAngle - prevX * sinAngle;
	}

	[[nodiscard]] inline Vector3 RotatedAroundY(float angleRad) const
	{
		Vector3 retval = *this;
		retval.RotateAroundY(angleRad);
		return retval;
	}

	inline void Normalize()
	{
		float length = (float)this->Length();
		if (length != 0.0)
		{
			X /= length;
			Y /= length;
			Z /= length;
		}
	}

	[[nodiscard]] inline Vector3 Normalized() const
	{
		Vector3 retval = *this;
		retval.Normalize();
		return retval;
	}

	inline void NormalizeXZ()
	{
		Y = 0.0f;
		Normalize();
	}

	[[nodiscard]] inline Vector3 NormalizedXZ()
	{
		Vector3 retval = *this;
		retval.NormalizeXZ();
		return retval;
	}

	Vector3 TrimToLength(float len) const;

	inline static bool AlmostEqualXZ(const Vector3& v1, const Vector3& v2, float epsilon) { return std::abs(v1.X - v2.X) <= epsilon && std::abs(v1.Z - v2.Z) <= epsilon; }
};

template <>
struct std::formatter<Vector3> : std::formatter<std::string> {
	auto format(const Vector3& v, format_context& ctx) const { return formatter<string>::format(std::format("{:.3f}; {:.3f}; {:.3f}", v.X, v.Y, v.Z), ctx); }
};

template<> inline void NvBinaryStreamWrite::Write(const Vector3& val) { Write<float>(val.X); Write<float>(val.Y); Write<float>(val.Z); }
