#include "Pointline.h"

unsigned short Pointline::m_globalPolylineNum = 0;

Pointline::Pointline(std::vector<std::shared_ptr<Object>> points) : m_points(points) {
	m_type = "Polyline";
	std::ostringstream os;
	os << "point_" << m_globalPolylineNum;
	name = os.str();
	m_globalPolylineNum += 1;

	for (auto& p : m_points) {
		auto point = dynamic_cast<Point*>(p.get());
		point->AddParent(this);
	}
}

void Pointline::UpdateMesh(const Device& device) {
	m_vertices.clear();
	m_vertices.reserve(m_points.size());
	m_edges.clear();
	m_edges.reserve(m_points.size() - 1);

	for (int i = 0; i < m_points.size(); ++i) {
		VERTEX vertex;
		vertex.pos = m_points[i]->transform.position();
		m_vertices.push_back(vertex);

		if (i < m_points.size() - 1) {
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
