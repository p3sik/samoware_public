
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

	Vector& operator+=(const Vector& other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	Vector& operator-=(const Vector& other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	Vector& operator*=(const Vector& other) {
		x *= other.x;
		y *= other.y;
		z *= other.z;
		return *this;
	}

	Vector& operator/=(const Vector& other) {
		x /= other.x;
		y /= other.y;
		z /= other.z;
		return *this;
	}

	Vector& operator*=(float num) {
		x *= num;
		y *= num;
		z *= num;
		return *this;
	}

	Vector& operator/=(float num) {
		x /= num;
		y /= num;
		z /= num;
		return *this;
	}

	float Length2D() const {
		return std::sqrtf(x * x + y * y);
	}

	float LengthSqr() const {
		return x * x + y * y + z * z;
	}
	
	float Length() const {
		return std::sqrtf(LengthSqr());
	}

	Vector Cross(const Vector& other) const {
		return Vector(y * other.z - z * other.y, x * other.z - z * other.x, x * other.y - y * other.x);
	}

	float Dot(const Vector& other) const {
		return x * other.x + y * other.y + z * other.z;
	}

	float Normalize() {
		float length = Length();
		x /= length + 1e-6f;
		y /= length + 1e-6f;
		z /= length + 1e-6f;
		return length;
	}
};

class __declspec(align(16)) VectorAligned : public Vector {
public:
	using Vector::Vector;

	VectorAligned(const Vector& other) {
		x = other.x;
		y = other.y;
		z = other.z;
	}
};
