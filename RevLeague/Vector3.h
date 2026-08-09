#pragma once

#include <format>
#include <cmath>

struct Vector3 final
{
	float X, Y, Z;

	inline Vector3() : X(0.f), Y(0.f), Z(0.f) {}
	inline Vector3(float x, float y, float z) : X(x), Y(y), Z(z) {}

	inline Vector3 operator+(const Vector3& rhs) const { return Vector3(X + rhs.X, Y + rhs.Y, Z + rhs.Z); }
	inline Vector3 operator-(const Vector3& rhs) const { return Vector3(X - rhs.X, Y - rhs.Y, Z - rhs.Z); }
	inline Vector3 operator*(float coeff) const { return Vector3(X * coeff, Y * coeff, Z * coeff); }
	inline Vector3 operator/(float coeff) const { return Vector3(X / coeff, Y / coeff, Z / coeff); }
	inline Vector3 operator-() const { return Vector3(-X, -Y, -Z); }

	inline void RotateAroundY(float angleRad)
	{
		float sinAngle = std::sin(angleRad);
		float cosAngle = std::cos(angleRad);
		float prevX = X, prevZ = Z;
		X = prevX * cosAngle + prevZ * sinAngle;
		Z = prevZ * cosAngle - prevX * sinAngle;
	}

	inline Vector3 RotatedAroundY(float angleRad)
	{
		Vector3 retval = *this;
		retval.RotateAroundY(angleRad);
		return retval;
	}
};

template <>
struct std::formatter<Vector3> : std::formatter<std::string> {
	auto format(const Vector3& v, format_context& ctx) const { return formatter<string>::format(std::format("{:.3f}; {:.3f}; {:.3f}", v.X, v.Y, v.Z), ctx); }
};
