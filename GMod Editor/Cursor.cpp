#include "Cursor.h"

Cursor::Cursor() : transform(), m_mesh() {}

void Cursor::UpdateMesh(const Device& device) {
	m_mesh.Initialize(device);
}

void Cursor::RenderMesh(Device& device, mini::dx_ptr<ID3D11Buffer>& constBuffColor) const {
	m_mesh.Render(device, constBuffColor);
}

