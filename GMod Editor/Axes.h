#pragma once
#include "../gmod/matrix4.h"
#include "../gmod/vector4.h"
#include "AxesModel.h"
#include "Camera.h"
#include "Data.h"
#include "Mesh.h"
#include "Shaders.h"

namespace app {
	class Axes {
	public:
		Axes(AxesModel* model = nullptr);
		gmod::matrix4<float> modelMatrix(const Camera& camera, float f, float a) const;
		ShaderType shaderType() const { return m_shaderType; }
		void SetModel(AxesModel* model);
		void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context) const;
	protected:
		AxesModel* m_model;
		ShaderType m_shaderType = ShaderType::RegularWithColors;
		// const gmod::vector4<float> m_offset = { 0.0f, 0.0f, -1.0f, 0.0f };
	};
}