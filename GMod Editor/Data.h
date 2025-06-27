#pragma once
#include "framework.h"
#include <d3d11.h>
#include <DirectXMath.h>

namespace app {
	struct Vertex_PoCo {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 color;
		static const D3D11_INPUT_ELEMENT_DESC layout[2];
	};

	struct Vertex_Po {
		DirectX::XMFLOAT3 position;
		static const D3D11_INPUT_ELEMENT_DESC layout[1];
	};

	struct Vertex_PoCoef {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 coefficient;
		static const D3D11_INPUT_ELEMENT_DESC layout[2];
	};

	struct Vertex_PoUVs {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 uv;
		static const D3D11_INPUT_ELEMENT_DESC layout[2];
	};
}
