#pragma once
#include "Object.h"
#include "CubeModel.h"

namespace app {
	class Cube : public Object {
	public:
		Cube(CubeModel* model);
		virtual void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const override;
	private:
		static unsigned short m_globalCubeNum;
		CubeModel* m_model;
	};
}