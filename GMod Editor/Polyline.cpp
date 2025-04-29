#include "Polyline.h"
#include <numeric>

using namespace app;

unsigned short Polyline::m_globalPolylineNum = 0;

Polyline::Polyline(bool increment) {
	m_type = "Polyline";
	std::ostringstream os;
	os << "polyline_" << m_globalPolylineNum;
	name = os.str();
	if (increment) {
		m_globalPolylineNum += 1;
	}
}

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

void app::Polyline::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const {
	map.at(ShaderType::Regular).Set(context);
	m_polylineMesh.Render(context);
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

	m_polylineMesh.Update(device, verts, idxs, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	Object::UpdateMesh(device);
}

void Polyline::RenderProperties() {
	Object::RenderProperties();
	if (ImGui::Button("Delete", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
		if (-1 != m_selectedIdx) {
			RemoveObject(objects.at(m_selectedIdx));
			m_selectedIdx = -1;
		}
	}
	if (ImGui::BeginTable("Points", 1, ImGuiTableFlags_ScrollY)) {
		ImGui::TableSetupColumn("Name");
		ImGui::TableHeadersRow();
		for (int i = 0; i < objects.size(); i++) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			if(ImGui::Selectable(objects[i]->name.c_str(), i == m_selectedIdx, ImGuiSelectableFlags_SpanAllColumns)) {
				if (m_selectedIdx == i) {
					m_selectedIdx = -1;
				} else {
					m_selectedIdx = i;
				}
			}
		}
		ImGui::EndTable();
	}
}

gmod::matrix4<double> app::Polyline::modelMatrix() const {
	return gmod::matrix4<double>::identity();
}
