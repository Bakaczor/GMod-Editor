#include "AxesModel.h"
#include "Data.h"

using namespace app;

AxesModel::AxesModel() : m_mesh() {}

void AxesModel::Initialize(const Device& device) {
	std::vector<USHORT> idxs = { 0, 1, 2, 3, 4, 5 };
	std::vector<Vertex_PoCo> verts = {
		{ DirectX::XMFLOAT3(-0.5f, 0, 0), DirectX::XMFLOAT3(colorX.data()) },
		{ DirectX::XMFLOAT3(0.5f, 0, 0),  DirectX::XMFLOAT3(colorX.data()) },
		{ DirectX::XMFLOAT3(0, -0.5f, 0), DirectX::XMFLOAT3(colorY.data()) },
		{ DirectX::XMFLOAT3(0, 0.5f, 0),  DirectX::XMFLOAT3(colorY.data()) },
		{ DirectX::XMFLOAT3(0, 0, -0.5f), DirectX::XMFLOAT3(colorZ.data()) },
		{ DirectX::XMFLOAT3(0, 0, 0.5f),  DirectX::XMFLOAT3(colorZ.data()) }
	};
	m_mesh.Update(device, verts, idxs, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

void AxesModel::Render(const mini::dx_ptr<ID3D11DeviceContext>& context) const {
	m_mesh.Render(context);
}

