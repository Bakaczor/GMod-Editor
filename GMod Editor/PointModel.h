#pragma once
#include "Mesh.h"

class PointModel {
public:
	PointModel();
	void Initialize(const Device& device);
	void Render(const mini::dx_ptr<ID3D11DeviceContext>& context) const;
private:
	const float m_r = 0.05f;
	const int m_parts = 12;
	Mesh m_mesh;
};
