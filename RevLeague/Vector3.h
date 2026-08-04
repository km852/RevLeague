#pragma once

struct Vector3 final
{
	float X, Y, Z;

	inline Vector3() : X(0.f), Y(0.f), Z(0.f) {}
	inline Vector3(float x, float y, float z) : X(x), Y(y), Z(z) {}
};
