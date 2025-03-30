#pragma once
#include "AxesMesh.h"

class Cursor {
public:
	gmod::Transform<double> transform;

	Cursor();
	void UpdateMesh(const Device& device);
	void RenderMesh(Device& device, mini::dx_ptr<ID3D11Buffer>& constBuffColor) const;
private:
	AxesMesh m_mesh;
};