#include "Cursor.h"

Cursor::Cursor(AxesModel* model) : transform(), m_model(model) {}

void Cursor::SetModel(AxesModel* model) {
	m_model = model;
}

void Cursor::RenderMesh(Device& device, mini::dx_ptr<ID3D11Buffer>& constBuffColor) const {
	if (m_model == nullptr) {
		std::cerr << "[Cursor] : Uninitialized model.";
	} else {
		m_model->Render(device, constBuffColor);
	}
}

