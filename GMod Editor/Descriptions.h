#pragma once
#include "framework.h"
#include <d3d11.h>

namespace app {
	struct SwapChainDescription : DXGI_SWAP_CHAIN_DESC {
		SwapChainDescription(HWND wndHwnd, SIZE wndSize);
	};

	struct Viewport : D3D11_VIEWPORT {
		explicit Viewport(SIZE size);
	};

	struct Texture2DDescription : D3D11_TEXTURE2D_DESC {
		Texture2DDescription(UINT width, UINT height, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);

		static Texture2DDescription DepthStencilDescription(UINT width, UINT height);

		static Texture2DDescription DynamicTextureDescription(UINT width, UINT height, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);

		static Texture2DDescription RWTextureDescription(UINT width, UINT height, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
	};

	struct BufferDescription : D3D11_BUFFER_DESC {
		BufferDescription(UINT bindFlags, size_t byteWidth);

		static BufferDescription VertexBufferDescription(size_t byteWidth) {
			return BufferDescription(D3D11_BIND_VERTEX_BUFFER, byteWidth);
		}

		static BufferDescription IndexBufferDescription(size_t byteWidth) {
			return BufferDescription(D3D11_BIND_INDEX_BUFFER, byteWidth);
		}

		static BufferDescription ConstantBufferDescription(size_t byteWidth);

		static BufferDescription RawBufferDescription(size_t byteWidth);

		static BufferDescription StagingBufferDescription(size_t byteWidth);

		static BufferDescription DynamicBufferDescription(size_t byteWidth);
	};

	struct RasterizerDescription : D3D11_RASTERIZER_DESC {
		RasterizerDescription();
	};

	struct BlendDescription : D3D11_BLEND_DESC {
		BlendDescription();

		static BlendDescription AlphaBlendDescription();
		static BlendDescription AdditiveBlendDescription();
	};

	struct DepthStencilDescription : D3D11_DEPTH_STENCIL_DESC {
		DepthStencilDescription();
	};

	struct SamplerDescription : D3D11_SAMPLER_DESC {
		SamplerDescription();
	};

	struct UAVDescription : D3D11_UNORDERED_ACCESS_VIEW_DESC {
		UAVDescription(UINT N, DXGI_FORMAT format);
	};
}