#pragma once

#include <format>

struct Vector3 final
{
	float X, Y, Z;

	inline Vector3() : X(0.f), Y(0.f), Z(0.f) {}
	inline Vector3(float x, float y, float z) : X(x), Y(y), Z(z) {}
};

template <>
struct std::formatter<Vector3> : std::formatter<std::string> {
	auto format(const Vector3& v, format_context& ctx) const { return formatter<string>::format(std::format("{:.3f}; {:.3f}; {:.3f}", v.X, v.Y, v.Z), ctx); }
};
