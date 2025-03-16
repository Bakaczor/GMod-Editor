#define NOMINMAX
#include "Torus.h"
#include "Data.h"
#include <algorithm>
#include <numbers>
#include <cmath>

unsigned short Torus::m_globalTorusNum = 1;

Torus::Torus(double R, double r, unsigned int uParts, unsigned int vParts) : m_R(R), m_r(r), m_uParts(uParts), m_vParts(vParts) {
	std::ostringstream os;
	os << "torus_" << m_globalTorusNum;
	name = os.str();
	m_globalTorusNum += 1;
	RecalculateGeometry();
}

void Torus::UpdateMesh(const Device& device) {
	std::vector<Vertex_PoCo> verts;
	verts.reserve(m_vertices.size());

	const unsigned int idxsNum = 2 * m_edges.size();
	std::vector<USHORT> idxs;
	idxs.reserve(idxsNum);

	for (const auto& vertex : m_vertices) {
		Vertex_PoCo v;
		v.position = DirectX::XMFLOAT3(
			static_cast<float>(vertex.pos.x()),
			static_cast<float>(vertex.pos.y()),
			static_cast<float>(vertex.pos.z()));
		v.color = DirectX::XMFLOAT3(color[0], color[1], color[2]);
		verts.push_back(v);
	}

	for (const auto& edge : m_edges) {
		idxs.push_back(edge.v1);
		idxs.push_back(edge.v2);
	}

	m_mesh.Update(device, verts, idxs);
}

void Torus::Set_R(double R) {
	m_R = R;
	RecalculateGeometry();
}

void Torus::Set_r(double r) {
	m_r = r;
	RecalculateGeometry();
}

void Torus::Set_uParts(unsigned int uParts) {
	m_uParts = std::max(uParts, m_uPartsMin);
	RecalculateGeometry();
}

void Torus::Set_vParts(unsigned int vParts) {
	m_vParts = std::max(vParts, m_vPartsMin);
	RecalculateGeometry();
}

void Torus::RecalculateGeometry() {
	const int verticesNum = m_uParts * m_vParts;
	m_vertices.clear();
	m_vertices.reserve(verticesNum);
	m_edges.clear();
	m_edges.reserve(2 * verticesNum);

	const double PI2 = 2 * std::numbers::pi;
	const double uStep = 2 * PI2 / m_uParts;
	const double vStep = 2 * PI2 / m_vParts;

	for (unsigned int j = 0; j < m_vParts; ++j) {
		const double v = j * vStep;
		const double cosv = std::cos(v);
		const double sinv = std::sin(v);

		for (unsigned int i = 0; i < m_uParts; ++i) {
			const double u = i * uStep;
			const double cosu = std::cos(u);
			const double sinu = std::sin(u);

			VERTEX vertex;
			vertex.pos = gmod::vector3<double>(
				cosv * (m_R + m_r * cosu),
				sinv * (m_R + m_r * cosu),
				m_r * sinu
			);
			vertex.idx = j * m_uParts + i;
			m_vertices.push_back(vertex);
		}
	}

	for (unsigned int j = 0; j < m_vParts; ++j) {
		const unsigned int j_m_uParts = j * m_uParts;
		const unsigned int j_1_m_vParts_m_uParts = ((j + 1) % m_vParts) * m_uParts;

		for (unsigned int i = 0; i < m_uParts; ++i) {
			unsigned int curr = j_m_uParts + i;
			unsigned int nextU = j_m_uParts + ((i + 1) % m_uParts);
			unsigned int nextV = j_1_m_vParts_m_uParts + i;

			EDGE nextUEdge;
			nextUEdge.v1 = curr;
			nextUEdge.v2 = nextU;
			m_edges.push_back(nextUEdge);

			EDGE nextVEdge;
			nextVEdge.v1 = curr;
			nextVEdge.v2 = nextV;
			m_edges.push_back(nextVEdge);
		}
	}
}