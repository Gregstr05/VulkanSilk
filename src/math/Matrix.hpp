//
// Created by Gregstr on 29/09/2026.
//

#pragma once
#include <cstddef>
#include <type_traits>

#include "Vector.hpp"

namespace silk::math {

template <typename Derived, typename T, std::size_t N>
struct MatrixOps {
	static_assert(std::is_standard_layout_v<T>, "T must be standard layout");

	// Rows degrade into vectors: m[r] is a Vector<T, N>&
	[[nodiscard]] constexpr Vector<T, N>& operator[](std::size_t r)
	{
		return static_cast<Derived&>(*this).members[r];
	}
	[[nodiscard]] constexpr const Vector<T, N>& operator[](std::size_t r) const
	{
		return static_cast<const Derived&>(*this).members[r];
	}

	[[nodiscard]] constexpr T& operator()(std::size_t r, std::size_t c)
	{
		return (*this)[r][c];
	}
	[[nodiscard]] constexpr const T& operator()(std::size_t r, std::size_t c) const
	{
		return (*this)[r][c];
	}

	friend constexpr bool operator==(const Derived& lhs, const Derived& rhs)
	{
		for (std::size_t r = 0; r < N; ++r)
		{
			if (!(lhs[r] == rhs[r]))
				return false;
		}
		return true;
	}

	static constexpr std::size_t Size() { return N; }

	[[nodiscard]] constexpr Vector<T, N> Column(std::size_t c) const
	{
		const Derived& self = static_cast<const Derived&>(*this);
		Vector<T, N> col{};
		for (std::size_t r = 0; r < N; ++r)
			col[r] = self[r][c];
		return col;
	}

	constexpr Derived operator+(const Derived& other) const
	{
		const Derived& self = static_cast<const Derived&>(*this);
		Derived result;
		for (std::size_t r = 0; r < N; ++r)
			result[r] = self[r] + other[r];
		return result;
	}

	constexpr Derived operator-() const
	{
		const Derived& self = static_cast<const Derived&>(*this);
		Derived result{};
		for (std::size_t r = 0; r < N; ++r)
			result[r] = -self[r];
		return result;
	}

	constexpr Derived operator-(const Derived& other) const
	{
		return *this + (-other);
	}

	constexpr Derived& operator+=(const Derived& other)
	{
		Derived& self = static_cast<Derived&>(*this);
		self = self + other;
		return self;
	}

	constexpr Derived operator*(T scalar) const
	{
		const Derived& self = static_cast<const Derived&>(*this);
		Derived result{};
		for (std::size_t r = 0; r < N; ++r)
			result[r] = self[r] * scalar;
		return result;
	}

	// Column-vector convention: (lhs * rhs) * v == lhs * (rhs * v)
	friend constexpr Derived operator*(const Derived& lhs, const Derived& rhs)
	{
		Derived result{};
		for (std::size_t r = 0; r < N; ++r)
		{
			for (std::size_t c = 0; c < N; ++c)
			{
				T sum = T(0);
				for (std::size_t k = 0; k < N; ++k)
					sum += lhs[r][k] * rhs[k][c];
				result[r][c] = sum;
			}
		}
		return result;
	}

	// v' = M * v
	friend constexpr Vector<T, N> operator*(const Derived& m, const Vector<T, N>& v)
	{
		Vector<T, N> result{};
		for (std::size_t r = 0; r < N; ++r)
			result[r] = m[r].Dot(v);
		return result;
	}
};

template <typename T, std::size_t N>
struct Matrix : MatrixOps<Matrix<T, N>, T, N> {
	Vector<T, N> members[N]{};

	static constexpr Matrix Identity()
	{
		Matrix m{};
		for (std::size_t i = 0; i < N; ++i)
			m(i, i) = T(1);
		return m;
	}

	[[nodiscard]] constexpr Matrix Transpose() const
	{
		Matrix m{};
		for (std::size_t r = 0; r < N; ++r)
			for (std::size_t c = 0; c < N; ++c)
				m(c, r) = (*this)(r, c);
		return m;
	}

	[[nodiscard]] constexpr T Trace() const
	{
		T sum = T(0);
		for (std::size_t i = 0; i < N; ++i)
			sum += (*this)(i, i);
		return sum;
	}

	[[nodiscard]] constexpr T Determinant() const
	{
		Matrix tmp = *this;
		T det = T(1);
		for (std::size_t col = 0; col < N; ++col)
		{
			std::size_t pivot = col;
			while (pivot < N && tmp(pivot, col) == T(0))
				++pivot;
			if (pivot == N)
				return T(0);
			if (pivot != col)
			{
				det = -det;
				for (std::size_t c = 0; c < N; ++c)
				{
					T t = tmp(col, c);
					tmp(col, c) = tmp(pivot, c);
					tmp(pivot, c) = t;
				}
			}
			det *= tmp(col, col);
			for (std::size_t r = col + 1; r < N; ++r)
			{
				T factor = tmp(r, col) / tmp(col, col);
				for (std::size_t c = col; c < N; ++c)
					tmp(r, c) -= factor * tmp(col, c);
			}
		}
		return det;
	}

	// Gauss-Jordan. Returns Identity for singular input: cheap, and keeps
	// this constexpr-friendly.
	[[nodiscard]] constexpr Matrix Inverse() const
	{
		Matrix a = *this;
		Matrix inv = Identity();

		for (std::size_t col = 0; col < N; ++col)
		{
			std::size_t pivot = col;
			while (pivot < N && a(pivot, col) == T(0))
				++pivot;
			if (pivot == N)
				return Identity();

			if (pivot != col)
			{
				for (std::size_t c = 0; c < N; ++c)
				{
					T t = a(col, c);
					a(col, c) = a(pivot, c);
					a(pivot, c) = t;

					t = inv(col, c);
					inv(col, c) = inv(pivot, c);
					inv(pivot, c) = t;
				}
			}

			T pivotValue = a(col, col);
			for (std::size_t c = 0; c < N; ++c)
			{
				a(col, c) /= pivotValue;
				inv(col, c) /= pivotValue;
			}

			for (std::size_t r = 0; r < N; ++r)
			{
				if (r == col)
					continue;
				T factor = a(r, col);
				if (factor == T(0))
					continue;
				for (std::size_t c = 0; c < N; ++c)
				{
					a(r, c) -= factor * a(col, c);
					inv(r, c) -= factor * inv(col, c);
				}
			}
		}
		return inv;
	}

	// 4x4-only helpers, only instantiated when called
	[[nodiscard]] constexpr Vector<T, 3> Translation() const
	{
		static_assert(N == 4, "Translation requires a 4x4 matrix");
		return Vector<T, 3>((*this)(0, 3), (*this)(1, 3), (*this)(2, 3));
	}
};

using Matrix2 = Matrix<float, 2>;
using Matrix3 = Matrix<float, 3>;
using Matrix4 = Matrix<float, 4>;

}
