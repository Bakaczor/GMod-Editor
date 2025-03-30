#pragma once
#include "../gmod/vector4.h"
#include "../gmod/matrix4.h"
#include "Data.h"
#include "Mesh.h"
#include "Camera.h"

class AxesMesh {
public:
	AxesMesh();
	void Initialize(const Device& device);
	void Render(Device& device, mini::dx_ptr<ID3D11Buffer>& constBuffColor) const;

	std::array<float, 4> m_colorX = { 1.0f, 0.0f, 0.0f, 1.0f };
	std::array<float, 4> m_colorY = { 0.0f, 1.0f, 0.0f, 1.0f };
	std::array<float, 4> m_colorZ = { 0.0f, 0.0f, 1.0f, 1.0f };
private:
	std::vector<USHORT> m_idxs = { 0, 1 };

	Mesh m_meshX;
	std::vector<Vertex_Po> m_vertsX = {
		{ DirectX::XMFLOAT3(-0.5f, 0, 0) },
		{ DirectX::XMFLOAT3(0.5f, 0, 0) }
	};
	Mesh m_meshY;
	std::vector<Vertex_Po> m_vertsY = {
		{ DirectX::XMFLOAT3(0, -0.5f, 0) },
		{ DirectX::XMFLOAT3(0, 0.5f, 0) }
	};
	Mesh m_meshZ;
	std::vector<Vertex_Po> m_vertsZ = {
		{ DirectX::XMFLOAT3(0, 0, -0.5f) },
		{ DirectX::XMFLOAT3(0, 0, 0.5f) }
	};
};