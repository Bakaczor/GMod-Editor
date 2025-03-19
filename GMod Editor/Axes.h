#pragma once
#include "../gmod/vector4.h"
#include "../gmod/matrix4.h"
#include "Data.h"
#include "Mesh.h"
#include "Camera.h"

class Axes {
public:
	Axes();
	gmod::matrix4<float> modelMatrix(const Camera& camera) const;
	void Initialize(const Device& device);
	void Render(Device& device, mini::dx_ptr<ID3D11Buffer>& constBuffColor) const;
protected:
	const float m_scale = 100.0f;
	const gmod::vector4<float> m_offset = { 0.0f, 0.0f, -1.0f, 0.0f };
	std::vector<USHORT> m_idxs = { 0, 1 };

	std::array<float, 4> m_colorX = { 1.0f, 0.0f, 0.0f, 1.0f };
	Mesh m_meshX;
	std::vector<Vertex_Po> m_vertsX = {
		{ DirectX::XMFLOAT3(-0.5f, 0, 0) },
		{ DirectX::XMFLOAT3(0.5f, 0, 0) }
	};
	
	std::array<float, 4> m_colorY = { 0.0f, 1.0f, 0.0f, 1.0f };
	Mesh m_meshY;
	std::vector<Vertex_Po> m_vertsY = {
		{ DirectX::XMFLOAT3(0, -0.5f, 0) },
		{ DirectX::XMFLOAT3(0, 0.5f, 0) }
	};

	std::array<float, 4> m_colorZ = { 0.0f, 0.0f, 1.0f, 1.0f };
	Mesh m_meshZ;
	std::vector<Vertex_Po> m_vertsZ = {
		{ DirectX::XMFLOAT3(0, 0, -0.5f) },
		{ DirectX::XMFLOAT3(0, 0, 0.5f) }
	};
};