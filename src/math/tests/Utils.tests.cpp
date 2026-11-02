//
// Created by Gregstr on 02/11/2026.
//

#include <cmath>
#include <cstddef>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "MathUtils.hpp"
#include "Matrix.hpp"
#include "Quaternion.hpp"

using Catch::Approx;
using silk::math::Matrix;
using silk::math::Matrix2;
using silk::math::Matrix3;
using silk::math::Matrix4;
using silk::math::Vector3;
using silk::math::Vector4;
using silk::math::Perspective;
using silk::math::Quat;

namespace {

constexpr float Pi = 3.14159265358979323846f;

bool ApproxEqual(const Matrix3& lhs, const Matrix3& rhs)
{
	for (std::size_t r = 0; r < 3; ++r)
		for (std::size_t c = 0; c < 3; ++c)
			if (lhs(r, c) != Approx(rhs(r, c)))
				return false;
	return true;
}

Matrix3 RotationZ(float radians)
{
	Matrix3 m{};
	m(0, 0) = std::cos(radians);
	m(0, 1) = -std::sin(radians);
	m(1, 0) = std::sin(radians);
	m(1, 1) = std::cos(radians);
	m(2, 2) = 1.f;
	return m;
}

} // namespace

TEST_CASE("Matrices degrade into vector rows and columns", "[matrix][vector]") {
	Matrix3 m = RotationZ(Pi / 2.f);

	Vector3 row = m[1];
	REQUIRE(row[0] == Approx(1.f));

	m[0] = Vector3(9.f, 0.f, 0.f);
	REQUIRE(m(0, 0) == Approx(9.f));

	Vector3 col = m.Column(2);
	REQUIRE(col[2] == Approx(1.f));
}

TEST_CASE("Matrix-vector multiplication rotates column vectors", "[matrix][geom]") {
	Vector3 v = RotationZ(Pi / 2.f) * Vector3(1.f, 0.f, 0.f);
	REQUIRE(v.x == Approx(0.f).margin(0.001f));
	REQUIRE(v.y == Approx(1.f).margin(0.001f));
	REQUIRE(v.z == Approx(0.f).margin(0.001f));
}

TEST_CASE("Matrix multiplication composes transforms", "[matrix][arith]") {
	REQUIRE(ApproxEqual(RotationZ(Pi / 2.f) * RotationZ(Pi / 2.f), RotationZ(Pi)));
}

TEST_CASE("Identity is the neutral element", "[matrix][arith]") {
	Matrix3 m = RotationZ(0.7f);
	Matrix3 I = Matrix3::Identity();
	REQUIRE(ApproxEqual(I * m, m));
	REQUIRE(ApproxEqual(m * I, m));
}

TEST_CASE("Transpose, trace and determinant", "[matrix][arith]") {
	Matrix3 m{};
	m(0, 0) = 1.f; m(0, 1) = 2.f;
	m(1, 0) = 3.f; m(1, 1) = 4.f;
	m(2, 2) = 5.f;

	REQUIRE(m.Trace() == Approx(10.f));
	REQUIRE(m.Determinant() == Approx(-10.f));

	Matrix2 k{};
	k(0, 0) = 1.f; k(0, 1) = 2.f;
	k(1, 0) = 3.f; k(1, 1) = 4.f;
	REQUIRE(k.Determinant() == Approx(-2.f));

	REQUIRE(m.Transpose()(0, 1) == Approx(3.f));
	REQUIRE(m.Transpose()(1, 0) == Approx(2.f));
}

TEST_CASE("Inverse undoes the original matrix", "[matrix][arith]") {
	Matrix2 m{};
	m(0, 0) = 4.f; m(0, 1) = 7.f;
	m(1, 0) = 2.f; m(1, 1) = 6.f;

	Matrix2 product = m * m.Inverse();
	REQUIRE(product(0, 0) == Approx(1.f));
	REQUIRE(product(0, 1) == Approx(0.f));
	REQUIRE(product(1, 0) == Approx(0.f));
	REQUIRE(product(1, 1) == Approx(1.f));
}

TEST_CASE("4x4 matrices expose their translation column", "[matrix][vector]") {
	REQUIRE(Translation(Vector3(1.f, 2.f, 3.f)).Translation() == Vector3(1.f, 2.f, 3.f));
}

TEST_CASE("Matrix arithmetic works at compile time", "[matrix][constexpr]") {
	constexpr Matrix<int, 2> a = [] {
		Matrix<int, 2> m{};
		m(0, 0) = 1; m(0, 1) = 2;
		m(1, 0) = 3; m(1, 1) = 4;
		return m;
	}();
	STATIC_REQUIRE(a.Determinant() == -2);
	STATIC_REQUIRE((a * Matrix<int, 2>::Identity()) == a);
}

TEST_CASE("Perspective maps view-space depths to Vulkan's [0, 1]", "[utils]") {
	const float nearZ = 0.5f;
	const float farZ = 100.f;
	Matrix4 P = Perspective(Pi / 4.f, 16.f / 9.f, nearZ, farZ);

	Vector4 nearPoint = P * Vector4(0.f, 0.f, -nearZ, 1.f);
	REQUIRE(nearPoint.z / nearPoint.w == Approx(0.f));

	Vector4 farPoint = P * Vector4(0.f, 0.f, -farZ, 1.f);
	REQUIRE(farPoint.z / farPoint.w == Approx(1.f));
}

TEST_CASE("LookAt places the camera at the origin looking down -Z", "[utils]") {
	Vector3 eye(1.f, 2.f, 3.f);
	Vector3 target(4.f, 2.f, 3.f);
	Matrix4 V = LookAt(eye, target);

	Vector4 origin = V * Vector4(eye.x, eye.y, eye.z, 1.f);
	REQUIRE(origin.x == Approx(0.f));
	REQUIRE(origin.y == Approx(0.f));
	REQUIRE(origin.z == Approx(0.f));

	Vector4 forward = V * Vector4(target.x, target.y, target.z, 1.f);
	REQUIRE(forward.x == Approx(0.f));
	REQUIRE(forward.y == Approx(0.f));
	REQUIRE(forward.z == Approx(-3.f));
}

TEST_CASE("TRS composes scale, rotation and translation", "[utils]") {
	Matrix4 m = TRS(Vector3(1.f, 0.f, 0.f),
		Quat::FromAxisAngle(Vector3(0.f, 0.f, 1.f), Pi / 2.f),
		Vector3(2.f, 2.f, 2.f));

	Vector4 p = m * Vector4(1.f, 0.f, 0.f, 1.f);
	REQUIRE(p.x == Approx(1.f));
	REQUIRE(p.y == Approx(2.f));
	REQUIRE(p.z == Approx(0.f));
	REQUIRE(p.w == Approx(1.f));
}
