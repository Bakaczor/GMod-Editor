#include "Polyline.h"
#include <numeric>

using namespace app;

unsigned short Polyline::m_globalPolylineNum = 0;

Polyline::Polyline(std::vector<Object*> objects) {
	m_type = "Polyline";
	std::ostringstream os;
	os << "polyline_" << m_globalPolylineNum;
	name = os.str();
	m_globalPolylineNum += 1;

	this->objects = objects;
	for (auto& obj : this->objects) {
		obj->AddParent(this);
	}
	geometryChanged = true;
}

void app::Polyline::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context) const {
	m_mesh.Render(context);
}

void Polyline::UpdateMesh(const Device& device) {
	std::vector<Vertex_Po> verts;
	verts.reserve(objects.size());
	std::vector<USHORT> idxs(objects.size());
	std::iota(idxs.begin(), idxs.end(), 0);

	for (const auto& obj : objects) {
		const auto& pos = obj->position();
		verts.push_back({ DirectX::XMFLOAT3(pos.x(), pos.y(), pos.z()) });
	}

	m_mesh.Update(device, verts, idxs, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
}

void Polyline::RenderProperties() {
	Object::RenderProperties();
	if (ImGui::BeginTable("Points", 1, ImGuiTableFlags_ScrollY)) {
		ImGui::TableSetupColumn("Name");
		ImGui::TableHeadersRow();
		for (int i = 0; i < objects.size(); i++) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Selectable(objects[i]->name.c_str(), false, ImGuiSelectableFlags_Disabled);
		}
		ImGui::EndTable();
	}
}

gmod::matrix4<double> app::Polyline::modelMatrix() const {
	return gmod::matrix4<double>::identity();
}
