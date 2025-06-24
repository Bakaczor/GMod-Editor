#include "Torus.h"
#include "Data.h"

using namespace app;

unsigned short Torus::m_globalTorusNum = 0;

Torus::Torus(double R, double r, unsigned int uParts, unsigned int vParts) : m_R(R), m_r(r), m_uParts(uParts), m_vParts(vParts) {
	intersectable = true;
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

void Torus::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const {
	map.at(ShaderType::Regular).Set(context);
	m_mesh.Render(context);
}

void Torus::UpdateMesh(const Device& device) {
	std::vector<Vertex_Po> verts;
	verts.reserve(m_vertices.size());

	const unsigned int idxsNum = 2 * m_edges.size();
	std::vector<USHORT> idxs;
	idxs.reserve(idxsNum);

	for (const auto& vertex : m_vertices) {
		Vertex_Po v;
		v.position = DirectX::XMFLOAT3(
			static_cast<float>(vertex.x),
			static_cast<float>(vertex.y),
			static_cast<float>(vertex.z));
		verts.push_back(v);
	}

	for (const auto& edge : m_edges) {
		idxs.push_back(edge.v1);
		idxs.push_back(edge.v2);
	}

	m_mesh.Update(device, verts, idxs, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	m_vertices.clear();
	m_edges.clear();

	Object::UpdateMesh(device);
}

void Torus::RenderProperties() {
	Object::RenderProperties();
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

#pragma region IGEOMETRICAL
IGeometrical::XYZBounds Torus::WorldBounds() const {
	const double xMin = -(m_R + m_r);
	const double xMax = (m_R + m_r);
	const double yMin = -m_r;
	const double yMax = m_r;
	const double zMin = -(m_R + m_r);
	const double zMax = (m_R + m_r);

	const auto& M = modelMatrix();

	std::array<gmod::vector3<double>, 8> corners = {
		LocalToWorld({xMin, yMin, zMin}),
		LocalToWorld({xMin, yMin, zMax}),
		LocalToWorld({xMin, yMax, zMin}),
		LocalToWorld({xMin, yMax, zMax}),
		LocalToWorld({xMax, yMin, zMin}),
		LocalToWorld({xMax, yMin, zMax}),
		LocalToWorld({xMax, yMax, zMin}),
		LocalToWorld({xMax, yMax, zMax})
	};

	gmod::vector3<double> min = corners[0];
	gmod::vector3<double> max = corners[0];

	for (const auto& pt : corners) {
		for (int i = 0; i < 3; ++i) {
			if (pt[i] < min[i]) { min[i] = pt[i]; }
			if (pt[i] > max[i]) { max[i] = pt[i]; }
		}
	}

	return { min, max };
}

IGeometrical::UVBounds Torus::ParametricBounds() const {
	return { 0.0, 2 * DirectX::XM_PI, 0.0, 2 * DirectX::XM_PI };
}

bool Torus::IsUClosed() const {
	return true;
}

bool Torus::IsVClosed() const {
	return true;
}

gmod::vector3<double> Torus::Point(double u, double v) const {
	double cosu = std::cos(u), sinu = std::sin(u);
	double cosv = std::cos(v), sinv = std::sin(v);
	double ring = m_R + m_r * cosu;

	gmod::vector3<double> local = {
		cosv * ring,
		m_r * sinu,
		sinv * ring
	};

	return LocalToWorld(local);
}

gmod::vector3<double> Torus::Tangent(double u, double v, gmod::vector3<double>* dPu, gmod::vector3<double>* dPv) const {
	double cosu = std::cos(u), sinu = std::sin(u);
	double cosv = std::cos(v), sinv = std::sin(v);

	gmod::vector3<double> du = {
		-cosv * m_r * sinu,
		m_r * cosu,
		-sinv * m_r * sinu
	};

	gmod::vector3<double> dv = {
		-sinv * (m_R + m_r * cosu),
		0.0,
		cosv * (m_R + m_r * cosu)
	};

	const auto& M = modelMatrix();
	gmod::matrix3<double> linear {
		M[0], M[1], M[2],    
		M[4], M[5], M[6],     
		M[8], M[9], M[10]     
	};

	du = normalize(linear * du);
	dv = normalize(linear * dv);

	if (dPu) { *dPu = du; }
	if (dPv) { *dPv = dv; }

	return normalize(du + dv);
}

gmod::vector3<double> Torus::Normal(double u, double v, gmod::vector3<double>* dPu, gmod::vector3<double>* dPv) const {
	gmod::vector3<double> du, dv;
	Tangent(u, v, &du, &dv);

	if (dPu) { *dPu = du; }
	if (dPv) { *dPv = dv; }

	return normalize(cross(du, dv));
}
#pragma endregion

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
			const double ring = m_R + m_r * cosu;

			VERTEX vertex{
				.x = cosv * ring,
				.y = m_r * sinu,
				.z = sinv * ring
			};
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

			EDGE nextUEdge{
				.v1 = static_cast<USHORT>(curr),
				.v2 = static_cast<USHORT>(nextU)
			};
			m_edges.push_back(nextUEdge);

			EDGE nextVEdge{
				.v1 = static_cast<USHORT>(curr),
				.v2 = static_cast<USHORT>(nextV)
			};
			m_edges.push_back(nextVEdge);
		}
	}
}

gmod::vector3<double> Torus::LocalToWorld(const gmod::vector3<double>& p) const {
	gmod::vector4<double> local(p.x(), p.y(), p.z(), 1.0);
	gmod::vector4<double> world = gmod::transform_coord(local, this->modelMatrix());
	return { world.x(), world.y(), world.z() };
}
