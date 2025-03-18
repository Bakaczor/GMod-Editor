#define NOMINMAX
#include "Data.h"
#include "Torus.h"
#include <algorithm>
#include <cmath>
#include <numbers>

unsigned short Torus::m_globalCubeNum = 1;

Torus::Torus(double R, double r, unsigned int uParts, unsigned int vParts) : m_R(R), m_r(r), m_uParts(uParts), m_vParts(vParts) {
	std::ostringstream os;
	os << "torus_" << m_globalCubeNum;
	name = os.str();
	m_globalCubeNum += 1;
	RecalculateGeometry();
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