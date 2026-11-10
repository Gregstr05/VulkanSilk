//
// Created by Gregstr on 02/11/2026.
//

#pragma once
#include <cmath>

#include "Matrix.hpp"
#include "Quaternion.hpp"
#include "Vector.hpp"

namespace silk::math {

// Conventions:
//  - Z-up, right-handed world
//  - Column vectors: v' = M * v, so (A * B) * v applies B first
//  - View space looks down -Z (right-handed)
//  - Vulkan NDC: +Y down, depth in [0, 1]

[[nodiscard]] constexpr Matrix4 Translation(const Vector3& t)
{
	Matrix4 m = Matrix4::Identity();
	m(0, 3) = t.x;
	m(1, 3) = t.y;
	m(2, 3) = t.z;
	return m;
}

[[nodiscard]] constexpr Matrix4 Scale(const Vector3& s)
{
	Matrix4 m{};
	m(0, 0) = s.x;
	m(1, 1) = s.y;
	m(2, 2) = s.z;
	m(3, 3) = 1.f;
	return m;
}

[[nodiscard]] constexpr Matrix4 Rotation(const Quat& q)
{
	return q.ToMatrix4();
}

[[nodiscard]] constexpr Matrix4 TRS(const Vector3& t, const Quat& r, const Vector3& s)
{
	return Translation(t) * Rotation(r) * Scale(s);
}

// Vulkan perspective: depth [0, 1], y flipped for Vulkan's y-down NDC.
// The flip inverts screen-space winding, so use
// frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE with back-face culling.
[[nodiscard]] inline Matrix4 Perspective(float fovY, float aspect, float nearZ, float farZ)
{
	Matrix4 m{};
	float f = 1.f / std::tan(fovY / 2.f);
	m(0, 0) = f / aspect;
	m(1, 1) = -f;
	m(2, 2) = farZ / (nearZ - farZ);
	m(2, 3) = nearZ * farZ / (nearZ - farZ);
	m(3, 2) = -1.f;
	return m;
}

[[nodiscard]] inline Matrix4 Ortho(float left, float right, float bottom, float top, float nearZ, float farZ)
{
	Matrix4 m{};
	m(0, 0) = 2.f / (right - left);
	m(1, 1) = -2.f / (top - bottom);
	m(2, 2) = -1.f / (farZ - nearZ);
	m(0, 3) = -(right + left) / (right - left);
	m(1, 3) = (top + bottom) / (top - bottom);
	m(2, 3) = -nearZ / (farZ - nearZ);
	m(3, 3) = 1.f;
	return m;
}

// Right-handed view matrix. Degenerates if the view direction is parallel
// to worldUp (looking straight up/down) - pass a different up in that case.
[[nodiscard]] inline Matrix4 LookAt(const Vector3& eye, const Vector3& target,
	const Vector3& worldUp = Vector3(0.f, 0.f, 1.f))
{
	Vector3 forward = (target - eye).Normalize();
	Vector3 right = forward.Cross(worldUp).Normalize();
	Vector3 up = right.Cross(forward);

	Matrix4 m = Matrix4::Identity();
	m(0, 0) = right.x;
	m(0, 1) = right.y;
	m(0, 2) = right.z;
	m(0, 3) = -right.Dot(eye);
	m(1, 0) = up.x;
	m(1, 1) = up.y;
	m(1, 2) = up.z;
	m(1, 3) = -up.Dot(eye);
	m(2, 0) = -forward.x;
	m(2, 1) = -forward.y;
	m(2, 2) = -forward.z;
	m(2, 3) = forward.Dot(eye);
	return m;
}

}
