#include "Polyline.h"
#include <numeric>

unsigned short Polyline::m_globalPointlineNum = 0;

Polyline::Polyline(std::vector<Object*> objects) {
	m_type = "Polyline";
	std::ostringstream os;
	os << "pointline_" << m_globalPointlineNum;
	name = os.str();
	m_globalPointlineNum += 1;

	objects = objects;
	for (auto& obj : objects) {
		auto point = dynamic_cast<Point*>(obj);
		point->AddParent(this);
	}
	geometryChanged = true;
}

Polyline::~Polyline() {
	for (auto& obj : objects) {
		obj->RemoveParent(this);
	}
}

void Polyline::UpdateMesh(const Device& device) {
	std::vector<Vertex_Po> verts(objects.size());
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
