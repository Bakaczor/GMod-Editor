#include "Data.h"
#include "Torus.h"

unsigned short Torus::m_globalTorusNum = 1;

Torus::Torus(double R, double r, unsigned int uParts, unsigned int vParts) : m_R(R), m_r(r), m_uParts(uParts), m_vParts(vParts) {
	m_type = "Torus";
	std::ostringstream os;
	os << "torus_" << m_globalTorusNum;
	name = os.str();
	m_globalTorusNum += 1;
	RecalculateGeometry();
}

double Torus::Get_R() const {
	return m_R;
}

void Torus::Set_R(double R) {
	m_R = R;
	RecalculateGeometry();
}

double Torus::Get_r() const {
	return m_r;
}

void Torus::Set_r(double r) {
	m_r = r;
	RecalculateGeometry();
}

int Torus::Get_uParts() const {
	return m_uParts;
}

void Torus::Set_uParts(int uParts) {
	m_uParts = std::max(uParts, m_uPartsMin);
	RecalculateGeometry();
}

int Torus::Get_vParts() const {
	return m_vParts;
}

void Torus::Set_vParts(int vParts) {
	m_vParts = std::max(vParts, m_vPartsMin);
	RecalculateGeometry();
}

void Torus::RenderObjectProperties() {
	Object::RenderObjectProperties();
	bool changed = false;
	double R = m_R;
	if (ImGui::InputDouble("R", &R, 0.001f, 0.1f, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
		m_R = R;
		changed = true;
	}
	double r = m_r;
	if (ImGui::InputDouble("r", &r, 0.001f, 0.1f, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
		m_r = r;
		changed = true;
	}
	int uParts = m_uParts;
	if (ImGui::InputInt("minor", &uParts, 1, 10)) {
		if (uParts * m_vParts > std::numeric_limits<USHORT>::max()) {
			uParts = m_uParts;
		}
		m_uParts = std::max(uParts, m_uPartsMin);
		changed = true;
	}
	int vParts = m_vParts;
	if (ImGui::InputInt("major", &vParts, 1, 10)) {
		if (vParts * m_uParts > std::numeric_limits<USHORT>::max()) {
			vParts = m_vParts;
		}
		m_vParts = std::max(vParts, m_vPartsMin);
		changed = true;
	}
	if (changed) {
		RecalculateGeometry();
	}
}

void Torus::RecalculateGeometry() {
	geometryChanged = true;

	const int verticesNum = m_uParts * m_vParts;
	m_vertices.clear();
	m_vertices.reserve(verticesNum);
	m_edges.clear();
	m_edges.reserve(2 * verticesNum);

	const double PI2 = 2 * DirectX::XM_PI;
	const double uStep = PI2 / m_uParts;
	const double vStep = PI2 / m_vParts;

	for (int j = 0; j < m_vParts; ++j) {
		const double v = j * vStep;
		const double cosv = std::cos(v);
		const double sinv = std::sin(v);

		for (int i = 0; i < m_uParts; ++i) {
			const double u = i * uStep;
			const double cosu = std::cos(u);
			const double sinu = std::sin(u);

			VERTEX vertex;
			vertex.pos = gmod::vector3<double>(
				cosv * (m_R + m_r * cosu),
				sinv * (m_R + m_r * cosu),
				m_r * sinu
			);
			m_vertices.push_back(vertex);
		}
	}

	for (int j = 0; j < m_vParts; ++j) {
		const int j_m_uParts = j * m_uParts;
		const int j_1_m_vParts_m_uParts = ((j + 1) % m_vParts) * m_uParts;

		for (int i = 0; i < m_uParts; ++i) {
			int curr = j_m_uParts + i;
			int nextU = j_m_uParts + ((i + 1) % m_uParts);
			int nextV = j_1_m_vParts_m_uParts + i;

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