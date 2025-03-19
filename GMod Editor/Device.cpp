#include "pch.h"
#include "Device.h"
#include "../mini/exceptions.h"
#include <fstream>

Device::Device(const mini::Window& window) {
	SwapChainDescription desc{ window.getHandle(), window.getClientSize() };
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* context = nullptr;
	IDXGISwapChain* swapChain = nullptr;

	const D3D_FEATURE_LEVEL featureLevels[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
	auto hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
		featureLevels, 2, D3D11_SDK_VERSION, &desc, &swapChain, &device, nullptr, &context);
	if (FAILED(hr)) {
		THROW_DX(hr);
	}

	m_device.reset(device);
	m_swapChain.reset(swapChain);
	m_deviceContext.reset(context);
}

mini::dx_ptr<ID3D11Texture2D> Device::CreateTexture(const D3D11_TEXTURE2D_DESC& desc) const {
	ID3D11Texture2D* temp;
	auto hr = m_device->CreateTexture2D(&desc, nullptr, &temp);
	if (FAILED(hr)) {
		THROW_DX(hr);
	}
	mini::dx_ptr<ID3D11Texture2D> result(temp);
	return result;
}

mini::dx_ptr<ID3D11RasterizerState> Device::CreateRasterizerState(const D3D11_RASTERIZER_DESC& desc) const {
	ID3D11RasterizerState* temp = nullptr;
	auto hr = m_device->CreateRasterizerState(&desc, &temp);
	if (FAILED(hr)) {
		THROW_DX(hr);
	}
	mini::dx_ptr<ID3D11RasterizerState> result(temp);
	return result;
}

#pragma region VIEWS
mini::dx_ptr<ID3D11RenderTargetView> Device::CreateRenderTargetView(const mini::dx_ptr<ID3D11Texture2D>& texture) const {
	ID3D11RenderTargetView* temp;
	auto hr = m_device->CreateRenderTargetView(texture.get(), nullptr, &temp);
	if (FAILED(hr)) {
		THROW_DX(hr);
	}
	mini::dx_ptr<ID3D11RenderTargetView> result(temp);
	return result;
}

mini::dx_ptr<ID3D11DepthStencilView> Device::CreateDepthStencilView(const mini::dx_ptr<ID3D11Texture2D>& texture) const {
	ID3D11DepthStencilView* temp;
	auto hr = m_device->CreateDepthStencilView(texture.get(), nullptr, &temp);
	if (FAILED(hr)) {
		THROW_DX(hr);
	}
	mini::dx_ptr<ID3D11DepthStencilView> result(temp);
	return result;
}

mini::dx_ptr<ID3D11DepthStencilView> Device::CreateDepthStencilView(SIZE size) const {
	auto desc = Texture2DDescription::DepthStencilDescription(size.cx, size.cy);
	mini::dx_ptr<ID3D11Texture2D> texture = CreateTexture(desc);
	return CreateDepthStencilView(texture);
}

mini::dx_ptr<ID3D11ShaderResourceView> Device::CreateShaderResourceView(const mini::dx_ptr<ID3D11Texture2D>& texture) const {
	ID3D11ShaderResourceView* temp;
	auto hr = m_device->CreateShaderResourceView(texture.get(), nullptr, &temp);
	if (FAILED(hr)) {
		THROW_DX(hr);
	}
	mini::dx_ptr<ID3D11ShaderResourceView> result(temp);
	return result;
}

mini::dx_ptr<ID3D11ShaderResourceView> Device::CreateShaderResourceView(SIZE size) const {
	auto desc = Texture2DDescription::DynamicTextureDescription(size.cx, size.cy);
	mini::dx_ptr<ID3D11Texture2D> texture = CreateTexture(desc);
	return CreateShaderResourceView(texture);
}
#pragma endregion

#pragma region BUFFERS
mini::dx_ptr<ID3D11Buffer> Device::CreateBuffer(const void* data, const D3D11_BUFFER_DESC& desc) const {
	D3D11_SUBRESOURCE_DATA sdata;
	ZeroMemory(&sdata, sizeof sdata);
	sdata.pSysMem = data;

	ID3D11Buffer* temp;
	auto hr = m_device->CreateBuffer(&desc, data ? &sdata : nullptr, &temp);
	if (FAILED(hr)) {
		THROW_DX(hr);
	}
	mini::dx_ptr<ID3D11Buffer> result(temp);
	return result;
}


void Device::UpdateBuffer(const mini::dx_ptr<ID3D11Buffer>& buffer, const void* data, std::size_t count) {
	D3D11_MAPPED_SUBRESOURCE res;
	auto hr = m_deviceContext->Map(buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	if (FAILED(hr)) {
		THROW_DX(hr);
	}
	memcpy(res.pData, data, count);
	m_deviceContext->Unmap(buffer.get(), 0);
}
#pragma endregion

#pragma region SHADERS
mini::dx_ptr<ID3D11VertexShader> Device::CreateVertexShader(std::vector<BYTE> vsCode) const {
	ID3D11VertexShader* temp;
	auto hr = m_device->CreateVertexShader(reinterpret_cast<const void*>(vsCode.data()), vsCode.size(), nullptr, &temp);
	if (FAILED(hr)) {
		THROW_DX(hr);
	}
	mini::dx_ptr<ID3D11VertexShader> result(temp);
	return result;
}

mini::dx_ptr<ID3D11PixelShader> Device::CreatePixelShader(std::vector<BYTE> psCode) const {
	ID3D11PixelShader* temp;
	auto hr = m_device->CreatePixelShader(reinterpret_cast<const void*>(psCode.data()), psCode.size(), nullptr, &temp);
	if (FAILED(hr)) {
		THROW_DX(hr);
	}
	mini::dx_ptr<ID3D11PixelShader> result(temp);
	return result;
}

std::vector<BYTE> Device::LoadByteCode(const std::wstring& filename) {
	std::ifstream sIn(filename, std::ios::in | std::ios::binary);
	if (!sIn) {
		THROW(L"Unable to open " + filename);
	}
	sIn.seekg(0, std::ios::end);
	auto byteCodeLength = sIn.tellg();
	sIn.seekg(0, std::ios::beg);
	std::vector<BYTE> byteCode(static_cast<unsigned int>(byteCodeLength));
	if (!sIn.read(reinterpret_cast<char*>(byteCode.data()), byteCodeLength)) {
		THROW(L"Error reading" + filename);
	}
	sIn.close();
	return byteCode;
}
#pragma endregion

#pragma region LAYOUTS
mini::dx_ptr<ID3D11InputLayout> Device::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* elements, UINT count, const std::vector<BYTE>& vsCode) const {
	ID3D11InputLayout* temp;
	auto hr = m_device->CreateInputLayout(elements, count,
		reinterpret_cast<const void*>(vsCode.data()), vsCode.size(), &temp);
	if (FAILED(hr)) {
		THROW_DX(hr);
	}
	mini::dx_ptr<ID3D11InputLayout> result(temp);
	return result;
}
#pragma endregion
