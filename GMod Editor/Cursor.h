#pragma once
#include "../gmod/Transform.h"
#include "AxesModel.h"

namespace app {
	class Cursor {
	public:
		gmod::Transform<double> transform;

		Cursor(AxesModel* model = nullptr);
		void SetModel(AxesModel* model);
		void RenderMesh(Device& device, mini::dx_ptr<ID3D11Buffer>& constBuffColor) const;
	private:
		AxesModel* m_model;
	};
}