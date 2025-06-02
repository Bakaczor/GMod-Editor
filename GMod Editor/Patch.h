#pragma once
#include "Object.h"

namespace app {
	struct Patch {
	public:
		static const size_t patchSize = 16U;
		static const size_t rowSize = 4U;
		std::array<USHORT, patchSize> indices;
		Patch(std::array<USHORT, patchSize> indices);
	};
}
