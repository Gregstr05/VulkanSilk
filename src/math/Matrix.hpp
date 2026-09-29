//
// Created by Gregstr on 29/09/2026.
//

#pragma once
#include <vector>

#include "Vector.hpp"

namespace silk::math {

template <typename T, size_t N>
class Matrix {
public:
	

private:
	std::vector<Vector<T, N>> members;
};

}
