#pragma once
#include "../gmod/vector3.h"
#include <algorithm>

namespace app {
	static class Helper {
	public:
		static inline bool AreEqualF(float a, float b, float eps) {
			return std::abs(a - b) < eps;
		}

		template<std::floating_point T1, std::floating_point T2>
		static inline bool AreEqualV3(gmod::vector3<T1> v, gmod::vector3<T2> w, float eps) {
			return
				std::abs(v.x() - w.x()) < eps &&
				std::abs(v.y() - w.y()) < eps &&
				std::abs(v.z() - w.z()) < eps;
		}
	};
}
