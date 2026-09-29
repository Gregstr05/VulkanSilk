//
// Created by Gregstr on 29/09/2026.
//

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "Vector.hpp"

using Catch::Approx;
using silk::math::Vector;
using silk::math::Vector2;
using silk::math::Vector3;
using silk::math::Vector4;

#include <type_traits>

TEST_CASE("Named components flow through CRTP operators",
          "[vector][crtp]") {
    Vector3 v;
    v.x = 1.f;
    v.y = 2.f;
    v.z = 3.f;

    Vector3 w = v + v;
    REQUIRE(w.x == Approx(2.f));
    REQUIRE(w.y == Approx(4.f));
    REQUIRE(w.z == Approx(6.f));

    v += v;
    REQUIRE(v.x == Approx(2.f));
    REQUIRE(v.y == Approx(4.f));
    REQUIRE(v.z == Approx(6.f));
}

TEST_CASE("Named components work in geometric functions",
          "[vector][crtp][geom]") {
    Vector3 v;
    v.x = 0.f;
    v.y = 3.f;
    v.z = 4.f;

    REQUIRE(v.Length() == Approx(5.f));

    Vector3 n = v.Normalize();
    REQUIRE(n.y == Approx(0.6f));
    REQUIRE(n.z == Approx(0.8f));
    REQUIRE(n.Dot(v) == Approx(v.x*n.x + v.y*n.y + v.z*n.z));
}

TEST_CASE("CRTP incurs no layout overhead", "[vector][crtp]") {
    STATIC_REQUIRE(std::is_standard_layout_v<Vector3>);
    STATIC_REQUIRE(std::is_trivially_copyable_v<Vector3>);
    STATIC_REQUIRE(sizeof(Vector2) == 2 * sizeof(float));
    STATIC_REQUIRE(sizeof(Vector3) == 3 * sizeof(float));
    STATIC_REQUIRE(sizeof(Vector4) == 4 * sizeof(float));
}

TEST_CASE("Generic vectors share the same operator surface", "[vector][crtp]") {
    // Primary template, no union — safe for constexpr and mixed sizes.
    constexpr Vector<int, 5> a = [] {
        Vector<int, 5> v{};
        for (int i = 0; i < 5; ++i) {
            v[static_cast<std::size_t>(i)] = i + 1;
        }
        return v;
    }();
    constexpr Vector<int, 5> b = [] {
        Vector<int, 5> v{};
        for (int i = 0; i < 5; ++i) {
            v[static_cast<std::size_t>(i)] = 2;
        }
        return v;
    }();

    STATIC_REQUIRE((a * 2) == (a + a));
    STATIC_REQUIRE(a.Dot(b) == 30);
    STATIC_REQUIRE((a - a) == Vector<int, 5>{});
}

TEST_CASE("Components can be indexed and iterated", "[vector][access]") {
    Vector3 v;
    v.x = 1.f;
    v.y = 2.f;
    v.z = 3.f;

    REQUIRE(v.Size() == 3);
    REQUIRE(v[0] == 1.f);
    REQUIRE(v[2] == 3.f);

    float sum = 0.f;
    for (float component : v) {
        sum += component;
    }
    REQUIRE(sum == Approx(6.f));
}

TEST_CASE("Color and position aliases share the same storage",
          "[vector][access]") {
    Vector4 v;
    v.r = 1.f;
    v.g = 0.5f;
    v.b = 0.25f;
    v.a = 1.f;

    REQUIRE(v.x == Approx(1.f));
    REQUIRE(v.y == Approx(0.5f));
    REQUIRE(v.z == Approx(0.25f));
    REQUIRE(v.w == Approx(1.f));

    v.x = 42.f;
    REQUIRE(v.r == Approx(42.f));
}

TEST_CASE("Vectors add component-wise", "[vector][arith]") {
    Vector2 a;
    a.x = 1.f;
    a.y = 2.f;
    Vector2 b;
    b.x = 10.f;
    b.y = 20.f;

    Vector2 c = a + b;

    REQUIRE(c.x == Approx(11.f));
    REQUIRE(c.y == Approx(22.f));
}

TEST_CASE("Vectors subtract component-wise", "[vector][arith]") {
    Vector2 a;
    a.x = 1.f;
    a.y = 2.f;
    Vector2 b;
    b.x = 10.f;
    b.y = 20.f;

    Vector2 c = a - b;
    Vector2 d = -a;

    REQUIRE(c.x == Approx(-9.f));
    REQUIRE(c.y == Approx(-18.f));
    REQUIRE(d.x == Approx(-1.f));
    REQUIRE(d.y == Approx(-2.f));
}

TEST_CASE("operator+= accumulates in place", "[vector][arith]") {
    Vector2 a;
    a.x = 1.f;
    a.y = 2.f;
    Vector2 b;
    b.x = 100.f;
    b.y = 200.f;

    a += b;
    a += a;

    REQUIRE(a.x == Approx(202.f));
    REQUIRE(a.y == Approx(404.f));
}

TEST_CASE("Scalar multiplication scales every component",
          "[vector][arith]") {
    Vector3 v;
    v.x = 1.f;
    v.y = -2.f;
    v.z = 3.f;

    Vector3 s = v * 2.f;

    REQUIRE(s.x == Approx(2.f));
    REQUIRE(s.y == Approx(-4.f));
    REQUIRE(s.z == Approx(6.f));
}

TEST_CASE("Scalar addition broadcasts to every component",
          "[vector][arith]") {
    Vector2 v;
    v.x = 1.f;
    v.y = 2.f;

    Vector2 s = v + 10.f;

    REQUIRE(s.x == Approx(11.f));
    REQUIRE(s.y == Approx(12.f));
}

TEST_CASE("Length computes the euclidean norm", "[vector][geom]") {
    Vector3 v;
    v.x = 3.f;
    v.y = 4.f;
    v.z = 0.f;

    REQUIRE(v.Length() == Approx(5.f));

    Vector3 zero{};
    REQUIRE(zero.Length() == Approx(0.f));
}

TEST_CASE("Normalize yields a unit vector without mutating the original",
          "[vector][geom]") {
    Vector3 v;
    v.x = 3.f;
    v.y = 4.f;
    v.z = 0.f;

    Vector3 n = v.Normalize();

    REQUIRE(n.Length() == Approx(1.f));
    REQUIRE(n.x == Approx(0.6f));
    REQUIRE(n.y == Approx(0.8f));
    REQUIRE(n.z == Approx(0.f));

    REQUIRE(v.Length() == Approx(5.f));
}

TEST_CASE("Dot product behaves like the inner product", "[vector][geom]") {
    Vector3 xAxis;
    xAxis.x = 1.f;
    Vector3 yAxis;
    yAxis.y = 1.f;
    Vector3 v;
    v.x = 1.f;
    v.y = 2.f;
    v.z = 3.f;

    REQUIRE(xAxis.Dot(yAxis) == Approx(0.f)); // orthogonal
    REQUIRE(v.Dot(v) == Approx(v.Length() * v.Length()));
    REQUIRE(v.Dot(xAxis) == Approx(1.f)); // projection onto x
}

TEST_CASE("Arithmetic works at compile time", "[vector][constexpr]") {
    constexpr Vector<int, 2> a = [] {
        Vector<int, 2> v;
        v[0] = 2;
        v[1] = 3;
        return v;
    }();
    constexpr Vector<int, 2> b = [] {
        Vector<int, 2> v;
        v[0] = 10;
        v[1] = 20;
        return v;
    }();

    STATIC_REQUIRE((a + b)[0] == 12);
    STATIC_REQUIRE((b - a)[1] == 17);
    STATIC_REQUIRE((a * 3)[0] == 6);
    STATIC_REQUIRE(a.Dot(b) == 80);
}