#pragma once
#include "Surface.h"

namespace app {
	class BSurface : public Surface {
	public:
		BSurface(SurfaceType type, float a, float b, unsigned int aPatch, unsigned int bPatch, unsigned int divisions);
		virtual void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const override;
		virtual void UpdateMesh(const Device& device) override;
	};
}