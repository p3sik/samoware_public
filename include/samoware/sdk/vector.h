
#pragma once

#include <cmath>

class Vector {
public:
	float x, y, z;

	Vector() : x(0.f), y(0.f), z(0.f) {}
	Vector(const Vector& other) : x(other.x), y(other.y), z(other.z) {}
	Vector(Vector&& other) : x(std::move(other.x)), y(std::move(other.y)), z(std::move(other.z)) {}
	Vector(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

	Vector& operator=(const Vector& other) {
		x = other.x;
		y = other.y;
		z = other.z;

		return *this;
	}

	Vector&& operator+(const Vector& other) const {
		return Vector(x + other.x, y + other.y, z + other.z);
	}

	Vector&& operator-(const Vector& other) const {
		return Vector(x - other.x, y - other.y, z - other.z);
	}

	Vector&& operator*(const Vector& other) const {
		return Vector(x * other.x, y * other.y, z * other.z);
	}

	Vector&& operator/(const Vector& other) const {
		return Vector(x / other.x, y / other.y, z / other.z);
	}

	Vector&& operator*(float num) const {
		return Vector(x * num, y * num, z * num);
	}

	Vector&& operator/(float num) const {
		return Vector(x / num, y / num, z / num);
	}

	float Length2D() const {
		return std::sqrtf(x * x + y * y);
	}
};
