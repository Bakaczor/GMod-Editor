#include "AxesModel.h"
#include "Data.h"

AxesModel::AxesModel() : m_meshX(), m_meshY(), m_meshZ() {}

void AxesModel::Initialize(const Device& device) {
	std::vector<USHORT> idxs = { 0, 1 };

	std::vector<Vertex_Po> vertsX = {
		{ DirectX::XMFLOAT3(-0.5f, 0, 0) },
		{ DirectX::XMFLOAT3(0.5f, 0, 0) }
	};
	std::vector<Vertex_Po> vertsY = {
		{ DirectX::XMFLOAT3(0, -0.5f, 0) },
		{ DirectX::XMFLOAT3(0, 0.5f, 0) }
	};
	std::vector<Vertex_Po> vertsZ = {
		{ DirectX::XMFLOAT3(0, 0, -0.5f) },
		{ DirectX::XMFLOAT3(0, 0, 0.5f) }
	};

	m_meshX.Update(device, vertsX, idxs, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	m_meshY.Update(device, vertsY, idxs, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	m_meshZ.Update(device, vertsZ, idxs, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

void AxesModel::Render(Device& device, mini::dx_ptr<ID3D11Buffer>& constBuffColor) const {
	device.UpdateBuffer(constBuffColor, DirectX::XMFLOAT4(m_colorX.data()));
	m_meshX.Render(device.deviceContext());
	device.UpdateBuffer(constBuffColor, DirectX::XMFLOAT4(m_colorY.data()));
	m_meshY.Render(device.deviceContext());
	device.UpdateBuffer(constBuffColor, DirectX::XMFLOAT4(m_colorZ.data()));
	m_meshZ.Render(device.deviceContext());
}

