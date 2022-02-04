
#pragma once

namespace math {
	constexpr double M_PI = 3.14159265358979323846;
	constexpr float M_PI_F = 3.14159265358979323846f;

	constexpr float deg2rad(float deg) {
		return deg * M_PI_F / 180.f;
	}

	constexpr float rad2deg(float rad) {
		return rad * 180.f / M_PI_F;
	}
}
