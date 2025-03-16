#pragma once
#include <DirectXMath.h>
#include <d3d11.h>

struct Vertex_PoCo {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 color;

	static const D3D11_INPUT_ELEMENT_DESC layout[2];
};