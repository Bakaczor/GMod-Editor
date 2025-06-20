#include "Cursor.h"

using namespace app;

Cursor::Cursor(AxesModel* model) : transform(), m_model(model) {}

void Cursor::SetModel(AxesModel* model) {
	m_model = model;
}

void Cursor::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const {
	if (m_model == nullptr) {
		std::cerr << "[Cursor] : Uninitialized model.";
	} else {
		map.at(ShaderType::RegularWithColors).Set(context);
		m_model->Render(context);
	}
}

