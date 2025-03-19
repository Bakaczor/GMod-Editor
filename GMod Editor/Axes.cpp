#include "Axes.h"

Axes::Axes() : m_meshX(), m_meshY(), m_meshZ() {}

gmod::matrix4<float> Axes::modelMatrix(const Camera& camera) const {
	gmod::matrix4<float> Ms = gmod::matrix4<float>::scaling(m_scale, m_scale, m_scale);
	// gmod::vector4<float> position = camera.target().position(); //camera.cameraPosition() + camera.viewMatrix() * m_offset;
	// gmod::matrix4<float> Mt = gmod::matrix4<float>::translation(position.x(), position.y(), position.z());
	return Ms;
}

void Axes::Initialize(const Device& device) {
	m_meshX.Update(device, m_vertsX, m_idxs);
	m_meshY.Update(device, m_vertsY, m_idxs);
	m_meshZ.Update(device, m_vertsZ, m_idxs);
}

void Axes::Render(Device& device, mini::dx_ptr<ID3D11Buffer>& constBuffColor) const {
	device.UpdateBuffer(constBuffColor, DirectX::XMFLOAT4(m_colorX.data()));
	m_meshX.Render(device.deviceContext());
	device.UpdateBuffer(constBuffColor, DirectX::XMFLOAT4(m_colorY.data()));
	m_meshY.Render(device.deviceContext());
	device.UpdateBuffer(constBuffColor, DirectX::XMFLOAT4(m_colorZ.data()));
	m_meshZ.Render(device.deviceContext());
}

