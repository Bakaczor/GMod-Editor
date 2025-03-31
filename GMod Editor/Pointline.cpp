#include "Pointline.h"

unsigned short Pointline::m_globalPolylineNum = 0;

Pointline::Pointline() {
	m_type = "Polyline";
	std::ostringstream os;
	os << "point_" << m_globalPolylineNum;
	name = os.str();
	m_globalPolylineNum += 1;
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
	Object::UpdateMesh(device);
}

void Pointline::RenderProperties() {
	Object::RenderProperties();
	// TODO : display points
}
