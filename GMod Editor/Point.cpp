#include "Point.h"

unsigned short Point::m_globalPointNum = 0;

Point::Point() {
	m_type = "Point";
	std::ostringstream os;
	os << "point_" << m_globalPointNum;
	name = os.str();
	m_globalPointNum += 1;
	InitGeometry();
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

	m_mesh.Update(device, verts, idxs);
	m_vertices.clear();
	m_edges.clear();
}

void Point::InitGeometry() {


}
