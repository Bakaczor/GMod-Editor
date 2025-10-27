#pragma once
#include "framework.h"
#include <d3d11.h>
#include <DirectXMath.h>

namespace app {
	struct DirLight {
		DirectX::XMFLOAT4 direction;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT4 weights;
	};

	struct Material {
		DirectX::XMFLOAT4 ambient;
		DirectX::XMFLOAT4 diffuse;
		DirectX::XMFLOAT4 specular;
		float shininess;
		float padding[3];
	};
}