//
// Created by Gregstr on 02/11/2026.
//

#include <cmath>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "Matrix.hpp"
#include "Quaternion.hpp"

using Catch::Approx;
using silk::math::Matrix3;
using silk::math::Quat;
using silk::math::Vector3;

namespace {

constexpr float Pi = 3.14159265358979323846f;

bool ApproxEqual(const Vector3& lhs, const Vector3& rhs)
{
	return lhs.x == Approx(rhs.x).margin(0.001f)
	&& lhs.y == Approx(rhs.y).margin(0.001f)
	&& lhs.z == Approx(rhs.z).margin(0.001f);
}

} // namespace

TEST_CASE("Identity quaternion preserves vectors", "[quaternion]") {
	Vector3 v(1.f, 2.f, 3.f);
	REQUIRE(ApproxEqual(Quat().Rotate(v), v));
	REQUIRE(ApproxEqual(Quat::Identity().Rotate(v), v));
}

TEST_CASE("Axis-angle quarter turn about Z maps +X to +Y", "[quaternion]") {
	Quat q = Quat::FromAxisAngle(Vector3(0.f, 0.f, 1.f), Pi / 2.f);
	REQUIRE(ApproxEqual(q.Rotate(Vector3(1.f, 0.f, 0.f)), Vector3(0.f, 1.f, 0.f)));
	REQUIRE(q.w == Approx(std::cos(Pi / 4.f)));
}

TEST_CASE("q and -q encode the same rotation", "[quaternion]") {
	Quat q = Quat::FromAxisAngle(Vector3(0.f, 0.f, 1.f), Pi / 2.f);
	Vector3 v(1.f, 2.f, 3.f);
	REQUIRE(ApproxEqual(q.Rotate(v), (-q).Rotate(v)));
}

TEST_CASE("Quaternion rotation matches its rotation matrix", "[quaternion]") {
	Quat q = Quat::FromAxisAngle(Vector3(1.f, 1.f, 0.f).Normalize(), 0.7f);
	Vector3 v(1.f, 2.f, 3.f);

	Matrix3 m = q.ToMatrix3();
	REQUIRE(ApproxEqual(q.Rotate(v), m * v));

	REQUIRE(q.ToMatrix4().Translation() == Vector3(0.f, 0.f, 0.f));
}

TEST_CASE("Composing quarter turns yields a half turn", "[quaternion]") {
	Quat quarter = Quat::FromAxisAngle(Vector3(0.f, 0.f, 1.f), Pi / 2.f);
	REQUIRE(ApproxEqual((quarter * quarter).Rotate(Vector3(1.f, 0.f, 0.f)), Vector3(-1.f, 0.f, 0.f)));
}

TEST_CASE("Slerp stays on the unit sphere", "[quaternion]") {
	Quat a = Quat::Identity();
	Quat b = Quat::FromAxisAngle(Vector3(0.f, 0.f, 1.f), Pi / 2.f);

	REQUIRE(ApproxEqual(Slerp(a, b, 0.f).Rotate(Vector3(1.f, 0.f, 0.f)), Vector3(1.f, 0.f, 0.f)));
	REQUIRE(ApproxEqual(Slerp(a, b, 1.f).Rotate(Vector3(1.f, 0.f, 0.f)), Vector3(0.f, 1.f, 0.f)));

	Quat mid = Slerp(a, b, 0.5f);
	REQUIRE(mid.Norm() == Approx(1.f));
	REQUIRE(mid.w == Approx(std::cos(Pi / 8.f)));
}

TEST_CASE("FromEuler yaw is a quarter turn about Z", "[quaternion]") {
	Quat q = Quat::FromEuler(Vector3(0.f, 0.f, Pi / 2.f));
	REQUIRE(ApproxEqual(q.Rotate(Vector3(1.f, 0.f, 0.f)), Vector3(0.f, 1.f, 0.f)));
}
