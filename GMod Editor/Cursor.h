#pragma once
#include "../gmod/Transform.h"
#include "AxesModel.h"
#include "Shaders.h"

namespace app {
	class Cursor {
	public:
		gmod::Transform<double> transform;

		Cursor(AxesModel* model = nullptr);
		ShaderType shaderType() const { return m_shaderType; }
		void SetModel(AxesModel* model);
		void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context) const;
	private:
		AxesModel* m_model;
		ShaderType m_shaderType = ShaderType::RegularWithColors;
	};
}