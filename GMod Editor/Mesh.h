#pragma once
#include "Data.h"
#include "Device.h"

namespace app {
	class Mesh {
	public:
		Mesh();
		Mesh(
			mini::dx_ptr_vector<ID3D11Buffer>&& vbuffers,
			std::vector<UINT>&& vstrides,
			mini::dx_ptr<ID3D11Buffer>&& indices,
			UINT indexCount,
			D3D_PRIMITIVE_TOPOLOGY primitiveType
		) : Mesh(std::move(vbuffers), std::move(vstrides), std::vector<UINT>(vbuffers.size(), 0U), std::move(indices), indexCount, primitiveType) {}

		Mesh(
			mini::dx_ptr_vector<ID3D11Buffer>&& vbuffers,
			std::vector<UINT>&& vstrides,
			std::vector<UINT>&& voffsets,
			mini::dx_ptr<ID3D11Buffer>&& indices,
			UINT indexCount,
			D3D_PRIMITIVE_TOPOLOGY primitiveType
		);

		Mesh(Mesh&& other) noexcept;
		Mesh(const Mesh& other) = delete;
		void Release();
		~Mesh();

		Mesh& operator=(const Mesh& other) = delete;
		Mesh& operator=(Mesh&& other) noexcept;
		void Render(const mini::dx_ptr<ID3D11DeviceContext>& context, DXGI_FORMAT format = DXGI_FORMAT_R16_UINT) const;

		template<typename Vertex, typename T>
		void Update(const Device& device, const std::vector<Vertex> verts, const std::vector<T> idxs, D3D_PRIMITIVE_TOPOLOGY primitiveType) {
			m_indexBuffer = device.CreateIndexBuffer(idxs);
			m_vertexBuffers.clear();
			m_vertexBuffers.push_back(device.CreateVertexBuffer(verts));
			m_strides = { sizeof(Vertex) };
			m_offsets = { 0 };
			m_indexCount = idxs.size();
			m_primitiveType = primitiveType;
		}

	private:
		mini::dx_ptr<ID3D11Buffer> m_indexBuffer;
		mini::dx_ptr_vector<ID3D11Buffer> m_vertexBuffers;
		std::vector<UINT> m_strides;
		std::vector<UINT> m_offsets;
		UINT m_indexCount;
		D3D_PRIMITIVE_TOPOLOGY m_primitiveType;
	};
}