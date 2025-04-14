#pragma once
#include "Device.h"

namespace app {
	struct Shaders {
		mini::dx_ptr<ID3D11VertexShader> vertexShader;
		mini::dx_ptr<ID3D11HullShader> hullShader;
		mini::dx_ptr<ID3D11DomainShader> domainShader;
		mini::dx_ptr<ID3D11PixelShader> pixelShader;
		mini::dx_ptr<ID3D11InputLayout> layout;

		void Set(const mini::dx_ptr<ID3D11DeviceContext>& context) const;
		void Unset(const mini::dx_ptr<ID3D11DeviceContext>& context) const;
	};

	enum class ShaderType {
		Regular, RegularWithColors, RegularWithTesselation
	};
}