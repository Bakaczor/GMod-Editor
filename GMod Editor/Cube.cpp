#include "Cube.h"

using namespace app;

unsigned short Cube::m_globalCubeNum = 0;

Cube::Cube(CubeModel* model) : m_model(model) {
	m_type = "Cube";
	std::ostringstream os;
	os << "cube_" << m_globalCubeNum;
	name = os.str();
	m_globalCubeNum += 1;
}

void Cube::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context) const {
	m_model->Render(context);
}
