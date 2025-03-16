#include "Mesh.h"
#include <algorithm>

Mesh::Mesh() : m_indexCount(0), m_primitiveType(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED) {}

Mesh::Mesh(mini::dx_ptr_vector<ID3D11Buffer>&& vbuffers, std::vector<UINT>&& vstrides, std::vector<UINT>&& voffsets, 
	mini::dx_ptr<ID3D11Buffer>&& indices, UINT indexCount, D3D_PRIMITIVE_TOPOLOGY primitiveType) {
	assert(vbuffers.size() == voffsets.size() && vbuffers.size() == vstrides.size());

	m_indexCount = indexCount;
	m_primitiveType = primitiveType;
	m_indexBuffer = move(indices);
	m_vertexBuffers = std::move(vbuffers);
	m_strides = std::move(vstrides);
	m_offsets = std::move(voffsets);
}

Mesh::Mesh(Mesh&& other) noexcept :
	m_indexBuffer(std::move(other.m_indexBuffer)), m_vertexBuffers(std::move(other.m_vertexBuffers)),
	m_strides(std::move(other.m_strides)), m_offsets(std::move(other.m_offsets)),
	m_indexCount(other.m_indexCount), m_primitiveType(other.m_primitiveType) {
	other.Release();
}

void Mesh::Release() {
	m_vertexBuffers.clear();
	m_strides.clear();
	m_offsets.clear();
	m_indexBuffer.reset();
	m_indexCount = 0;
	m_primitiveType = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
	Release();
	m_vertexBuffers = std::move(other.m_vertexBuffers);
	m_indexBuffer = std::move(other.m_indexBuffer);
	m_strides = std::move(other.m_strides);
	m_offsets = std::move(other.m_offsets);
	m_indexCount = other.m_indexCount;
	m_primitiveType = other.m_primitiveType;
	other.Release();
	return *this;
}

void Mesh::Render(const mini::dx_ptr<ID3D11DeviceContext>& context) const {
	if (!m_indexBuffer || m_vertexBuffers.empty()) { return; }
	context->IASetPrimitiveTopology(m_primitiveType);
	context->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetVertexBuffers(0, m_vertexBuffers.size(), m_vertexBuffers.data(), m_strides.data(), m_offsets.data());
	context->DrawIndexed(m_indexCount, 0, 0);
}

Mesh::~Mesh() { Release(); }
