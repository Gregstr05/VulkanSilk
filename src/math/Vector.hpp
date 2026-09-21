//
// Created by gregstr on 21.09.26.
//

#pragma once
#include <cstddef>
#include <type_traits>

namespace silk::math {

template <typename T, std::size_t N>
struct Vector {
    static_assert(std::is_standard_layout_v<T>, "T must be standard layout");

    T members[N];

    [[nodiscard]] constexpr T& operator[](std::size_t i) {return members[i];}
    [[nodiscard]] constexpr const T& operator[](std::size_t i) const {return members[i];}

    constexpr T* begin() {return members;}
    constexpr const T* begin() const {return members;}
    constexpr T* end() {return members + N;}
    constexpr const T* end() const {return members + N;}

    constexpr std::size_t size() const {return N;}

    constexpr Vector operator+(const Vector& other) const
    {
        Vector vec;
        for (size_t i = 0; i < N; ++i)
        {
            vec[i] = members[i] + other[i];
        }
        return vec;
    }

    constexpr Vector operator-(const Vector& other) const
    {
        return this+(-other);
    }

    constexpr Vector operator*(const size_t x) const
    {
        Vector vec;
        for (size_t i = 0; i < N; ++i)
        {
            vec[i] = members[i] * x;
        }
        return vec;
    }

};

template <typename T>
struct Vector<T, 2> {
    union {
        struct {T x, y;};
        T members[2];
    };
};

template <typename T>
struct Vector<T, 3> {
    union {
        struct {T x, y, z;};
        struct {T r, g, b;};
        T members[3];
    };
};

template <typename T>
struct Vector<T, 4> {
    union {
        struct {T x, y, z, w;};
        struct {T r, g, b, a;};
        T members[4];
    };
};

using Vector2 = Vector<float, 2>;
using Vector3 = Vector<float, 3>;
using Vector4 = Vector<float, 4>;

}
