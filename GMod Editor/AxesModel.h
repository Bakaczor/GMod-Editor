#pragma once
#include "Mesh.h"
#include <array>

namespace app {
	class AxesModel {
	public:
		AxesModel();
		void Initialize(const Device& device);
		void Render(const mini::dx_ptr<ID3D11DeviceContext>& context) const;

		std::array<float, 3> colorX = { 1.0f, 0.0f, 0.0f };
		std::array<float, 3> colorY = { 0.0f, 1.0f, 0.0f };
		std::array<float, 3> colorZ = { 0.0f, 0.0f, 1.0f };
	private:
		Mesh m_mesh;
	};
}