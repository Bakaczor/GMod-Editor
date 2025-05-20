#pragma once
#include "Object.h"

namespace app {
	struct Patch {
	public:
		static const size_t patchSize = 16U;
		static const size_t rowSize = 4U;
		Patch();
		void AddRow(std::array<Object*, rowSize> row);
		std::array<DirectX::XMFLOAT3, patchSize> GetPatch() const;
	private:
		std::vector<Object*> m_points;
	};
}
