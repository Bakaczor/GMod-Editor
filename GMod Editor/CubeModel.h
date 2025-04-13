#pragma once
#include "Mesh.h"

class CubeModel {
public:
	CubeModel();
	void Initialize(const Device& device);
	void Render(const mini::dx_ptr<ID3D11DeviceContext>& context) const;
private:
	Mesh m_mesh;
};
