#include "Pointline.h"

unsigned short Pointline::m_globalPointlineNum = 0;

Pointline::Pointline(std::vector<Object*> objects) {
	m_type = "Pointline";
	std::ostringstream os;
	os << "pointline_" << m_globalPointlineNum;
	name = os.str();
	m_globalPointlineNum += 1;

	selected = objects;
	for (auto& obj : selected) {
		auto point = dynamic_cast<Point*>(obj);
		point->AddParent(this);
	}
	geometryChanged = true;
}

void Pointline::UpdateMesh(const Device& device) {
	m_vertices.clear();
	m_vertices.reserve(selected.size());
	m_edges.clear();
	m_edges.reserve(selected.size() - 1);

	for (int i = 0; i < selected.size(); ++i) {
		VERTEX vertex;
		vertex.pos = selected[i]->transform.position();
		m_vertices.push_back(vertex);

		if (i < selected.size() - 1) {
			EDGE edge;
			edge.v1 = i;
			edge.v2 = i + 1;
			m_edges.push_back(edge);
		}
	}
	if (selected.size() <= 1) {
		EDGE edge;
		edge.v1 = 0;
		edge.v2 = 0;
		m_edges.push_back(edge);
	}
	Object::UpdateMesh(device);
}

void Pointline::RenderProperties() {
	Object::RenderProperties();
	if (ImGui::BeginTable("Points", 1, ImGuiTableFlags_ScrollY)) {
		ImGui::TableSetupColumn("Name");
		ImGui::TableHeadersRow();
		for (int i = 0; i < selected.size(); i++) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Selectable(selected[i]->name.c_str(), false, ImGuiSelectableFlags_Disabled);
		}
		ImGui::EndTable();
	}
}

void Pointline::RemoveReferences() {
	for (auto& obj : selected) {
		obj->RemoveParent(this);
	}
}
