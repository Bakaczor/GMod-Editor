#include "Point.h"
#include "Selection.h"

unsigned short Point::m_globalPointNum = 0;

Point::Point() {
	m_type = "Point";
	std::ostringstream os;
	os << "point_" << m_globalPointNum;
	name = os.str();
	m_globalPointNum += 1;
	InitGeometry();
	color = { 1.0f, 0.0f, 0.0f, 1.0f };
}

void Point::AddParent(Object* obj) {
	if (std::find_if(m_parents.begin(), m_parents.end(), [&obj](const auto& o) { return o->id == obj->id; }) != m_parents.end()) { return; }
	m_parents.push_back(obj);
}

void Point::RemoveParent(Object* obj) {
	std::erase_if(m_parents, [&obj](const auto& o) { return o->id == obj->id; });
}

void Point::InformParents() {
	for (auto& obj : m_parents) {
		obj->geometryChanged = true;
	}
}

void Point::RemoveReferences() {
	for (auto& obj : m_parents) {
		obj->geometryChanged = true;
		auto selection = dynamic_cast<Selection*>(obj);
		if (selection != nullptr) {
			std::erase_if(selection->selected, [this](const auto& o) { return o->id == id; });
		}
	}
}

void Point::UpdateMesh(const Device& device) {
	std::vector<Vertex_Po> verts;
	verts.reserve(m_vertices.size());

	const unsigned int idxsNum = 3 * m_faces.size();
	std::vector<USHORT> idxs;
	idxs.reserve(idxsNum);

	for (const auto& vertex : m_vertices) {
		Vertex_Po v;
		v.position = DirectX::XMFLOAT3(
			static_cast<float>(vertex.pos.x()),
			static_cast<float>(vertex.pos.y()),
			static_cast<float>(vertex.pos.z()));
		verts.push_back(v);
	}

	for (const auto& face : m_faces) {
		idxs.push_back(face.v1);
		idxs.push_back(face.v2);
		idxs.push_back(face.v3);
	}

	m_mesh.Update(device, verts, idxs, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_vertices.clear();
	m_edges.clear();
}

void Point::InitGeometry() {
	geometryChanged = true;

	const int verticesNum = m_parts * m_parts;
	m_vertices.clear();
	m_vertices.reserve(verticesNum);
	m_faces.clear();
	m_faces.reserve(2 * verticesNum);

	const double step = 2 * DirectX::XM_PI / m_parts;

	for (int j = 0; j < m_parts; ++j) {
		const double v = j * step;
		const double cosv = std::cos(v);
		const double sinv = std::sin(v);

		for (int i = 0; i < m_parts; ++i) {
			const double u = i * step;
			const double cosu = std::cos(u);
			const double sinu = std::sin(u);

			VERTEX vertex;
			vertex.pos = m_r * gmod::vector3<double>(cosv * cosu, sinv * cosu, sinu);
			m_vertices.push_back(vertex);
		}
	}

	for (int j = 0; j < m_parts - 1; ++j) {
		for (int i = 0; i < m_parts - 1; ++i) {
			USHORT v1 = j * m_parts + i;
			USHORT v2 = j * m_parts + (i + 1) % m_parts;
			USHORT v3 = (j + 1) * m_parts + i;
			USHORT v4 = (j + 1) * m_parts + (i + 1) % m_parts;

			m_faces.push_back({ v1, v2, v3 });
			m_faces.push_back({ v2, v4, v3 });
		}
	}
}
