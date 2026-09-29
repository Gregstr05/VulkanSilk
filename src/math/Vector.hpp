//
// Created by gregstr on 21.09.26.
//

#pragma once
#include <cstddef>
#include <type_traits>
#include <cmath>

namespace silk::math {

template <typename Derived, typename T, std::size_t N>
struct VectorOps {
    static_assert(std::is_standard_layout_v<T>, "T must be standard layout");

    [[nodiscard]] constexpr T& operator[](std::size_t i)
    {
        return static_cast<Derived&>(*this).members[i];
    }
    [[nodiscard]] constexpr const T& operator[](std::size_t i) const
    {
        return static_cast<const Derived&>(*this).members[i];
    }

    friend constexpr bool operator==(const Derived& lhs, const Derived& rhs)
    {
        for (std::size_t i = 0; i < N; ++i)
        {
            if (lhs[i] != rhs[i])
                return false;
        }
        return true;
    }

    constexpr T* begin() { return static_cast<Derived&>(*this).members; }
    constexpr const T* begin() const {
        return static_cast<const Derived&>(*this).members;
    }
    constexpr T* end() { return begin() + N; }
    constexpr const T* end() const { return begin() + N; }

    static constexpr std::size_t Size() { return N; }

    constexpr Derived operator+(const Derived& other) const
    {
        Derived vec;
        for (size_t i = 0; i < N; ++i)
        {
            vec[i] = (*this)[i] + other[i];
        }
        return vec;
    }

    constexpr Derived operator-() const {
        Derived vec;
        for (std::size_t i = 0; i < N; ++i) {
            vec[i] = -(*this)[i];
        }
        return vec;
    }

    constexpr Derived operator-(const Derived& other) const
    {
        return *this+(-other);
    }

    constexpr Derived operator*(const T x) const
    {
        Derived vec;
        for (size_t i = 0; i < N; ++i)
        {
            vec[i] = (*this)[i] * x;
        }
        return vec;
    }

    constexpr Derived& operator+=(const Derived& other) {
        auto& self = static_cast<Derived&>(*this);
        self = self + other;
        return self;
    }

    constexpr Derived operator+(const T x) const
    {
        Derived vec;
        for (size_t i = 0; i < N; ++i)
        {
            vec[i] = (*this)[i] + x;
        }
        return vec;
    }

    constexpr T Length() const
    {
        T sum = 0;
        for (size_t i = 0; i < N; ++i)
            sum += pow((*this)[i], 2);
        return std::sqrt(sum);
    }

    constexpr Derived Normalize() const
    {
        Derived vec;
        T len = Length();
        for (size_t i = 0; i < N; ++i)
        {
            vec[i] = (*this)[i] / len;
        }
        return vec;
    }

    constexpr T Dot(const Derived& other) const
    {
        T sum = 0;
        for (size_t i = 0; i < N; ++i)
            sum += (*this)[i] * other[i];
        return sum;
    }
};

template <typename T, size_t N>
struct Vector : VectorOps<Vector<T, N>, T, N> {
    T members[N]{};
};

template <typename T>
struct Vector<T, 2> : VectorOps<Vector<T, 2>, T, 2> {
    union {
        struct {T x, y;};
        struct {T u, w;};
        T members[2]{};
    };

    Vector(T x, T y) : x(x), y(y)
    {
    }
};

template <typename T>
struct Vector<T, 3> : VectorOps<Vector<T, 3>, T, 3> {
    union {
        struct {T x, y, z;};
        struct {T r, g, b;};
        T members[3]{};
    };

    Vector(T x, T y, T z) : x(x), y(y), z(z)
    {
    }
};

template <typename T>
struct Vector<T, 4> : VectorOps<Vector<T, 4>, T, 4> {
    union {
        struct {T x, y, z, w;};
        struct {T r, g, b, a;};
        T members[4]{};
    };

    Vector(T x, T y, T z, T w) : x(x), y(y), z(z), w(w)
    {
    }
};

using Vector2 = Vector<float, 2>;
using Vector3 = Vector<float, 3>;
using Vector4 = Vector<float, 4>;

}
