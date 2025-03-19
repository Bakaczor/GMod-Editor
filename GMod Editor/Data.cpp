#include "pch.h"
#include "Data.h"

const D3D11_INPUT_ELEMENT_DESC Vertex_PoCo::layout[2] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(Vertex_PoCo, position), 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex_PoCo, color), D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

const D3D11_INPUT_ELEMENT_DESC Vertex_Po::layout[1] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(Vertex_Po, position), 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};