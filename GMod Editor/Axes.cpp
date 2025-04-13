#include "Axes.h"

Axes::Axes(AxesModel* model) : m_model(model) {}

gmod::matrix4<float> Axes::modelMatrix(const Camera& camera, float f, float a) const {
	float s = f * std::tanf(a / 2);
	gmod::matrix4<float> Ms = gmod::matrix4<float>::scaling(s, s, s);
	// gmod::vector4<float> position = camera.target().position(); //camera.cameraPosition() + camera.viewMatrix() * m_offset;
	// gmod::matrix4<float> Mt = gmod::matrix4<float>::translation(position.x(), position.y(), position.z());
	return Ms;
}

void Axes::SetModel(AxesModel* model) {
	m_model = model;
}

void Axes::RenderMesh(Device& device, mini::dx_ptr<ID3D11Buffer>& constBuffColor) const {
	if (m_model == nullptr) {
		std::cerr << "[Axes] : Uninitialized model.";
	} else {
		m_model->Render(device, constBuffColor);
	}
}