#include "PointModel.h"
#include "Data.h"
#include <cmath>

PointModel::PointModel() : m_mesh() {}

void PointModel::Initialize(const Device& device) {
	const int verticesNum = m_parts * m_parts;
	std::vector<Vertex_Po> verts;
	verts.reserve(verticesNum);

	std::vector<USHORT> idxs;
	idxs.reserve(6 * verticesNum);

	const double step = 2 * DirectX::XM_PI / m_parts;

	for (int j = 0; j < m_parts; ++j) {
		const double v = j * step;
		const double cosv = std::cos(v);
		const double sinv = std::sin(v);

		for (int i = 0; i < m_parts; ++i) {
			const double u = i * step;
			const double cosu = std::cos(u);
			const double sinu = std::sin(u);

			Vertex_Po vertex;
			vertex.position = DirectX::XMFLOAT3(
				static_cast<float>(m_r * cosv * cosu),
				static_cast<float>(m_r * sinv * cosu),
				static_cast<float>(m_r * sinu));
			verts.push_back(vertex);
		}
	}

	for (int j = 0; j < m_parts - 1; ++j) {
		for (int i = 0; i < m_parts - 1; ++i) {
			USHORT v1 = j * m_parts + i;
			USHORT v2 = j * m_parts + (i + 1) % m_parts;
			USHORT v3 = (j + 1) * m_parts + i;
			USHORT v4 = (j + 1) * m_parts + (i + 1) % m_parts;

			idxs.push_back(v1);
			idxs.push_back(v2);
			idxs.push_back(v3);

			idxs.push_back(v2);
			idxs.push_back(v4);
			idxs.push_back(v3);
		}
	}

	m_mesh.Update(device, verts, idxs, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void PointModel::Render(const mini::dx_ptr<ID3D11DeviceContext>& context) const {
	m_mesh.Render(context);
}
