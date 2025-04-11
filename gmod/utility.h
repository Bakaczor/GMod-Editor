#pragma once
#include "pch.h"
#include "vector4.h"
#include "matrix4.h"

namespace gmod {
	template <floating_point T>
	inline static T deg2rad(T deg) {
		return std::numbers::pi_v<T> * deg / 180;
	}

	template <floating_point T>
	inline static T rad2deg(T rad) {
		return 180 * rad / std::numbers::pi_v<T>;
	}

	template <floating_point T>
	static vector4<T> transform_coord(const vector4<T>& C, const matrix4<T>& M) {
		vector4<T> res = (C.z() * M.col(2)) + M.col(3);
		res = C.y() * M.col(1) + res;
		res = C.x() * M.col(0) + res;
		res = res * (1 / res.w());
		return vector4<T>(res.x(), res.y(), res.z(), 1);
	}
}