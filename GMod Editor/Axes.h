#pragma once
#include "../gmod/vector4.h"
#include "../gmod/matrix4.h"
#include "Data.h"
#include "Mesh.h"
#include "Camera.h"
#include "AxesModel.h"

class Axes {
public:
	Axes(AxesModel* model = nullptr);
	gmod::matrix4<float> modelMatrix(const Camera& camera, float f, float a) const;
	void SetModel(AxesModel* model);
	void RenderMesh(Device& device, mini::dx_ptr<ID3D11Buffer>& constBuffColor) const;
protected:
	AxesModel* m_model;
	// const gmod::vector4<float> m_offset = { 0.0f, 0.0f, -1.0f, 0.0f };
};