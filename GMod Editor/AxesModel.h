#pragma once
#include "Mesh.h"
#include <array>

namespace app {
	class AxesModel {
	public:
		AxesModel();
		void Initialize(const Device& device);
		void Render(Device& device, mini::dx_ptr<ID3D11Buffer>& constBuffColor) const;

		std::array<float, 4> m_colorX = { 1.0f, 0.0f, 0.0f, 1.0f };
		std::array<float, 4> m_colorY = { 0.0f, 1.0f, 0.0f, 1.0f };
		std::array<float, 4> m_colorZ = { 0.0f, 0.0f, 1.0f, 1.0f };
	private:
		Mesh m_meshX;
		Mesh m_meshY;
		Mesh m_meshZ;
	};
}