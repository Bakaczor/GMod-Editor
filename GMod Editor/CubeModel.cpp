#include "CubeModel.h"
#include "Data.h"

CubeModel::CubeModel() : m_mesh() {}

void CubeModel::Initialize(const Device& device) {
	std::vector<USHORT> idxs = { 
		0, 1, 1, 2, 2, 3, 3, 0,
		0, 4, 1, 5, 2, 6, 3, 7,
		4, 5, 5, 6, 6, 7, 7, 4
	};

	std::vector<Vertex_Po> verts = {
		{ DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f) },
		{ DirectX::XMFLOAT3(0.5f, -0.5f,  0.5f) },
		{ DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f) },
		{ DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f) },
		{ DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f) },
		{ DirectX::XMFLOAT3(0.5f,  0.5f,  0.5f) },
		{ DirectX::XMFLOAT3(0.5f,  0.5f, -0.5f) },
		{ DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f) }
	};

	m_mesh.Update(device, verts, idxs, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

void CubeModel::Render(const mini::dx_ptr<ID3D11DeviceContext>& context) const {
	m_mesh.Render(context);
}
