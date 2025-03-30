#include "Axes.h"

Axes::Axes() : m_mesh() {}

gmod::matrix4<float> Axes::modelMatrix(const Camera& camera) const {
	gmod::matrix4<float> Ms = gmod::matrix4<float>::scaling(m_scale, m_scale, m_scale);
	// gmod::vector4<float> position = camera.target().position(); //camera.cameraPosition() + camera.viewMatrix() * m_offset;
	// gmod::matrix4<float> Mt = gmod::matrix4<float>::translation(position.x(), position.y(), position.z());
	return Ms;
}

void Axes::UpdateMesh(const Device& device) {
	m_mesh.Initialize(device);
}

void Axes::RenderMesh(Device& device, mini::dx_ptr<ID3D11Buffer>& constBuffColor) const {
	m_mesh.Render(device, constBuffColor);
}