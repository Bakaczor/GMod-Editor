#pragma once
#include "framework.h"
#include <d3d11.h>
#include <DirectXMath.h>

namespace app {
	struct DirLight {
		DirectX::XMFLOAT3 direction;
		DirectX::XMFLOAT3 color;
		DirectX::XMFLOAT3 weights;
	};

	struct Material {
		DirectX::XMFLOAT3 ambient;
		DirectX::XMFLOAT3 diffuse;
		DirectX::XMFLOAT3 specular;
		float shininess;
		float padding[3];
	};
}