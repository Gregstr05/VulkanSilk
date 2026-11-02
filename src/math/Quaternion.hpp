//
// Created by Gregstr on 02/11/2026.
//

#pragma once
#include <cmath>
#include <cstddef>
#include <limits>

#include "Matrix.hpp"
#include "Vector.hpp"

namespace silk::math {

template <typename T>
struct Quaternion {
	T x{}, y{}, z{};
	T w{1};

	Quaternion() = default;
	constexpr Quaternion(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

	static constexpr Quaternion Identity()
	{
		return Quaternion();
	}

	[[nodiscard]] friend constexpr bool operator==(const Quaternion& lhs, const Quaternion& rhs)
	{
		return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
	}

	// Hamilton product: applying (a * b) to v rotates by b first, then by a
	constexpr Quaternion operator*(const Quaternion& other) const
	{
		return Quaternion(
			w * other.x + x * other.w + y * other.z - z * other.y,
			w * other.y - x * other.z + y * other.w + z * other.x,
			w * other.z + x * other.y - y * other.x + z * other.w,
			w * other.w - x * other.x - y * other.y - z * other.z);
	}

	constexpr Quaternion& operator*=(const Quaternion& other)
	{
		*this = *this * other;
		return *this;
	}

	constexpr Quaternion operator-() const
	{
		return Quaternion(-x, -y, -z, -w);
	}

	[[nodiscard]] constexpr Quaternion Conjugate() const
	{
		return Quaternion(-x, -y, -z, w);
	}

	[[nodiscard]] constexpr T Norm() const
	{
		return std::sqrt(x * x + y * y + z * z + w * w);
	}

	[[nodiscard]] Quaternion Normalized() const
	{
		T n = Norm();
		return Quaternion(x / n, y / n, z / n, w / n);
	}

	[[nodiscard]] constexpr Quaternion Inverse() const
	{
		T normSq = x * x + y * y + z * z + w * w;
		return Quaternion(-x / normSq, -y / normSq, -z / normSq, w / normSq);
	}

	// v' = q * v * q^-1
	[[nodiscard]] constexpr Vector<T, 3> Rotate(const Vector<T, 3>& v) const
	{
		Vector<T, 3> axis(x, y, z);
		Vector<T, 3> t = axis.Cross(v) * T(2);
		return v + t * w + axis.Cross(t);
	}

	// axis must be normalized
	static Quaternion FromAxisAngle(const Vector<T, 3>& axis, T angle)
	{
		T half = angle / T(2);
		T s = std::sin(half);
		return Quaternion(axis.x * s, axis.y * s, axis.z * s, std::cos(half));
	}

	/*
	 * Z-up convention:
	 * angles.x = pitch (about +X),
	 * angles.y = roll (about +Y),
	 * angles.z = yaw (about +Z).
	 * Note: Euler angles still gimbal-lock at pitch +-90 degrees,
	 * prefer FromAxisAngle + Slerp for cameras and free objects.
	 */
	static Quaternion FromEuler(const Vector<T, 3>& angles)
	{
		Quaternion yaw = FromAxisAngle(Vector<T, 3>(T(0), T(0), T(1)), angles.z);
		Quaternion pitch = FromAxisAngle(Vector<T, 3>(T(1), T(0), T(0)), angles.x);
		Quaternion roll = FromAxisAngle(Vector<T, 3>(T(0), T(1), T(0)), angles.y);
		return yaw * pitch * roll;
	}

	[[nodiscard]] constexpr Matrix<T, 3> ToMatrix3() const
	{
		Matrix<T, 3> m{};
		T xx = x * x;
		T yy = y * y;
		T zz = z * z;
		m(0, 0) = T(1) - T(2) * (yy + zz);
		m(0, 1) = T(2) * (x * y - z * w);
		m(0, 2) = T(2) * (x * z + y * w);
		m(1, 0) = T(2) * (x * y + z * w);
		m(1, 1) = T(1) - T(2) * (xx + zz);
		m(1, 2) = T(2) * (y * z - x * w);
		m(2, 0) = T(2) * (x * z - y * w);
		m(2, 1) = T(2) * (y * z + x * w);
		m(2, 2) = T(1) - T(2) * (xx + yy);
		return m;
	}

	[[nodiscard]] constexpr Matrix<T, 4> ToMatrix4() const
	{
		Matrix<T, 4> m = Matrix<T, 4>::Identity();
		Matrix<T, 3> r = ToMatrix3();
		for (std::size_t row = 0; row < 3; ++row)
			for (std::size_t col = 0; col < 3; ++col)
				m(row, col) = r(row, col);
		return m;
	}
};

// Shortest-path spherical interpolation, both quaternions must be normalized
template <typename T>
[[nodiscard]] Quaternion<T> Slerp(const Quaternion<T>& a, const Quaternion<T>& b, T t)
{
	T cosTheta = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	Quaternion<T> end = b;
	if (cosTheta < T(0))
	{
		end = -b;
		cosTheta = -cosTheta;
	}
	if (cosTheta > T(1))
		cosTheta = T(1);

	if (cosTheta > T(1) - std::numeric_limits<T>::epsilon())
	{
		// Nearly identical: nlerp avoids division by ~zero
		Quaternion<T> q(
			a.x + (end.x - a.x) * t,
			a.y + (end.y - a.y) * t,
			a.z + (end.z - a.z) * t,
			a.w + (end.w - a.w) * t);
		return q.Normalized();
	}

	T theta = std::acos(cosTheta);
	T sinTheta = std::sin(theta);
	T wa = std::sin((T(1) - t) * theta) / sinTheta;
	T wb = std::sin(t * theta) / sinTheta;
	return Quaternion<T>(
		a.x * wa + end.x * wb,
		a.y * wa + end.y * wb,
		a.z * wa + end.z * wb,
		a.w * wa + end.w * wb);
}

using Quat = Quaternion<float>;

}
