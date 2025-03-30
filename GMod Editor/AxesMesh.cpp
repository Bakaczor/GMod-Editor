#include "AxesMesh.h"

AxesMesh::AxesMesh() : m_meshX(), m_meshY(), m_meshZ() {}

void AxesMesh::Initialize(const Device& device) {
	m_meshX.Update(device, m_vertsX, m_idxs);
	m_meshY.Update(device, m_vertsY, m_idxs);
	m_meshZ.Update(device, m_vertsZ, m_idxs);
}

void AxesMesh::Render(Device& device, mini::dx_ptr<ID3D11Buffer>& constBuffColor) const {
	device.UpdateBuffer(constBuffColor, DirectX::XMFLOAT4(m_colorX.data()));
	m_meshX.Render(device.deviceContext());
	device.UpdateBuffer(constBuffColor, DirectX::XMFLOAT4(m_colorY.data()));
	m_meshY.Render(device.deviceContext());
	device.UpdateBuffer(constBuffColor, DirectX::XMFLOAT4(m_colorZ.data()));
	m_meshZ.Render(device.deviceContext());
}

