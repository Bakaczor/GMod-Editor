#pragma once
#include "../mini/dxptr.h"
#include "Descriptions.h"
#include <vector>
#include <string>

namespace mini {
	class Window;
}

class Device {
public:
	explicit Device(const mini::Window& window);

	const mini::dx_ptr<ID3D11Device>& device() const { return m_device; }

	const mini::dx_ptr<ID3D11DeviceContext>& deviceContext() const { return m_deviceContext; }

	const mini::dx_ptr<IDXGISwapChain>& swapChain() const { return m_swapChain; }

	ID3D11Device* operator->() const { return m_device.get(); }

#pragma region VIEWS
	mini::dx_ptr<ID3D11RenderTargetView> CreateRenderTargetView(const mini::dx_ptr<ID3D11Texture2D>& texture) const;

	mini::dx_ptr<ID3D11DepthStencilView> CreateDepthStencilView(const mini::dx_ptr<ID3D11Texture2D>& texture) const;

	mini::dx_ptr<ID3D11DepthStencilView> CreateDepthStencilView(SIZE size) const;

	mini::dx_ptr<ID3D11ShaderResourceView> CreateShaderResourceView(const mini::dx_ptr<ID3D11Texture2D>& texture) const;

	mini::dx_ptr<ID3D11ShaderResourceView> CreateShaderResourceView(SIZE size) const;
#pragma endregion

#pragma region BUFFERS
	mini::dx_ptr<ID3D11Buffer> CreateBuffer(const void* data, const D3D11_BUFFER_DESC& desc) const;

	template<class T>
	mini::dx_ptr<ID3D11Buffer> CreateVertexBuffer(const std::vector<T>& vertices) const {
		auto desc = BufferDescription::VertexBufferDescription(vertices.size() * sizeof(T));
		return CreateBuffer(reinterpret_cast<const void*>(vertices.data()), desc);
	}

	template<class T> 
	mini::dx_ptr<ID3D11Buffer> CreateIndexBuffer(const std::vector<T>& indices) const {
		auto desc = BufferDescription::IndexBufferDescription(indices.size() * sizeof(T));
		return CreateBuffer(reinterpret_cast<const void*>(indices.data()), desc);
	}

	template<typename T, size_t N = 1>
	mini::dx_ptr<ID3D11Buffer> CreateConstantBuffer() const {
		auto desc = BufferDescription::ConstantBufferDescription(N * sizeof(T));
		return CreateBuffer(nullptr, desc);
	}
#pragma endregion

#pragma region SHADERS
	mini::dx_ptr<ID3D11VertexShader> CreateVertexShader(std::vector<BYTE> vsCode) const;

	mini::dx_ptr<ID3D11PixelShader> CreatePixelShader(std::vector<BYTE> psCode) const;
	
	static std::vector<BYTE> LoadByteCode(const std::wstring& filename);
#pragma endregion

#pragma region LAYOUTS
	mini::dx_ptr<ID3D11InputLayout> CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* elements, UINT count, const std::vector<BYTE>& vsCode) const;

	mini::dx_ptr<ID3D11InputLayout> CreateInputLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC>& elements, const std::vector<BYTE>& vsCode) const {
		return CreateInputLayout(elements.data(), elements.size(), vsCode);
	}

	template<UINT N>
	mini::dx_ptr<ID3D11InputLayout> CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC(&elements)[N], const std::vector<BYTE>& vsCode) const {
		return CreateInputLayout(elements, N, vsCode);
	}

	template<typename Vertex>
	mini::dx_ptr<ID3D11InputLayout> CreateInputLayout(const std::vector<BYTE>& vsCode) const {
		return CreateInputLayout(Vertex::layout, vsCode);
	}
#pragma endregion

	mini::dx_ptr<ID3D11Texture2D> CreateTexture(const D3D11_TEXTURE2D_DESC& desc) const;

private:
	mini::dx_ptr<ID3D11Device> m_device;
	mini::dx_ptr<ID3D11DeviceContext> m_deviceContext;
	mini::dx_ptr<IDXGISwapChain> m_swapChain;
};
