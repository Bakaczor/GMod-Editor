#include "Object.h"

int Object::m_globalObjectId = 1;
unsigned short Object::m_globalObjectNum = 1;

Object::Object() : transform(), m_mesh() {
	std::ostringstream os;
	os << "object_" << m_globalObjectNum;
	name = os.str();
	m_globalObjectNum += 1;

	id = m_globalObjectId;
	m_globalObjectId += 1;
}

void Object::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context) const {
	m_mesh.Render(context);
}

void Object::UpdateMesh(const Device& device) {
	std::vector<Vertex_Po> verts;
	verts.reserve(m_vertices.size());

	const unsigned int idxsNum = 2 * m_edges.size();
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

	for (const auto& edge : m_edges) {
		idxs.push_back(edge.v1);
		idxs.push_back(edge.v2);
	}

	m_mesh.Update(device, verts, idxs);
}
