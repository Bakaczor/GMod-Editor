#pragma once
#include "../gmod/vector4.h"
#include "../gmod/matrix4.h"
#include "Data.h"
#include "Mesh.h"
#include "Camera.h"
#include "AxesMesh.h"

class Axes {
public:
	Axes();
	gmod::matrix4<float> modelMatrix(const Camera& camera) const;
	void UpdateMesh(const Device& device);
	void RenderMesh(Device& device, mini::dx_ptr<ID3D11Buffer>& constBuffColor) const;
protected:
	AxesMesh m_mesh;
	const float m_scale = 1000.0f;
	const gmod::vector4<float> m_offset = { 0.0f, 0.0f, -1.0f, 0.0f };
};