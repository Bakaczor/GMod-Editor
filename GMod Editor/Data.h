#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <DirectXMath.h>
#include <d3d11.h>

#include <vector>
#include <string>
#include <array>
#include <algorithm>
#include <cmath>

struct Vertex_PoCo {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 color;
	static const D3D11_INPUT_ELEMENT_DESC layout[2];
};

struct Vertex_Po {
	DirectX::XMFLOAT3 position;
	static const D3D11_INPUT_ELEMENT_DESC layout[1];
};
