#ifndef GMOD_PCH_H
#define GMOD_PCH_H

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <iostream>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <utility>

namespace gmod {
	template<typename T>
	concept floating_point = std::floating_point<T>;
}
#endif //GMOD_PCH_H
