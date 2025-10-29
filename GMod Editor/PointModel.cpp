#include "PointModel.h"
#include "Data.h"
#include <cmath>

using namespace app;

PointModel::PointModel() : m_mesh() {}

void PointModel::Initialize(const Device& device) {
	const int verticesNum = m_parts * (m_parts - 1) + 2;
	std::vector<Vertex_Po> verts;
	verts.reserve(verticesNum);

	std::vector<USHORT> idxs;
	idxs.reserve(3 * m_parts * ((m_parts - 2) * 2 + 2));

	const double stepv = 2 * DirectX::XM_PI / m_parts;
	const double stepu = DirectX::XM_PI / m_parts;

	verts.push_back(Vertex_Po { DirectX::XMFLOAT3(0, m_r, 0) }); // top

	for (int j = 0; j < m_parts; ++j) {
		const double v = j * stepv;
		const double cosv = std::cos(v);
		const double sinv = std::sin(v);

		for (int i = 1; i < m_parts; ++i) {
			const double u = i * stepu;
			const double cosu = std::cos(u);
			const double sinu = std::sin(u);

			Vertex_Po vertex{ 
				DirectX::XMFLOAT3(
				static_cast<float>(m_r * sinu * sinv),
				static_cast<float>(m_r * cosu), 
				static_cast<float>(m_r * sinu * cosv)) 
			};
			verts.push_back(vertex);
		}
	}

	verts.push_back(Vertex_Po{ DirectX::XMFLOAT3(0, -m_r, 0) }); // bottom

	USHORT top = 0;
	USHORT bottom = verticesNum - 1;

	const int offset = m_parts - 1;

	// top
	const USHORT firstTopJ = 1;
	const USHORT lastTopJ = offset * offset + firstTopJ;
	for (USHORT j = firstTopJ; j <= lastTopJ; j += offset) {
		USHORT next_j = j + offset;
		if (j == lastTopJ) { next_j = firstTopJ; }

		idxs.push_back(j);
		idxs.push_back(0);
		idxs.push_back(next_j);
	}

	// bottom
	const USHORT firstBottomJ = firstTopJ + offset - 1;
	const USHORT lastBottomJ = offset * offset + firstBottomJ;
	for (USHORT j = firstBottomJ; j <= lastBottomJ; j += offset) {
		USHORT next_j = j + offset;
		if (next_j == lastBottomJ) { next_j = firstBottomJ; }

		idxs.push_back(j);
		idxs.push_back(next_j);
		idxs.push_back(bottom);
	}

	// sides
	for (USHORT j = 0; j < m_parts - 2; ++j) {
		for (USHORT i = 0; i < m_parts; ++i) {
			USHORT v1 = i * offset + j + firstTopJ;
			USHORT v2 = v1 + 1;
			USHORT v3 = i == m_parts - 1 ? j + firstTopJ : v1 + offset;
			USHORT v4 = v3 + 1;

			idxs.push_back(v1);
			idxs.push_back(v4);
			idxs.push_back(v2);

			idxs.push_back(v1);
			idxs.push_back(v3);
			idxs.push_back(v4);
		}
	}

	m_mesh.Update(device, verts, idxs, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void PointModel::Render(const mini::dx_ptr<ID3D11DeviceContext>& context) const {
	m_mesh.Render(context);
}
