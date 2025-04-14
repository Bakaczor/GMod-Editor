#pragma once
#include <unordered_map>
#include "../gmod/Transform.h"
#include "AxesModel.h"
#include "Shaders.h"

namespace app {
	class Cursor {
	public:
		gmod::Transform<double> transform;

		Cursor(AxesModel* model = nullptr);
		void SetModel(AxesModel* model);
		void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const;
	private:
		AxesModel* m_model;
	};
}