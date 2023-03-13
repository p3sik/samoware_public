
#pragma once

#include <algorithm>
#include <cmath>

#include "vector.h"
#include "math.h"

class Angle {
public:
	float p, y, r;

	Angle() : p(0.f), y(0.f), r(0.f) {}
	Angle(const Angle& other) : p(other.p), y(other.y), r(other.r) {}
	Angle(Angle&& other) : p(std::move(other.p)), y(std::move(other.y)), r(std::move(other.r)) {}
	Angle(float p_, float y_, float r_) : p(p_), y(y_), r(r_) {}

	Angle& operator=(const Angle& other) {
		p = other.p;
		y = other.y;
		r = other.r;

		return *this;
	}

	Angle&& operator+(const Angle& other) const {
		return Angle(p + other.p, y + other.y, r + other.r);
	}

	Angle&& operator-(const Angle& other) const {
		return Angle(p - other.p, y - other.y, r - other.r);
	}

	Angle&& operator*(const Angle& other) const {
		return Angle(p * other.p, y * other.y, r * other.r);
	}

	Angle&& operator/(const Angle& other) const {
		return Angle(p / other.p, y / other.p, r / other.r);
	}

	Angle&& operator*(float num) const {
		return Angle(p * num, y * num, r * num);
	}

	Angle&& operator/(float num) const {
		return Angle(p / num, y / num, r / num);
	}

	Angle& operator+=(const Angle& other) {
		p += other.p;
		y += other.y;
		r += other.r;

		return *this;
	}

	Angle& operator-=(const Angle& other) {
		p -= other.p;
		y -= other.y;
		r -= other.r;

		return *this;
	}

	Angle& operator*=(const Angle& other) {
		p *= other.p;
		y *= other.y;
		r *= other.r;

		return *this;
	}

	Angle& operator/=(const Angle& other) {
		p /= other.p;
		y /= other.y;
		r /= other.r;

		return *this;
	}

	Angle& operator/=(float num) {
		p /= num;
		y /= num;
		r /= num;

		return *this;
	}

	Angle& operator*=(float num) {
		p *= num;
		y *= num;
		r *= num;

		return *this;
	}

	void Normalize() noexcept {
		p = std::clamp(p, -89.f, 89.f);
		y = Normalize180(y);
		r = 0.f;
	}

	static float Normalize180(float ang) noexcept {
		ang = fmodf(ang + 180, 360);

		if (ang < 0)
			ang += 360;

		return ang - 180;
	}

	static Angle FromVector(const Vector& vec) {
		Angle ang;

		if (vec.y == 0.f && vec.x == 0.f) {
			ang.y = 0;

			if (vec.z > 0)
				ang.p = 270;
			else
				ang.p = 90;
		} else {
			ang.y = (std::atan2f(vec.y, vec.x) * 180.f / math::M_PI_F);
			if (ang.y < 0)
				ang.y += 360;

			ang.p = (std::atan2f(-vec.z, vec.Length2D()) * 180.f / math::M_PI_F);
			if (ang.p < 0)
				ang.p += 360;
		}

		return ang;
	}

	Vector Forward() const {
		float radp = math::deg2rad(p);
		float rady = math::deg2rad(y);

		float sp = std::sinf(radp), cp = std::cosf(radp);
		float sy = std::sinf(rady), cy = std::cosf(rady);

		return Vector(cp * cy, cp * sy, -sp);
	}

	Vector Right() const {
		float radp = math::deg2rad(p);
		float rady = math::deg2rad(y);
		float radr = math::deg2rad(r);

		float sp = std::sinf(radp), cp = std::cosf(radp);
		float sy = std::sinf(rady), cy = std::cosf(rady);
		float sr = std::sinf(radr), cr = std::cosf(radr);

		return Vector(-1.f * sr * sp * cy + -1.f * cr * -sy,
					  -1.f * sr * sp * sy + -1.f * cr * cy,
					  -1.f * sr * cp);
	}

	Vector Up() const {
		float radp = math::deg2rad(p);
		float rady = math::deg2rad(y);
		float radr = math::deg2rad(r);

		float sp = std::sinf(radp), cp = std::cosf(radp);
		float sy = std::sinf(rady), cy = std::cosf(rady);
		float sr = std::sinf(radr), cr = std::cosf(radr);

		return Vector(cr * sp * cy + -sr * -sy,
					  cr * sp * sy + -sr * cy,
					  cr * cp);
	}

	void Vectors(Vector& outForward, Vector& outRight, Vector& outUp) const {
		outForward = Forward();
		outRight = Right();
		outUp = Up();
	}
};
