#include "Patch.h"

using namespace app;

Patch::Patch() {
	m_points.reserve(patchSize);
}

void Patch::AddRow(std::array<Object*, rowSize> row) {
	std::copy(row.begin(), row.end(), std::back_inserter(m_points));
}

std::array<DirectX::XMFLOAT3, Patch::patchSize> Patch::GetPatch() const {
	std::array<DirectX::XMFLOAT3, Patch::patchSize> output{};
	
	std::transform(m_points.begin(), m_points.end(), output.begin(), [](Object* obj) {
        const auto& pos = obj->position();
        return DirectX::XMFLOAT3(pos.x(), pos.y(), pos.z());
    });

	return output;
}