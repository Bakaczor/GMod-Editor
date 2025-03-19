#pragma once
#include "pch.h"

struct Vertex_PoCo {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 color;
	static const D3D11_INPUT_ELEMENT_DESC layout[2];
};

struct Vertex_Po {
	DirectX::XMFLOAT3 position;
	static const D3D11_INPUT_ELEMENT_DESC layout[1];
};
